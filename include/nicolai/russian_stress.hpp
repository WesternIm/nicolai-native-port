#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace nicolai {

struct RussianStressDictionary {
    bool valid = false;
    std::string error;
    std::size_t parsed_lines = 0;
    std::size_t entries = 0;
    std::unordered_map<std::string, std::size_t> stress_vowel_by_word;
};

// Parse the legacy exc_rus.txt format used with Elan/SpeechCube Nicolai.
// The file is CP1251. In the orthographic pronunciation notation the second
// '<' marker follows the stressed vowel, e.g. <абрико<с> -> абрикОс.
RussianStressDictionary parse_exc_rus_cp1251(const std::vector<std::uint8_t>& bytes);
RussianStressDictionary load_exc_rus_cp1251(const std::filesystem::path& path);
std::optional<std::size_t> lookup_stress_vowel(
    const RussianStressDictionary& dict, const std::string& lowercase_utf8_word);

// Opt-in, limited fixed-stem -ика family fallback. Exact forms retain priority;
// missing/conflicting/end-stressed anchors and compound words are not guessed.
// This is not a general Russian morphological or legacy NLP implementation.
std::optional<std::size_t> lookup_fixed_ika_stress_m43(
    const RussianStressDictionary& dict, const std::string& lowercase_utf8_word);

} // namespace nicolai
