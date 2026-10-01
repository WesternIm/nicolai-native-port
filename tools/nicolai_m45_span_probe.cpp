// Optional, isolated x86 oracle for one inspected original span function.
// No DLL entry point, SAPI server, voice database or real user text is used.
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr std::size_t stride = 0x59c;
constexpr const char* supported_sha =
    "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7";

std::string sha256(const char* path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("cannot_read_original");
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    struct Guard {
        BCRYPT_ALG_HANDLE& algorithm; BCRYPT_HASH_HANDLE& hash;
        ~Guard() { if (hash) BCryptDestroyHash(hash);
            if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
    } guard{algorithm, hash};
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0 ||
        BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) < 0)
        throw std::runtime_error("cannot_create_sha256");
    std::array<char, 65536> chunk{};
    while (file.read(chunk.data(), chunk.size()) || file.gcount()) {
        if (BCryptHashData(hash, reinterpret_cast<PUCHAR>(chunk.data()),
                           static_cast<ULONG>(file.gcount()), 0) < 0)
            throw std::runtime_error("cannot_hash_original");
    }
    if (!file.eof()) throw std::runtime_error("cannot_read_original");
    std::array<unsigned char, 32> digest{};
    if (BCryptFinishHash(hash, digest.data(), digest.size(), 0) < 0)
        throw std::runtime_error("cannot_finish_sha256");
    std::ostringstream result;
    for (const auto byte : digest) result << std::hex << std::setw(2)
                                        << std::setfill('0') << static_cast<int>(byte);
    return result.str();
}

// Guard both ends of every caller-owned array; never hand the original a
// vector which may reallocate while its function is running.
struct Buffer {
    std::vector<unsigned char> bytes;
    explicit Buffer(std::size_t size) : bytes(size + 64, 0xa5) {
        std::fill(bytes.begin() + 32, bytes.end() - 32, 0);
    }
    unsigned char* data() { return bytes.data() + 32; }
    void check() const {
        if (!std::all_of(bytes.begin(), bytes.begin() + 32, [](auto v) { return v == 0xa5; }) ||
            !std::all_of(bytes.end() - 32, bytes.end(), [](auto v) { return v == 0xa5; }))
            throw std::runtime_error("original_crossed_buffer_guard");
    }
};
template<class T> void put(Buffer& buffer, std::size_t at, T value) {
    std::memcpy(buffer.data() + at, &value, sizeof(value));
}

void bind_crt(unsigned char* base, HMODULE crt, std::size_t rva, const char* name) {
    const auto function = GetProcAddress(crt, name);
    if (!function) throw std::runtime_error("missing_inspected_crt_import");
    DWORD previous = 0, ignored = 0;
    auto* slot = base + rva;
    if (!VirtualProtect(slot, sizeof(function), PAGE_READWRITE, &previous))
        throw std::runtime_error("cannot_bind_inspected_crt_import");
    std::memcpy(slot, &function, sizeof(function));
    if (!VirtualProtect(slot, sizeof(function), previous, &ignored))
        throw std::runtime_error("cannot_restore_iat_protection");
}

using Split = void (__cdecl*)(void*);
void run_case(Split split, const std::string& name, int count, char seed,
              bool marked, bool candidates, int punctuation = 0, char punct = ' ',
              int grammar_kind = 0, int next_kind = 0, int next_form = 0) {
    if (count < 1 || count > 32) throw std::runtime_error("invalid_synthetic_count");
    const auto slots = static_cast<std::size_t>(count + 2);
    Buffer state(0x3c00), morph(slots * stride), raw(slots), codes(slots * 4),
           words(slots * 96), pointers(slots * sizeof(char*));
    for (int i = 0; i < count + 2; ++i) {
        auto* word = reinterpret_cast<char*>(words.data() + i * 96);
        // Invented ASCII token, not a lexical entry copied from the DLL.
        std::strcpy(word, marked ? "qzx<qv" : "qzxqv");
        put(pointers, i * sizeof(char*), word);
        raw.data()[i] = static_cast<unsigned char>(seed);
        morph.data()[i * stride] = candidates ? 1 : 0;
        morph.data()[i * stride + 27] = 99; // inspected default/no-grammar branch
        morph.data()[i * stride + 0x590] = ' ';
    }
    if (punctuation) morph.data()[punctuation * stride + 0x590] = punct;
    if (grammar_kind) {
        morph.data()[stride] = morph.data()[2 * stride] = 1;
        morph.data()[stride + 27] = static_cast<unsigned char>(grammar_kind);
        morph.data()[2 * stride + 27] = static_cast<unsigned char>(next_kind);
        morph.data()[2 * stride + 29] = static_cast<unsigned char>(next_form);
    }
    put(state, 0x124, morph.data()); put(state, 0x128, raw.data());
    put(state, 0x12c, codes.data()); put(state, 0x13c, pointers.data());
    put<std::uint32_t>(state, 0x134, count);
    split(state.data());
    for (const auto* b : {&state, &morph, &raw, &codes, &words, &pointers}) b->check();
    std::cout << name << '\t' << count << '\t';
    for (int i = 1; i <= count; ++i) std::cout << static_cast<char>(raw.data()[i]);
    std::cout << '\t';
    for (int i = 1; i <= count; ++i) {
        if (i > 1) std::cout << '|';
        const auto* code = reinterpret_cast<char*>(codes.data() + i * 4);
        if (!std::memchr(code, 0, 4)) throw std::runtime_error("unterminated_boundary_code");
        std::cout << code;
    }
    std::cout << '\n';
}
}

