// Private, bounded intermediate snapshots. Included inside the debugger's
// anonymous namespace. No proprietary records belong in version control.
constexpr std::size_t kMorphStrideM46 = 0x59c;
constexpr int kMaxWordsM46 = 256;
// DebugBreakProcess issues a remote break-in thread during safe detachment.
constexpr DWORD kLinguisticAccessM46 = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ |
    PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD | PROCESS_TERMINATE;

void close_debug_image_file_m46(const DEBUG_EVENT& event) {
    // Debug process/thread handles are owned by the Windows event lifecycle.
    // Closing them at CREATE can make DebugActiveProcessStop fail (error 5).
    if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT && event.u.CreateProcessInfo.hFile)
        CloseHandle(event.u.CreateProcessInfo.hFile);
    if (event.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT && event.u.LoadDll.hFile)
        CloseHandle(event.u.LoadDll.hFile);
}

std::string read_string_m46(HANDLE process, std::uintptr_t address, std::size_t limit) {
    if (!address) throw std::runtime_error("null linguistic string");
    std::string out;
    for (std::size_t i = 0; i < limit; ++i) {
        const auto byte = read_value<unsigned char>(process, address + i);
        if (!byte) return out;
        out += static_cast<char>(byte);
    }
    throw std::runtime_error("unterminated linguistic string");
}

std::string hex_m46(const std::vector<unsigned char>& bytes) {
    const char digits[] = "0123456789abcdef";
    std::string result;
    for (auto byte : bytes) {
        result += digits[byte >> 4]; result += digits[byte & 15];
    }
    return result;
}
std::string hex_m46(const std::string& bytes) {
    return hex_m46(std::vector<unsigned char>(bytes.begin(), bytes.end()));
}

void validate_owner_m46(DWORD pid, DWORD parent_pid, bool render_host = false) {
    if (!pid || !parent_pid || ((pid == parent_pid) != render_host))
        throw std::runtime_error("invalid owned-process identity");
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot inspect process owner");
    PROCESSENTRY32W item{}; item.dwSize = sizeof(item);
    bool found = false, worker_owned = false;
    if (Process32FirstW(snap, &item)) do {
        if (item.th32ProcessID == GetCurrentProcessId() && item.th32ParentProcessID == parent_pid)
            worker_owned = true;
        if (item.th32ProcessID == pid && (render_host ||
            (item.th32ParentProcessID == parent_pid &&
             _wcsicmp(item.szExeFile, L"ettsengine.exe") == 0))) found = true;
    } while (Process32NextW(snap, &item));
    CloseHandle(snap);
    HANDLE parent = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, parent_pid);
    if (!parent) throw std::runtime_error("owned trigger is unavailable");
    const bool alive = WaitForSingleObject(parent, 0) == WAIT_TIMEOUT;
    HANDLE target = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    FILETIME parent_start{}, child_start{}, unused_a{}, unused_b{}, unused_c{};
    const bool order = target && GetProcessTimes(parent, &parent_start, &unused_a, &unused_b, &unused_c) &&
        GetProcessTimes(target, &child_start, &unused_a, &unused_b, &unused_c) &&
        (render_host ? CompareFileTime(&child_start, &parent_start) == 0 :
                       CompareFileTime(&child_start, &parent_start) >= 0);
    bool same_executable = !render_host;
    if (render_host && target) {
        wchar_t target_path[32768]{}, worker_path[32768]{};
        DWORD length = 32768;
        same_executable = QueryFullProcessImageNameW(target, 0, target_path, &length) &&
            GetModuleFileNameW(nullptr, worker_path, 32768) &&
            _wcsicmp(target_path, worker_path) == 0;
    }
    if (target) CloseHandle(target);
    CloseHandle(parent);
    if (!found || !worker_owned || !alive || !order || !same_executable)
        throw std::runtime_error("target is not an owned server or the worker's same-EXE render parent");
}

