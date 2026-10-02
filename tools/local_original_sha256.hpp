#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <array>
#include <fstream>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace nicolai::local_original {
inline std::string sha256(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("cannot_read_original_module");
    HCRYPTPROV provider = 0; HCRYPTHASH hash = 0;
    struct Guard {
        HCRYPTPROV& provider; HCRYPTHASH& hash;
        ~Guard() { if (hash) CryptDestroyHash(hash); if (provider) CryptReleaseContext(provider, 0); }
    } guard{provider, hash};
    if (!CryptAcquireContextW(&provider, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
        !CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash))
        throw std::runtime_error("cannot_create_original_hash");
    std::array<char, 65536> buffer{};
    while (file.read(buffer.data(), buffer.size()) || file.gcount()) {
        if (!CryptHashData(hash, reinterpret_cast<const BYTE*>(buffer.data()),
                           static_cast<DWORD>(file.gcount()), 0))
            throw std::runtime_error("cannot_hash_original_module");
    }
    if (!file.eof()) throw std::runtime_error("cannot_read_original_module");
    std::array<BYTE, 32> digest{}; DWORD size = static_cast<DWORD>(digest.size());
    if (!CryptGetHashParam(hash, HP_HASHVAL, digest.data(), &size, 0) || size != digest.size())
        throw std::runtime_error("cannot_finish_original_hash");
    constexpr char hex[] = "0123456789abcdef";
    std::string result;
    for (auto byte : digest) { result += hex[byte >> 4]; result += hex[byte & 15]; }
    return result;
}
}
