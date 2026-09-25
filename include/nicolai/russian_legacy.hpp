#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace nicolai {

struct LegacyAbbreviationDictionary {
    bool valid = false;
    std::string error;
    std::size_t parsed_lines = 0;
    std::size_t entries = 0;
    std::vector<std::pair<std::string,std::string>> replacements; // UTF-8, longest key first
};

struct LegacyExceptionDictionary {
    bool valid = false;
    std::string error;
    std::size_t parsed_lines = 0;
    std::size_t entries = 0;
    std::size_t multiword_entries = 0;
    std::unordered_map<std::string,std::string> single_word; // lowercase key -> marked pronunciation
    std::vector<std::pair<std::string,std::string>> multiword; // lowercase key -> marked pronunciation
};

LegacyAbbreviationDictionary parse_abb_rus_cp1251(const std::vector<std::uint8_t>& bytes);
LegacyAbbreviationDictionary load_abb_rus_cp1251(const std::filesystem::path& path);
LegacyExceptionDictionary parse_exc_rus_replacements_cp1251(const std::vector<std::uint8_t>& bytes);
LegacyExceptionDictionary load_exc_rus_replacements_cp1251(const std::filesystem::path& path);

struct LegacyNormalizationResult {
    bool valid = false;
    std::string error;
    std::string normalized_utf8;
    std::size_t abbreviation_replacements = 0;
    std::size_t exception_replacements = 0;
    std::size_t number_replacements = 0;
};

// PC-compatibility text normalization layer. It applies the original abb_rus
// expansion syntax, full orthographic exception replacements from exc_rus
// (including up to five-word phrases), and deterministic cardinal integer
// expansion for otherwise-unhandled decimal integers. Legacy '<' / '`' stress
// marks in dictionary pronunciations are translated to U+0301.
LegacyNormalizationResult normalize_russian_legacy_text(
    const std::string& utf8_text,
    const LegacyAbbreviationDictionary* abbreviations = nullptr,
    const LegacyExceptionDictionary* exceptions = nullptr);

} // namespace nicolai
