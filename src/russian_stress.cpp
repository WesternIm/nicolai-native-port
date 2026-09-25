#include "nicolai/russian_stress.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>

namespace nicolai {
namespace {

std::uint32_t cp1251_cp(std::uint8_t b) {
    if (b < 0x80) return b;
    if (b >= 0xC0 && b <= 0xDF) return 0x0410 + (b - 0xC0);
    if (b >= 0xE0) return 0x0430 + (b - 0xE0);
    if (b == 0xA8) return 0x0401;
    if (b == 0xB8) return 0x0451;
    // Only a small punctuation subset matters for this dictionary parser.
    if (b == 0xAB) return 0x00AB;
    if (b == 0xBB) return 0x00BB;
    return 0xFFFD;
}

void append_utf8(std::string& s, std::uint32_t cp) {
    if (cp <= 0x7F) s.push_back(static_cast<char>(cp));
    else if (cp <= 0x7FF) {
        s.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        s.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        s.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string trim_ascii(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

std::string cp1251_to_lower_utf8(const std::string& s) {
    std::string out;
    for (unsigned char ub : s) {
        auto cp = cp1251_cp(ub);
        if (cp >= 0x0410 && cp <= 0x042F) cp += 0x20;
        if (cp == 0x0401) cp = 0x0451;
        if ((cp >= 0x0430 && cp <= 0x044F) || cp == 0x0451) append_utf8(out, cp);
        else if (cp < 0x80 && (std::isalnum(static_cast<unsigned char>(cp)) || cp == '-'))
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(cp))));
    }
    return out;
}

bool is_cp1251_vowel(std::uint8_t b) {
    const auto cp = cp1251_cp(b);
    switch (cp) {
        case U'а': case U'о': case U'у': case U'ы': case U'э': case U'и':
        case U'я': case U'ё': case U'ю': case U'е':
        case U'А': case U'О': case U'У': case U'Ы': case U'Э': case U'И':
        case U'Я': case U'Ё': case U'Ю': case U'Е': return true;
        default: return false;
    }
}

std::optional<std::size_t> parse_stress_ordinal(const std::string& rhs) {
    // exc_rus orthographic entries use a leading '<' and a second '<' directly
    // after the stressed vowel. Ignore phonetic [] entries for M17.
    const auto first = rhs.find('<');
    if (first == std::string::npos) return std::nullopt;
    const auto marker = rhs.find('<', first + 1);
    if (marker == std::string::npos) return std::nullopt;
    std::size_t vowel_count = 0;
    for (std::size_t i = first + 1; i < marker; ++i)
        if (is_cp1251_vowel(static_cast<std::uint8_t>(rhs[i]))) ++vowel_count;
    if (vowel_count == 0) return std::nullopt;
    return vowel_count - 1;
}

} // namespace

RussianStressDictionary parse_exc_rus_cp1251(const std::vector<std::uint8_t>& bytes) {
    RussianStressDictionary d;
    std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    std::size_t pos = 0;
    while (pos <= text.size()) {
        auto end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        pos = end + 1;
        ++d.parsed_lines;
        auto t = trim_ascii(line);
        if (t.empty() || t.rfind("//", 0) == 0) continue;
        const auto colon = t.find(':');
        if (colon == std::string::npos) continue;
        auto lhs = trim_ascii(t.substr(0, colon));
        auto rhs = t.substr(colon + 1);
        // M17 only indexes single-word exceptions. Multiword exceptions can be
        // added later without changing the file parser.
        if (lhs.find_first_of(" \t") != std::string::npos) continue;
        const auto stress = parse_stress_ordinal(rhs);
        if (!stress) continue;
        auto word = cp1251_to_lower_utf8(lhs);
        if (word.empty()) continue;
        d.stress_vowel_by_word[word] = *stress;
    }
    d.entries = d.stress_vowel_by_word.size();
    d.valid = d.entries != 0;
    if (!d.valid) d.error = "no_stress_entries_parsed";
    return d;
}

RussianStressDictionary load_exc_rus_cp1251(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        RussianStressDictionary d;
        d.error = "cannot_open_exc_rus";
        return d;
    }
    std::vector<std::uint8_t> b((std::istreambuf_iterator<char>(f)), {});
    return parse_exc_rus_cp1251(b);
}

std::optional<std::size_t> lookup_stress_vowel(
    const RussianStressDictionary& dict, const std::string& lowercase_utf8_word) {
    const auto it = dict.stress_vowel_by_word.find(lowercase_utf8_word);
    if (it == dict.stress_vowel_by_word.end()) return std::nullopt;
    return it->second;
}

} // namespace nicolai