void validate_file_m46(const std::filesystem::path& path) {
    if (path.empty() || nicolai::local_original::sha256(path) !=
        "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7")
        throw std::runtime_error("unsupported_original_module_sha256");
}

void validate_file_m46(DWORD pid) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) throw std::runtime_error("cannot verify original module file");
    MODULEENTRY32W item{}; item.dwSize = sizeof(item);
    std::filesystem::path path;
    if (Module32FirstW(snapshot, &item)) do {
        if (_wcsicmp(item.szModule, L"mtsyc32.dll") == 0) path = item.szExePath;
    } while (Module32NextW(snapshot, &item));
    CloseHandle(snapshot);
    validate_file_m46(path);
}

std::filesystem::path debug_image_path_m46(HANDLE file) {
    if (!file || file == INVALID_HANDLE_VALUE) return {};
    wchar_t path[32768]{};
    const auto length = GetFinalPathNameByHandleW(file, path, 32768, FILE_NAME_NORMALIZED);
    if (!length || length >= 32768) return {};
    return std::filesystem::path(path);
}

void json_linguistic_state_m46(std::ostream& destination, HANDLE process,
                              std::uintptr_t state, bool source_records) {
    const auto count = read_value<std::int32_t>(process, state + 0x134);
    if (count < 1 || count > kMaxWordsM46)
        throw std::runtime_error("linguistic word count outside guarded domain");
    const auto morph = read_value<std::uint32_t>(process, state + 0x124);
    const auto raw = read_value<std::uint32_t>(process, state + 0x128);
    const auto codes = read_value<std::uint32_t>(process, state + 0x12c);
    const auto words = read_value<std::uint32_t>(process, state + 0x13c);
    const auto descriptors = source_records ? read_value<std::uint32_t>(process, state + 0x16c) : 0;
    if (!morph || !raw || !codes || !words || (source_records && !descriptors))
        throw std::runtime_error("null linguistic array");
    // Stage even for unit callers: a rejected read must never leave partial JSON.
    std::ostringstream out;
    out << "{\"word_count\":" << count << ",\"words\":[";
    int total_records = 0;
    for (int i = 1; i <= count; ++i) {
        if (i > 1) out << ',';
        const auto word = read_value<std::uint32_t>(process, words + i * 4);
        const auto block = morph + i * kMorphStrideM46;
        const auto candidates = read_value<unsigned char>(process, block);
        if (candidates > 70) throw std::runtime_error("morphology count outside block capacity");
        out << "{\"index\":" << i << ",\"annotated_cp866_hex\":\""
            << hex_m46(read_string_m46(process, word, 4096)) << "\",\"punctuation_cp866_hex\":\""
            << hex_m46(read_string_m46(process, block + 0x590, 12))
            << "\",\"raw_separator\":" << static_cast<int>(read_value<unsigned char>(process, raw + i))
            << ",\"code_hex\":\"" << hex_m46(read_string_m46(process, codes + i * 4, 4))
            << "\",\"candidate_count\":" << static_cast<int>(candidates) << ",\"candidates_hex\":[";
        for (int c = 0; c < candidates; ++c) {
            if (c) out << ',';
            out << '\"' << hex_m46(read_vector<unsigned char>(process, block + (c + 1) * 20, 20)) << '\"';
        }
        out << ']';
        if (source_records) {
            const auto descriptor = descriptors + i * 12;
            const auto records = read_value<std::uint32_t>(process, descriptor);
            const auto phones = read_value<unsigned char>(process, descriptor + 4);
            total_records += phones;
            if (phones > 128 || total_records > 4096 || (phones && !records))
                throw std::runtime_error("source phone records outside guarded domain");
            out << ",\"source_phone_count\":" << static_cast<int>(phones) << ",\"source_phone_records_hex\":[";
            for (int p = 0; p < phones; ++p) {
                if (p) out << ',';
                out << '\"' << hex_m46(read_vector<unsigned char>(process, records + p * 32, 32)) << '\"';
            }
            out << ']';
        }
        out << '}';
    }
    out << "]}";
    destination << out.str();
}
