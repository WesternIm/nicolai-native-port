#if !defined(_WIN32) || defined(_WIN64)
#error nicolai_m36_runtime_capture requires a Win32 x86 build
#endif

#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>
#include "local_original_sha256.hpp"

namespace {
constexpr std::uintptr_t kBuilderRva = 0x1a2780;
constexpr std::uintptr_t kKnownReturnRva = 0x10ce7e;
constexpr DWORD kExpectedTimestamp = 0x412a0cb4;
constexpr DWORD kExpectedImageSize = 0x797000;
constexpr DWORD kExpectedPreferredBase = 0x10000000;
constexpr int kMaxFeatureCount = 11;
constexpr int kMaxNodes = 1000;

std::atomic<bool> g_stop{false};
BOOL WINAPI console_handler(DWORD type) {
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT ||
        type == CTRL_CLOSE_EVENT) {
        g_stop = true;
        return TRUE;
    }
    return FALSE;
}

struct FeatureSnapshot {
    int count = 0;
    std::vector<std::int16_t> interval_duration;
    std::vector<std::int16_t> pitch_anchor;
};

struct DescriptorSnapshot {
    int node_count = 0;
    int split_index = 0;
    std::vector<std::int32_t> source_position;
    std::vector<std::int16_t> voicing;
    std::vector<std::int16_t> duration_q11;
    std::vector<std::int16_t> pitch_q11;
};

struct PendingCall {
    std::uint64_t sequence = 0;
    std::uintptr_t feature_ptr = 0;
    std::uintptr_t previous_ptr = 0;
    std::uintptr_t next_ptr = 0;
    FeatureSnapshot feature_before;
    DescriptorSnapshot previous_before;
    DescriptorSnapshot next_before;
};

struct StepState {
    std::uintptr_t breakpoint_address = 0;
    std::vector<HANDLE> suspended_threads;
};

template <class T>
T read_value(HANDLE process, std::uintptr_t address) {
    T value{};
    SIZE_T got = 0;
    if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(address),
            &value, sizeof(value), &got) || got != sizeof(value))
        throw std::runtime_error("ReadProcessMemory scalar failed");
    return value;
}

template <class T>
std::vector<T> read_vector(HANDLE process, std::uintptr_t address,
    std::size_t count) {
    std::vector<T> out(count);
    if (count == 0) return out;
    SIZE_T got = 0;
    const SIZE_T bytes = sizeof(T) * count;
    if (!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(address),
            out.data(), bytes, &got) || got != bytes)
        throw std::runtime_error("ReadProcessMemory vector failed");
    return out;
}

FeatureSnapshot snapshot_feature(HANDLE process, std::uintptr_t ptr) {
    FeatureSnapshot out;
    out.count = read_value<std::int32_t>(process, ptr);
    if (out.count <= 0) return out;
    if (out.count > kMaxFeatureCount)
        throw std::runtime_error("feature count outside guarded capture domain");
    out.interval_duration = read_vector<std::int16_t>(process, ptr + 0x04,
        static_cast<std::size_t>(out.count - 1));
    out.pitch_anchor = read_vector<std::int16_t>(process, ptr + 0x18,
        static_cast<std::size_t>(out.count));
    return out;
}

DescriptorSnapshot snapshot_descriptor(HANDLE process, std::uintptr_t ptr) {
    DescriptorSnapshot out;
    out.node_count = read_value<std::int32_t>(process, ptr + 0x0c);
    out.split_index = read_value<std::int32_t>(process, ptr + 0x10);
    if (out.node_count < 2 || out.node_count > kMaxNodes ||
        out.split_index < 0 || out.split_index >= out.node_count)
        throw std::runtime_error("descriptor outside guarded capture domain");
    out.source_position = read_vector<std::int32_t>(process, ptr + 0x14,
        static_cast<std::size_t>(out.node_count));
    const auto intervals = static_cast<std::size_t>(out.node_count - 1);
    out.voicing = read_vector<std::int16_t>(process, ptr + 0xfb4, intervals);
    out.duration_q11 = read_vector<std::int16_t>(process, ptr + 0x1784,
        intervals);
    out.pitch_q11 = read_vector<std::int16_t>(process, ptr + 0x1f54,
        intervals);
    return out;
}

std::uintptr_t find_module_base(DWORD pid, const wchar_t* wanted) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE |
        TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateToolhelp32Snapshot modules failed");
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    std::uintptr_t base = 0;
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, wanted) == 0) {
                base = reinterpret_cast<std::uintptr_t>(me.modBaseAddr);
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return base;
}

