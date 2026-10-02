#define NICOLAI_CAPTURE_TEST
#include "../tools/nicolai_m36_runtime_capture.cpp"

namespace fs = std::filesystem;

std::string contents(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

struct Worker {
    PROCESS_INFORMATION child{};
    fs::path records, stop, log;
    Worker(const fs::path& directory) {
        fs::create_directories(directory);
        records = directory / "records.jsonl"; stop = directory / "stop"; log = directory / "worker.log";
        wchar_t exe[32768]{};
        require(GetModuleFileNameW(nullptr, exe, 32768) != 0, "fixture executable unavailable");
        std::wstring command = L"\"" + std::wstring(exe) + L"\" --worker " +
            std::to_wstring(GetCurrentProcessId()) + L" \"" + records.wstring() +
            L"\" \"" + stop.wstring() + L"\"";
        SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
        HANDLE output = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        require(output != INVALID_HANDLE_VALUE, "fixture log unavailable");
        HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_EXISTING, 0, nullptr);
        STARTUPINFOW startup{}; startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        startup.wShowWindow = SW_HIDE; startup.hStdInput = input;
        startup.hStdOutput = startup.hStdError = output;
        const BOOL launched = CreateProcessW(exe, command.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startup, &child);
        CloseHandle(output); if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
        require(launched != FALSE, "fixture worker launch failed");
    }
    void ready() {
        const auto deadline = GetTickCount64() + 5000;
        while (GetTickCount64() < deadline) {
            require(WaitForSingleObject(child.hProcess, 0) == WAIT_TIMEOUT, "fixture worker exited before ready");
            if (contents(log).find("M36_RUNTIME_CAPTURE_READY mode=linguistics") != std::string::npos) return;
            Sleep(10);
        }
        throw std::runtime_error("fixture worker readiness timeout");
    }
    void request_stop() { std::ofstream flag(stop, std::ios::binary); }
    DWORD finish() {
        require(WaitForSingleObject(child.hProcess, 5000) == WAIT_OBJECT_0, "fixture detach timeout");
        DWORD code = 0;
        require(GetExitCodeProcess(child.hProcess, &code) != FALSE, "fixture exit code unavailable");
        BOOL debugging = TRUE;
        require(CheckRemoteDebuggerPresent(GetCurrentProcess(), &debugging) && !debugging,
                "render fixture was not left detached");
        require(fs::exists(records) && fs::file_size(records) == 0, "fixture fabricated linguistic records");
        return code;
    }
    ~Worker() {
        if (!child.hProcess) return;
        request_stop();
        if (WaitForSingleObject(child.hProcess, 5000) == WAIT_TIMEOUT) {
            // This synthetic parent never loads the pinned DLL, so no INT3 can
            // have been installed. Only the retained test worker is terminated.
            TerminateProcess(child.hProcess, ERROR_CANCELLED);
            WaitForSingleObject(child.hProcess, 1000);
        }
        CloseHandle(child.hThread); CloseHandle(child.hProcess);
    }
};

void check_parent_cancellation(const fs::path& directory) {
    fs::create_directories(directory);
    wchar_t exe[32768]{};
    require(GetModuleFileNameW(nullptr, exe, 32768) != 0, "cancel fixture executable unavailable");
    std::wstring command = L"\"" + std::wstring(exe) + L"\" --cancel-parent \"" + directory.wstring() + L"\"";
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    PROCESS_INFORMATION parent{};
    require(CreateProcessW(exe, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                          nullptr, nullptr, &startup, &parent) != FALSE, "cancel fixture launch failed");
    HANDLE worker = nullptr;
    struct Cleanup {
        PROCESS_INFORMATION& parent; HANDLE& worker;
        ~Cleanup() {
            TerminateProcess(parent.hProcess, ERROR_CANCELLED);
            WaitForSingleObject(parent.hProcess, 1000);
            // Only this synthetic parent's verified worker: no pinned DLL is loaded.
            if (worker) {
                if (WaitForSingleObject(worker, 5000) == WAIT_TIMEOUT) TerminateProcess(worker, ERROR_CANCELLED);
                CloseHandle(worker);
            }
            CloseHandle(parent.hThread); CloseHandle(parent.hProcess);
        }
    } cleanup{parent, worker};
    DWORD worker_pid = 0;
    const auto deadline = GetTickCount64() + 5000;
    while (!worker_pid && GetTickCount64() < deadline) {
        std::ifstream marker(directory / "worker.pid"); marker >> worker_pid;
        if (!worker_pid) Sleep(10);
    }
    require(worker_pid != 0, "cancel fixture worker marker absent");
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    require(snapshot != INVALID_HANDLE_VALUE, "cancel fixture ownership snapshot failed");
    PROCESSENTRY32W item{}; item.dwSize = sizeof(item);
    bool owned = false;
    if (Process32FirstW(snapshot, &item)) do {
        if (item.th32ProcessID == worker_pid && item.th32ParentProcessID == parent.dwProcessId &&
            _wcsicmp(item.szExeFile, fs::path(exe).filename().c_str()) == 0) owned = true;
    } while (Process32NextW(snapshot, &item));
    CloseHandle(snapshot);
    require(owned, "cancel fixture worker not owned");
    worker = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, worker_pid);
    require(worker != nullptr, "cancel fixture worker unavailable");
    require(TerminateProcess(parent.hProcess, ERROR_CANCELLED) != FALSE, "cancel fixture parent termination failed");
    require(WaitForSingleObject(parent.hProcess, 1000) == WAIT_OBJECT_0, "cancel fixture parent did not exit");
    require(WaitForSingleObject(worker, 5000) == WAIT_OBJECT_0, "worker survived owned render cancellation");
    DWORD code = 0;
    require(GetExitCodeProcess(worker, &code) && code != 0, "cancelled capture falsely succeeded");
    require(fs::file_size(directory / "records.jsonl") == 0, "cancel fixture fabricated records");
}

