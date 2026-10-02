#define NICOLAI_CAPTURE_TEST
#include "../tools/nicolai_m36_runtime_capture.cpp"
#include <cstring>

void check_detach_m46() {
    wchar_t path[32768]{};
    if (!GetModuleFileNameW(nullptr, path, 32768)) throw std::runtime_error("fixture path unavailable");
    std::wstring command = L"\"" + std::wstring(path) + L"\" --detach-fixture";
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    PROCESS_INFORMATION child{};
    if (!CreateProcessW(path, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &startup, &child)) throw std::runtime_error("fixture launch failed");
    bool attached = false, active = false;
    DEBUG_EVENT event{};
    struct Cleanup {
        PROCESS_INFORMATION& child; bool& attached; bool& active; DEBUG_EVENT& event;
        ~Cleanup() {
            TerminateProcess(child.hProcess, ERROR_CANCELLED); // This test's synthetic child only.
            if (active) ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
            if (attached) DebugActiveProcessStop(child.dwProcessId);
            WaitForSingleObject(child.hProcess, 1000);
            CloseHandle(child.hThread); CloseHandle(child.hProcess);
        }
    } cleanup{child, attached, active, event};
    HANDLE process = OpenProcess(kLinguisticAccessM46, FALSE, child.dwProcessId);
    if (!process) throw std::runtime_error("fixture access mask rejected");
    struct HandleGuard { HANDLE h; ~HandleGuard() { CloseHandle(h); } } handle{process};
    if (!DebugActiveProcess(child.dwProcessId)) throw std::runtime_error("fixture attach failed");
    attached = true;
    if (!DebugSetProcessKillOnExit(FALSE)) throw std::runtime_error("fixture detach policy failed");
    const auto deadline = GetTickCount64() + 5000;
    int breaks = 0;
    while (breaks < 2 && GetTickCount64() < deadline) {
        if (!WaitForDebugEvent(&event, 100)) {
            if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
            throw std::runtime_error("fixture debug wait failed");
        }
        active = true;
        close_debug_image_file_m46(event);
        bool request_break = false;
        DWORD continuation = DBG_CONTINUE;
        if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            if (event.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) {
                ++breaks; request_break = breaks == 1;
            } else continuation = DBG_EXCEPTION_NOT_HANDLED;
        }
        if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, continuation))
            throw std::runtime_error("fixture continuation failed");
        active = false;
        if (request_break && !DebugBreakProcess(process)) throw std::runtime_error("fixture detach break failed");
    }
    if (breaks != 2) throw std::runtime_error("fixture remote break was not observed");
    if (!DebugActiveProcessStop(child.dwProcessId))
        throw std::runtime_error("fixture detach Win32 error=" + std::to_string(GetLastError()));
    attached = false;
    BOOL debugging = TRUE;
    if (!CheckRemoteDebuggerPresent(child.hProcess, &debugging) || debugging ||
        WaitForSingleObject(child.hProcess, 0) != WAIT_TIMEOUT)
        throw std::runtime_error("fixture was not left alive and detached");
}