void validate_remote_image(HANDLE process, std::uintptr_t base) {
    const auto dos = read_value<IMAGE_DOS_HEADER>(process, base);
    if (dos.e_magic != IMAGE_DOS_SIGNATURE)
        throw std::runtime_error("remote mtsyc32 has invalid DOS header");
    const auto nt = read_value<IMAGE_NT_HEADERS32>(process,
        base + static_cast<std::uintptr_t>(dos.e_lfanew));
    if (nt.Signature != IMAGE_NT_SIGNATURE ||
        nt.FileHeader.TimeDateStamp != kExpectedTimestamp ||
        nt.OptionalHeader.ImageBase != kExpectedPreferredBase ||
        nt.OptionalHeader.SizeOfImage != kExpectedImageSize)
        throw std::runtime_error("remote mtsyc32 image identifiers mismatch");
}

BYTE read_byte(HANDLE process, std::uintptr_t address) {
    return read_value<BYTE>(process, address);
}

void write_byte(HANDLE process, std::uintptr_t address, BYTE value) {
    DWORD old_protect = 0;
    if (!VirtualProtectEx(process, reinterpret_cast<LPVOID>(address), 1,
            PAGE_EXECUTE_READWRITE, &old_protect))
        throw std::runtime_error("VirtualProtectEx failed");
    SIZE_T wrote = 0;
    const BOOL ok = WriteProcessMemory(process, reinterpret_cast<LPVOID>(address),
        &value, 1, &wrote);
    DWORD ignored = 0;
    VirtualProtectEx(process, reinterpret_cast<LPVOID>(address), 1,
        old_protect, &ignored);
    FlushInstructionCache(process, reinterpret_cast<LPCVOID>(address), 1);
    if (!ok || wrote != 1)
        throw std::runtime_error("WriteProcessMemory breakpoint failed");
}

std::vector<HANDLE> suspend_other_threads(DWORD pid, DWORD current_tid) {
    std::vector<HANDLE> handles;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return handles;
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);
    if (Thread32First(snap, &te)) {
        do {
            if (te.th32OwnerProcessID != pid || te.th32ThreadID == current_tid)
                continue;
            HANDLE h = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            if (!h) continue;
            if (SuspendThread(h) == static_cast<DWORD>(-1)) {
                CloseHandle(h);
                continue;
            }
            handles.push_back(h);
        } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
    return handles;
}

void resume_threads(std::vector<HANDLE>& handles) {
    for (HANDLE h : handles) {
        ResumeThread(h);
        CloseHandle(h);
    }
    handles.clear();
}

template <class T>
void json_array(std::ostream& os, const std::vector<T>& values) {
    os << '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i) os << ',';
        os << static_cast<long long>(values[i]);
    }
    os << ']';
}

void json_feature(std::ostream& os, const FeatureSnapshot& f) {
    os << "{\"count\":" << f.count << ",\"interval_duration\":";
    json_array(os, f.interval_duration);
    os << ",\"pitch_anchor\":";
    json_array(os, f.pitch_anchor);
    os << '}';
}

void json_descriptor(std::ostream& os, const DescriptorSnapshot& d) {
    os << "{\"node_count\":" << d.node_count
       << ",\"split_index\":" << d.split_index
       << ",\"source_position\":";
    json_array(os, d.source_position);
    os << ",\"voicing\":";
    json_array(os, d.voicing);
    os << ",\"duration_q11\":";
    json_array(os, d.duration_q11);
    os << ",\"pitch_q11\":";
    json_array(os, d.pitch_q11);
    os << '}';
}

void write_record(std::ofstream& out, DWORD tid, const PendingCall& pending,
    const FeatureSnapshot& feature_after,
    const DescriptorSnapshot& previous_after,
    const DescriptorSnapshot& next_after) {
    out << "{\"schema\":\"nicolai-m36-runtime-record-v1\""
        << ",\"sequence\":" << pending.sequence
        << ",\"thread_id\":" << tid
        << ",\"feature_before\":";
    json_feature(out, pending.feature_before);
    out << ",\"feature_after\":";
    json_feature(out, feature_after);
    out << ",\"previous_before\":";
    json_descriptor(out, pending.previous_before);
    out << ",\"previous_after\":";
    json_descriptor(out, previous_after);
    out << ",\"next_before\":";
    json_descriptor(out, pending.next_before);
    out << ",\"next_after\":";
    json_descriptor(out, next_after);
    out << "}\n";
    out.flush();
}