int wmain(int argc, wchar_t** argv) try {
    if (argc == 5 && std::wstring(argv[1]) == L"--worker") {
        const DWORD pid = std::stoul(argv[2]);
        return run_route_capture(pid, fs::path(argv[3]).u8string(), fs::path(argv[4]).u8string(), true, pid, true);
    }
    if (argc == 3 && std::wstring(argv[1]) == L"--cancel-parent") {
        Worker worker{fs::path(argv[2])};
        worker.ready();
        { std::ofstream marker(fs::path(argv[2]) / "worker.pid"); marker << worker.child.dwProcessId; }
        Sleep(15000);
        return 0;
    }
    require(argc == 2, "untrusted module fixture argument required");
    bool hash_rejected = false;
    try { validate_file_m46(fs::path(argv[1])); }
    catch (const std::runtime_error& e) { hash_rejected = std::string(e.what()) == "unsupported_original_module_sha256"; }
    require(hash_rejected, "wrong module SHA256 accepted");
    wchar_t temp[32768]{};
    require(GetTempPathW(32768, temp) != 0, "temporary directory unavailable");
    const auto directory = fs::path(temp) / (L"Nicolai-M46-тест-" +
        std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
    struct Files { fs::path path; ~Files() { std::error_code ignored; fs::remove_all(path, ignored); } } files{directory};
    // A worker may debug only its same-EXE parent, not itself or an unrelated PID.
    for (bool render_host : {false, true}) {
        bool rejected = false;
        try { validate_owner_m46(GetCurrentProcessId(), GetCurrentProcessId(), render_host); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected, "unowned worker identity accepted");
    }
    {
        Worker worker(directory / "missing");
        worker.ready();
        require(contents(worker.log).find("hooks=waiting-for-module") != std::string::npos,
                "lazy worker required a preloaded engine");
        worker.request_stop();
        require(worker.finish() == 3, "missing original falsely succeeded");
        require(contents(worker.log).find("original module not loaded") != std::string::npos,
                "missing original diagnostic absent");
    }
    {
        Worker worker(directory / "untrusted");
        worker.ready();
        // Exercise the real LOAD_DLL event path. A same-named but wrong DLL
        // must be rejected BEFORE modifying it and must leave this parent alive.
        HMODULE module = LoadLibraryW(argv[1]);
        require(module != nullptr, "untrusted fixture failed to load");
        struct Module { HMODULE h; ~Module() { FreeLibrary(h); } } cleanup{module};
        require(worker.finish() == 1, "untrusted DLL accepted");
        const auto log = contents(worker.log);
        require(log.find("M46_HOOKS_READY") == std::string::npos,
                "breakpoints armed on an untrusted DLL");
        if (log.find("remote mtsyc32 image identifiers mismatch") == std::string::npos &&
            log.find("unsupported_original") == std::string::npos)
            throw std::runtime_error("untrusted DLL rejection missing: " + log);
    }
    check_parent_cancellation(directory / "cancel");
    std::cout << "owned render lazy attach, Unicode paths, missing-module stop, untrusted LOAD_DLL rejection and parent cancellation passed\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