int main(int argc, char** argv) try {
    if (argc == 2 && std::string(argv[1]) == "--detach-fixture") { Sleep(10000); return 0; }
    check_detach_m46();
    std::vector<BYTE> state(0x4000, 0), morph(3 * kMorphStrideM46, 0), raw(3, ' '),
                      codes(12, 0), descriptors(36, 0), phones(32, 0);
    char word[] = "qzx<qv";
    char* words[]{word, word, word};
    auto put = [](std::vector<BYTE>& bytes, std::size_t at, const auto& value) {
        std::memcpy(bytes.data() + at, &value, sizeof(value));
    };
    put(state, 0x134, std::int32_t{1});
    put(state, 0x124, morph.data()); put(state, 0x128, raw.data());
    put(state, 0x12c, codes.data());
    auto* word_pointers = words; put(state, 0x13c, word_pointers);
    put(state, 0x16c, descriptors.data());
    morph[kMorphStrideM46] = 1;
    put(morph, kMorphStrideM46 + 20, std::uint32_t{0x12345678});
    morph[kMorphStrideM46 + 24] = 5;
    morph[kMorphStrideM46 + 25] = 2;
    morph[kMorphStrideM46 + 27] = 4;
    morph[kMorphStrideM46 + 29] = 60;
    put(morph, kMorphStrideM46 + 32, std::int32_t{-7});
    morph[kMorphStrideM46 + 36] = 9;
    put(morph, kMorphStrideM46 + 40, std::uint32_t{0xabcdef01});
    morph[kMorphStrideM46 + 0x590] = ',';
    raw[1] = ','; std::memcpy(codes.data() + 4, "///", 4);
    put(descriptors, 12, phones.data()); descriptors[16] = 1;
    phones[0x1d] = 251; // raw representation only; no semantic guess
    std::ostringstream out;
    const auto address = reinterpret_cast<std::uintptr_t>(state.data());
    json_linguistic_state_m46(out, GetCurrentProcess(), address, true);
    for (const auto* needle : {"\"word_count\":1", "\"candidate_count\":1",
                               "\"annotated_cp866_hex\":\"717a783c7176\"",
                               "\"code_hex\":\"2f2f2f\"", "\"source_phone_count\":1",
                               "\"candidates_hex\":[\"7856341205020004003c0000f9ffffff09000000\"]",
                               "\"candidate_payloads_hex\":[\"05020004003c0000f9ffffff0900000001efcdab\"]"})
        if (out.str().find(needle) == std::string::npos) throw std::runtime_error("snapshot field mismatch");
    const auto saved_state = state;
    // Early hooks must not inspect separator/code/source arrays before setup.
    put(state, 0x128, std::uint32_t{0}); put(state, 0x12c, std::uint32_t{0});
    put(state, 0x16c, std::uint32_t{0});
    std::ostringstream early;
    json_linguistic_state_m46(early, GetCurrentProcess(), address, false, true);
    if (early.str().find("candidate_payloads_hex") == std::string::npos ||
        early.str().find("code_hex") != std::string::npos)
        throw std::runtime_error("early analysis read uninitialized separator arrays");
    std::copy(saved_state.begin(), saved_state.end(), state.begin());
    // The last payload ends exactly at punctuation, not four bytes inside it.
    morph[kMorphStrideM46] = 70;
    put(morph, kMorphStrideM46 + 0x58c, std::uint32_t{0x11223344});
    std::ostringstream capacity;
    json_linguistic_state_m46(capacity, GetCurrentProcess(), address, false, true);
    if (capacity.str().find("0000000000000000000000000000000044332211\"]") == std::string::npos)
        throw std::runtime_error("last candidate payload boundary mismatch");
    morph[kMorphStrideM46] = 1;
    std::fill(state.begin(), state.end(), BYTE{0});
    for (bool records : {false, true}) {
        std::ostringstream empty;
        json_linguistic_state_m46(empty, GetCurrentProcess(), address, records);
        if (empty.str() != "{\"word_count\":0,\"words\":[]}")
            throw std::runtime_error("zero-word snapshot dereferenced unused arrays");
    }
    std::copy(saved_state.begin(), saved_state.end(), state.begin());
    int rejected = 0;
    auto reject = [&](bool records) {
        std::ostringstream staged; staged << "prefix";
        try { json_linguistic_state_m46(staged, GetCurrentProcess(), address, records); }
        catch (const std::runtime_error&) {
            if (staged.str() != "prefix") throw std::runtime_error("partial rejected snapshot");
            ++rejected; return;
        }
        throw std::runtime_error("invalid snapshot accepted");
    };
    put(state, 0x134, std::int32_t{257}); reject(false);
    put(state, 0x134, std::int32_t{-1}); reject(true);
    put(state, 0x134, std::int32_t{1});
    morph[kMorphStrideM46] = 71; reject(false); morph[kMorphStrideM46] = 1;
    std::memset(codes.data() + 4, '/', 4); reject(false); std::memcpy(codes.data() + 4, "///", 4);
    descriptors[16] = 129; reject(true); descriptors[16] = 1;
    auto* null_pointer = static_cast<BYTE*>(nullptr); put(state, 0x124, null_pointer); reject(false);
    try { validate_owner_m46(GetCurrentProcessId(), 0); }
    catch (const std::runtime_error&) { ++rejected; }
    if (rejected != 7) throw std::runtime_error("negative contract count mismatch");
    std::cout << "synthetic linguistic snapshot: fields, zero-word calls, atomic output, seven guards and owned debug detach passed\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