bool file_exists(const std::string& path) {
    if (path.empty()) return false;
    const DWORD attr = GetFileAttributesW(std::filesystem::u8path(path).c_str());
    return attr != INVALID_FILE_ATTRIBUTES &&
        (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
#include "m46_linguistic_snapshot.inc"
#include "m36_route_capture.inc"
} // namespace

#ifndef NICOLAI_CAPTURE_TEST
int nicolai_runtime_capture_main(int argc, char** argv) {
    if (argc == 6 && std::string(argv[4]) == "--linguistics") {
        try {
            return run_route_capture(static_cast<DWORD>(std::stoul(argv[1])), argv[2], argv[3],
                                     true, static_cast<DWORD>(std::stoul(argv[5])));
        } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 2; }
    }
    if (argc == 5 && std::string(argv[4]) == "--route")
        return run_route_capture(static_cast<DWORD>(std::stoul(argv[1])), argv[2], argv[3]);
    if (argc < 3 || argc > 4) {
        std::cerr << "usage: nicolai_m36_runtime_capture <ettsengine-pid> "
                     "<output.jsonl> [stop-file] [--route | --linguistics <owned-trigger-pid>]\n";
        return 2;
    }

    const DWORD pid = static_cast<DWORD>(std::stoul(argv[1]));
    const std::string output_path = argv[2];
    const std::string stop_file = argc == 4 ? argv[3] : std::string{};
    HANDLE process = nullptr;
    bool attached = false;
    std::uintptr_t entry = 0, ret = 0;
    BYTE entry_original = 0, ret_original = 0;

    try {
        SetConsoleCtrlHandler(console_handler, TRUE);
        const auto module_base = find_module_base(pid, L"mtsyc32.dll");
        if (!module_base)
            throw std::runtime_error("mtsyc32.dll is not loaded in target process");

        process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
            PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
        if (!process) throw std::runtime_error("OpenProcess failed");
        validate_remote_image(process, module_base);

        entry = module_base + kBuilderRva;
        ret = module_base + kKnownReturnRva;
        entry_original = read_byte(process, entry);
        ret_original = read_byte(process, ret);

        if (!DebugActiveProcess(pid))
            throw std::runtime_error("DebugActiveProcess failed");
        attached = true;
        DebugSetProcessKillOnExit(FALSE);

        write_byte(process, entry, 0xcc);
        write_byte(process, ret, 0xcc);

        std::ofstream out(std::filesystem::u8path(output_path), std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("cannot open capture output");

        std::map<DWORD, PendingCall> pending;
        std::map<DWORD, StepState> stepping;
        std::uint64_t sequence = 0;
        std::uint64_t records = 0;
        std::uint64_t skipped = 0;

        std::cout << "M36_RUNTIME_CAPTURE_READY pid=" << pid
                  << " module=0x" << std::hex << module_base << std::dec << "\n";
        std::cout.flush();

        while (!g_stop) {
            if (file_exists(stop_file) && pending.empty()) break;
            DEBUG_EVENT ev{};
            if (!WaitForDebugEvent(&ev, 100)) {
                if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
                throw std::runtime_error("WaitForDebugEvent failed");
            }

            DWORD continue_status = DBG_CONTINUE;
            bool should_exit = false;

            if (ev.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                const auto code = ev.u.Exception.ExceptionRecord.ExceptionCode;
                const auto exception_address = reinterpret_cast<std::uintptr_t>(
                    ev.u.Exception.ExceptionRecord.ExceptionAddress);
                HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
                    THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME,
                    FALSE, ev.dwThreadId);
                if (!thread) throw std::runtime_error("OpenThread failed");

                CONTEXT ctx{};
                ctx.ContextFlags = CONTEXT_CONTROL;
                if (!GetThreadContext(thread, &ctx)) {
                    CloseHandle(thread);
                    throw std::runtime_error("GetThreadContext failed");
                }

                if (code == EXCEPTION_BREAKPOINT && exception_address == entry) {
                    write_byte(process, entry, entry_original);
                    ctx.Eip = static_cast<DWORD>(entry);
                    ctx.EFlags |= 0x100;

                    const auto return_address =
                        read_value<std::uint32_t>(process, ctx.Esp);
                    if (return_address == static_cast<std::uint32_t>(ret)) {
                        try {
                            PendingCall call;
                            call.sequence = ++sequence;
                            call.feature_ptr = read_value<std::uint32_t>(
                                process, ctx.Esp + 4);
                            call.previous_ptr = read_value<std::uint32_t>(
                                process, ctx.Esp + 8);
                            call.next_ptr = read_value<std::uint32_t>(
                                process, ctx.Esp + 12);
                            call.feature_before = snapshot_feature(process,
                                call.feature_ptr);
                            call.previous_before = snapshot_descriptor(process,
                                call.previous_ptr);
                            call.next_before = snapshot_descriptor(process,
                                call.next_ptr);
                            pending[ev.dwThreadId] = std::move(call);
                        } catch (const std::exception&) {
                            ++skipped;
                            pending.erase(ev.dwThreadId);
                        }
                    }

                    StepState step;
                    step.breakpoint_address = entry;
                    step.suspended_threads =
                        suspend_other_threads(pid, ev.dwThreadId);
                    stepping[ev.dwThreadId] = std::move(step);
                    if (!SetThreadContext(thread, &ctx)) {
                        CloseHandle(thread);
                        throw std::runtime_error("SetThreadContext entry failed");
                    }
                } else if (code == EXCEPTION_BREAKPOINT &&
                           exception_address == ret) {
                    write_byte(process, ret, ret_original);
                    ctx.Eip = static_cast<DWORD>(ret);
                    ctx.EFlags |= 0x100;

                    auto it = pending.find(ev.dwThreadId);
                    if (it != pending.end()) {
                        try {
                            const auto feature_after = snapshot_feature(process,
                                it->second.feature_ptr);
                            const auto previous_after = snapshot_descriptor(process,
                                it->second.previous_ptr);
                            const auto next_after = snapshot_descriptor(process,
                                it->second.next_ptr);
                            write_record(out, ev.dwThreadId, it->second,
                                feature_after, previous_after, next_after);
                            ++records;
                        } catch (const std::exception&) {
                            ++skipped;
                        }
                        pending.erase(it);
                    }

                    StepState step;
                    step.breakpoint_address = ret;
                    step.suspended_threads =
                        suspend_other_threads(pid, ev.dwThreadId);
                    stepping[ev.dwThreadId] = std::move(step);
                    if (!SetThreadContext(thread, &ctx)) {
                        CloseHandle(thread);
                        throw std::runtime_error("SetThreadContext return failed");
                    }
                } else if (code == EXCEPTION_SINGLE_STEP) {
                    auto it = stepping.find(ev.dwThreadId);
                    if (it != stepping.end()) {
                        write_byte(process, it->second.breakpoint_address, 0xcc);
                        ctx.EFlags &= ~static_cast<DWORD>(0x100);
                        if (!SetThreadContext(thread, &ctx)) {
                            CloseHandle(thread);
                            throw std::runtime_error("SetThreadContext step failed");
                        }
                        resume_threads(it->second.suspended_threads);
                        stepping.erase(it);
                    } else {
                        continue_status = DBG_EXCEPTION_NOT_HANDLED;
                    }
                } else if (code == EXCEPTION_BREAKPOINT) {
                    // Initial debugger attach breakpoint or another debugger-owned
                    // breakpoint. Swallow only breakpoint exceptions; unrelated
                    // exceptions are left to the target process below.
                    continue_status = DBG_CONTINUE;
                } else {
                    continue_status = DBG_EXCEPTION_NOT_HANDLED;
                }
                CloseHandle(thread);
            } else if (ev.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
                if (ev.u.CreateProcessInfo.hFile)
                    CloseHandle(ev.u.CreateProcessInfo.hFile);
                if (ev.u.CreateProcessInfo.hThread)
                    CloseHandle(ev.u.CreateProcessInfo.hThread);
                if (ev.u.CreateProcessInfo.hProcess)
                    CloseHandle(ev.u.CreateProcessInfo.hProcess);
            } else if (ev.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
                if (ev.u.CreateThread.hThread) CloseHandle(ev.u.CreateThread.hThread);
            } else if (ev.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT) {
                if (ev.u.LoadDll.hFile) CloseHandle(ev.u.LoadDll.hFile);
            } else if (ev.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT) {
                pending.erase(ev.dwThreadId);
                auto it = stepping.find(ev.dwThreadId);
                if (it != stepping.end()) {
                    resume_threads(it->second.suspended_threads);
                    stepping.erase(it);
                }
            } else if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
                should_exit = true;
            }

            ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, continue_status);
            if (should_exit) break;
        }

        // Restore bytes before detaching. If a thread is in our single-step
        // state, its peers are manually suspended; release them first.
        for (auto& kv : stepping) resume_threads(kv.second.suspended_threads);
        write_byte(process, entry, entry_original);
        write_byte(process, ret, ret_original);
        DebugActiveProcessStop(pid);
        attached = false;

        std::cout << "{\"records\":" << records
                  << ",\"skipped\":" << skipped << "}\n";
        CloseHandle(process);
        return records ? 0 : 3;
    } catch (const std::exception& e) {
        if (process) {
            try {
                if (entry) write_byte(process, entry, entry_original);
                if (ret) write_byte(process, ret, ret_original);
            } catch (...) {}
        }
        if (attached) DebugActiveProcessStop(pid);
        if (process) CloseHandle(process);
        std::cerr << e.what() << "\n";
        return 1;
    }
}
#ifndef NICOLAI_CAPTURE_EMBEDDED
int main(int argc, char** argv) { return nicolai_runtime_capture_main(argc, argv); }
#endif
#endif
