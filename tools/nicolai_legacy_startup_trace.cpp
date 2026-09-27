// Bounded, non-patching startup diagnostics for an owned legacy child process.
#if !defined(_WIN32) || defined(_WIN64)
#error nicolai_legacy_startup_trace requires Win32 x86
#endif
#include <windows.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void close(HANDLE h) { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); }
std::string utf8(const std::wstring& s) {
    const int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
        nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    if (n) WideCharToMultiByte(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
        out.data(), n, nullptr, nullptr);
    return out;
}
std::string json(const std::string& s) {
    const char hex[] = "0123456789abcdef";
    std::string out = "\"";
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if (c < 32) { out += "\\u00"; out += hex[c >> 4]; out += hex[c & 15]; }
        else out += c;
    }
    return out + '"';
}
std::string path(HANDLE file) {
    std::vector<wchar_t> buf(32768);
    const DWORD n = GetFinalPathNameByHandleW(file, buf.data(),
        static_cast<DWORD>(buf.size()), FILE_NAME_NORMALIZED);
    return n && n < buf.size() ? utf8(std::wstring(buf.data(), n)) : "";
}
std::string debug_text(HANDLE process, const OUTPUT_DEBUG_STRING_INFO& info) {
    const SIZE_T size = std::min<SIZE_T>(info.nDebugStringLength, 8192) *
        (info.fUnicode ? sizeof(wchar_t) : 1);
    std::vector<char> buf(size + sizeof(wchar_t), 0);
    SIZE_T got = 0;
    if (!ReadProcessMemory(process, info.lpDebugStringData, buf.data(), size, &got))
        return "<unreadable>";
    if (info.fUnicode) return utf8(std::wstring(reinterpret_cast<wchar_t*>(buf.data())));
    const int n = MultiByteToWideChar(CP_ACP, 0, buf.data(), -1, nullptr, 0);
    std::wstring wide(n, L'\0');
    if (n) MultiByteToWideChar(CP_ACP, 0, buf.data(), -1, wide.data(), n);
    if (!wide.empty()) wide.pop_back();
    return utf8(wide);
}
}

