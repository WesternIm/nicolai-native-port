// Included in the talker's anonymous namespace. Opt-in, one-EXE diagnostics
// for THIS render or its original server child, never a shared process.
#if !defined(_WIN64)
struct OriginalCaptureM46 {
    HANDLE worker = nullptr, server = nullptr;
    bool server_owned = false;
    fs::path stop, log;

    static DWORD owned_server() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot inspect original server ownership");
        PROCESSENTRY32W item{}; item.dwSize = sizeof(item);
        DWORD found = 0;
        if (Process32FirstW(snapshot, &item)) do {
            if (_wcsicmp(item.szExeFile, L"ettsengine.exe") == 0) {
                if (item.th32ParentProcessID != GetCurrentProcessId()) {
                    CloseHandle(snapshot);
                    throw std::runtime_error("shared original servers are not instrumented");
                }
                if (found) { CloseHandle(snapshot); throw std::runtime_error("multiple owned original servers"); }
                found = item.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &item));
        CloseHandle(snapshot);
        // The supported S5 wrapper starts its out-of-process core in SetOutput.
        // The caller has bound output but has NOT submitted text yet. Attaching
        // to our render instead would never observe that server's core.
        if (!found) throw std::runtime_error("no owned original server after SAPI output binding");
        return found;
    }

    static void require_no_server() {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot inspect existing original servers");
        PROCESSENTRY32W item{}; item.dwSize = sizeof(item);
        bool existing = false;
        if (Process32FirstW(snapshot, &item)) do {
            if (_wcsicmp(item.szExeFile, L"ettsengine.exe") == 0) existing = true;
        } while (Process32NextW(snapshot, &item));
        CloseHandle(snapshot);
        if (existing) throw std::runtime_error("M46 trace requires a fresh original session; existing servers are left untouched");
    }

    void signal_stop() noexcept {
        if (stop.empty()) return;
        HANDLE flag = CreateFileW(stop.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (flag != INVALID_HANDLE_VALUE) CloseHandle(flag);
    }
    void cleanup() noexcept {
        if (worker && WaitForSingleObject(worker, 0) == WAIT_TIMEOUT) {
            signal_stop();
            if (WaitForSingleObject(worker, 4000) == WAIT_TIMEOUT) {
                // Never strand INT3 patches in a live server if the worker
                // cannot detach. Only this render or its verified child is owned.
                if (server_owned) { TerminateProcess(server, ERROR_CANCELLED); WaitForSingleObject(server, 1000); }
                if (WaitForSingleObject(worker, 1000) == WAIT_TIMEOUT) TerminateProcess(worker, ERROR_CANCELLED);
            }
        }
        if (worker) { CloseHandle(worker); worker = nullptr; }
        // This diagnostic demanded a fresh session and retained our direct
        // child before attaching. Release SAPI first (caller declaration order),
        // then reap only that retained server; never leave it shared for a later
        // test, nor search for a replacement PID during cleanup.
        if (server_owned && WaitForSingleObject(server, 250) == WAIT_TIMEOUT) {
            TerminateProcess(server, ERROR_CANCELLED);
            WaitForSingleObject(server, 1000);
        }
        if (server) { CloseHandle(server); server = nullptr; }
        server_owned = false;
    }
    explicit OriginalCaptureM46(const fs::path& directory) {
        try {
            const DWORD pid = owned_server();
            server = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
            if (!server) throw std::runtime_error("cannot retain owned original server handle");
            FILETIME parent_start{}, child_start{}, a{}, b{}, c{};
            if (!GetProcessTimes(GetCurrentProcess(), &parent_start, &a, &b, &c) ||
                !GetProcessTimes(server, &child_start, &a, &b, &c) ||
                CompareFileTime(&child_start, &parent_start) < 0)
                throw std::runtime_error("original server predates this render process");
            if (owned_server() != pid || WaitForSingleObject(server, 0) != WAIT_TIMEOUT)
                throw std::runtime_error("original server ownership changed");
            server_owned = true;
            stop = directory / "linguistics-m46.stop";
            log = directory / "linguistics-m46-worker.log";
            const auto records = directory / "linguistics-m46.jsonl";
            for (const auto& path : {stop, log, records})
                if (fs::exists(path)) throw std::runtime_error("M46 diagnostic outputs must be fresh");
            SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
            HANDLE output = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (output == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot create M46 worker log");
            HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       &security, OPEN_EXISTING, 0, nullptr);
            const auto exe = executable();
            auto command = quote(exe.wstring()) + L" --m46-capture-worker " + std::to_wstring(pid) +
                L" " + quote(records.wstring()) + L" " + quote(stop.wstring()) +
                L" --linguistics " +
                std::to_wstring(GetCurrentProcessId());
            STARTUPINFOW startup{}; startup.cb = sizeof(startup);
            startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            startup.wShowWindow = SW_HIDE; startup.hStdOutput = startup.hStdError = output;
            startup.hStdInput = input;
            PROCESS_INFORMATION child{};
            const BOOL started = CreateProcessW(exe.c_str(), command.data(), nullptr, nullptr, TRUE,
                CREATE_NO_WINDOW, nullptr, exe.parent_path().c_str(), &startup, &child);
            CloseHandle(output); if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
            if (!started) throw std::runtime_error("cannot launch embedded M46 capture worker");
            worker = child.hProcess; CloseHandle(child.hThread);
            const auto deadline = GetTickCount64() + 7000;
            while (GetTickCount64() < deadline) {
                if (WaitForSingleObject(worker, 0) != WAIT_TIMEOUT)
                    throw std::runtime_error("M46 worker exited before READY; see linguistics-m46-worker.log");
                const auto status = read_utf8(log);
                if (status.find("M36_RUNTIME_CAPTURE_READY mode=linguistics") != std::string::npos &&
                    status.find("hooks=ready") != std::string::npos) return;
                Sleep(20);
            }
            throw std::runtime_error("M46 worker READY timeout");
        } catch (...) { cleanup(); throw; }
    }
    void finish() {
        signal_stop();
        if (WaitForSingleObject(worker, 7000) != WAIT_OBJECT_0)
            throw std::runtime_error("M46 worker detach timeout");
        DWORD code = 1;
        if (!GetExitCodeProcess(worker, &code) || code != 0)
            throw std::runtime_error("M46 capture incomplete; see linguistics-m46-worker.log");
    }
    ~OriginalCaptureM46() { cleanup(); }
};
#endif