int main(int argc, char** argv) try {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    if (argc != 2) { std::cerr << "usage: nicolai_m45_span_probe local-mtsyc32.dll\n"; return 2; }
    if (sha256(argv[1]) != supported_sha) throw std::runtime_error("unsupported_original_sha256");
    if (GetModuleHandleA("mtsyc32.dll")) throw std::runtime_error("original_already_loaded");
    // Map in this disposable process only. Never initialize the proprietary
    // DLL or execute another path with its unresolved/private imports.
    const auto module = LoadLibraryExA(argv[1], nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!module) throw std::runtime_error("cannot_map_original");
    struct ModuleGuard { HMODULE module; ~ModuleGuard() { FreeLibrary(module); } } guard{module};
    auto* base = reinterpret_cast<unsigned char*>(module);
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const unsigned char head[] = {0x81, 0xec, 0xdc, 0, 0, 0, 0x53, 0x55, 0x56, 0x57};
    if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
        nt->OptionalHeader.ImageBase != 0x10000000 ||
        nt->OptionalHeader.SizeOfImage != 0x797000 ||
        nt->FileHeader.TimeDateStamp != 0x412a0cb4 ||
        std::memcmp(base + 0x216c70, head, sizeof(head)))
        throw std::runtime_error("unsupported_original_code_fingerprint");
    // Only these three imports occur on the inspected synthetic branches.
    // The SHA guard rejects any other image before import binding/execution.
    const auto crt = LoadLibraryExA("msvcrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!crt) throw std::runtime_error("cannot_load_system_crt");
    ModuleGuard crt_guard{crt};
    bind_crt(base, crt, 0x23a154, "sprintf");
    bind_crt(base, crt, 0x23a21c, "strchr");
    bind_crt(base, crt, 0x23a220, "strncmp");
    const auto split = reinterpret_cast<Split>(base + 0x216c70);
    for (int n : {1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 22, 32}) {
        for (char seed : {' ', '_', '=', '+'}) {
            const auto suffix = std::to_string(static_cast<int>(seed)) + "_" + std::to_string(n);
            run_case(split, "empty_" + suffix, n, seed, false, false);
            run_case(split, "candidate_" + suffix, n, seed, false, true);
            run_case(split, "marked_" + suffix, n, seed, true, true);
        }
    }
    for (char punctuation : {',', ':', '(', ')', '_', '.', '!', '?'})
        run_case(split, "punct_" + std::to_string(static_cast<int>(punctuation)),
                 4, ' ', false, true, 2, punctuation);
    for (char seed : {' ', '=', '+'}) {
        const auto suffix = std::to_string(static_cast<int>(seed));
        run_case(split, "kind4_yes_" + suffix, 3, seed, false, true, 0, ' ', 4, 6, 60);
        run_case(split, "kind4_no_" + suffix, 3, seed, false, true, 0, ' ', 4, 6, 59);
        run_case(split, "kind10_yes_" + suffix, 3, seed, false, true, 0, ' ', 10, 4, 1);
        run_case(split, "kind10_no_" + suffix, 3, seed, false, true, 0, ' ', 10, 99, 1);
    }
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