int wmain(int argc, wchar_t** argv) {
    if (argc < 4 || argc > 5) {
        std::cerr << "usage: nicolai_legacy_startup_trace <exe> <fresh-output.jsonl> "
                     "<timeout-seconds:1..60> [arguments]\n";
        return 2;
    }
    PROCESS_INFORMATION pi{};
    bool running = false;
    try {
        const unsigned timeout = std::stoul(argv[3]);
        if (timeout < 1 || timeout > 60) throw std::runtime_error("timeout outside 1..60");
        HANDLE output = CreateFileW(argv[2], GENERIC_WRITE, 0, nullptr, CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (output == INVALID_HANDLE_VALUE)
            throw std::runtime_error("output must be a new writable file");
        auto emit = [&](const std::string& row) {
            const std::string line = row + '\n';
            DWORD written = 0;
            if (!WriteFile(output, line.data(), static_cast<DWORD>(line.size()),
                    &written, nullptr) || written != line.size())
                throw std::runtime_error("trace write failed");
            FlushFileBuffers(output);
        };
        // Close the trace on every exception, including CreateProcess failure.
        struct OutputGuard { HANDLE h; ~OutputGuard() { close(h); } } guard{output};
        std::wstring exe = argv[1];
        const auto slash = exe.find_last_of(L"\\/");
        if (slash == std::wstring::npos) throw std::runtime_error("exe must be an absolute path");
        std::wstring directory = exe.substr(0, slash);
        std::wstring command = L"\"" + exe + L"\"";
        if (argc == 5) command += L" " + std::wstring(argv[4]);
        STARTUPINFOW si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        if (!CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, FALSE,
                DEBUG_ONLY_THIS_PROCESS | CREATE_NO_WINDOW, nullptr,
                directory.c_str(), &si, &pi))
            throw std::runtime_error("CreateProcess failed: " + std::to_string(GetLastError()));
        running = true;
        DebugSetProcessKillOnExit(FALSE);
        emit("{\"event\":\"start\",\"pid\":" + std::to_string(pi.dwProcessId) + "}");
        const ULONGLONG start = GetTickCount64();
        bool timed_out = false, initial_break = false;
        DWORD child_exit = 0;
        while (running) {
            if (!timed_out && GetTickCount64() - start >= timeout * 1000ull) {
                timed_out = true;
                emit("{\"event\":\"timeout\"}");
                // This tool owns only this child, never an existing server.
                if (!TerminateProcess(pi.hProcess, ERROR_TIMEOUT))
                    throw std::runtime_error("owned child termination failed");
            }
            DEBUG_EVENT ev{};
            if (!WaitForDebugEvent(&ev, 100)) {
                if (GetLastError() == ERROR_SEM_TIMEOUT) continue;
                throw std::runtime_error("WaitForDebugEvent failed");
            }
            DWORD status = DBG_CONTINUE;
            const std::string prefix = "{\"ms\":" +
                std::to_string(GetTickCount64() - start) + ",\"tid\":" +
                std::to_string(ev.dwThreadId);
            switch (ev.dwDebugEventCode) {
            case CREATE_PROCESS_DEBUG_EVENT:
                emit(prefix + ",\"event\":\"image\",\"path\":" + json(path(ev.u.CreateProcessInfo.hFile)) + "}");
                close(ev.u.CreateProcessInfo.hFile);
                close(ev.u.CreateProcessInfo.hThread);
                close(ev.u.CreateProcessInfo.hProcess);
                break;
            case CREATE_THREAD_DEBUG_EVENT: close(ev.u.CreateThread.hThread); break;
            case LOAD_DLL_DEBUG_EVENT:
                emit(prefix + ",\"event\":\"module\",\"base\":" +
                    std::to_string(reinterpret_cast<uintptr_t>(ev.u.LoadDll.lpBaseOfDll)) +
                    ",\"path\":" + json(path(ev.u.LoadDll.hFile)) + "}");
                close(ev.u.LoadDll.hFile); break;
            case OUTPUT_DEBUG_STRING_EVENT:
                emit(prefix + ",\"event\":\"debug\",\"text\":" +
                    json(debug_text(pi.hProcess, ev.u.DebugString)) + "}"); break;
            case EXCEPTION_DEBUG_EVENT: {
                const auto& ex = ev.u.Exception;
                emit(prefix + ",\"event\":\"exception\",\"code\":" +
                    std::to_string(ex.ExceptionRecord.ExceptionCode) + ",\"address\":" +
                    std::to_string(reinterpret_cast<uintptr_t>(ex.ExceptionRecord.ExceptionAddress)) +
                    ",\"first_chance\":" + std::to_string(ex.dwFirstChance) + "}");
                if (ex.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT && !initial_break)
                    initial_break = true;
                else status = DBG_EXCEPTION_NOT_HANDLED;
                break;
            }
            case EXIT_PROCESS_DEBUG_EVENT:
                child_exit = ev.u.ExitProcess.dwExitCode;
                emit(prefix + ",\"event\":\"exit\",\"code\":" + std::to_string(child_exit) + "}");
                running = false; break;
            }
            if (!ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, status))
                throw std::runtime_error("ContinueDebugEvent failed");
        }
        close(pi.hThread); close(pi.hProcess);
        std::cout << "startup trace: exit=" << child_exit << " timeout=" << timed_out << '\n';
        return timed_out ? 3 : (child_exit == 0 ? 0 : 4);
    } catch (const std::exception& e) {
        if (running) {
            TerminateProcess(pi.hProcess, ERROR_CANCELLED);
            DebugActiveProcessStop(pi.dwProcessId);
            WaitForSingleObject(pi.hProcess, 5000);
        }
        close(pi.hThread); close(pi.hProcess);
        std::cerr << e.what() << '\n';
        return 1;
    }
}
