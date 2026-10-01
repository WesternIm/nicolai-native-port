#pragma once

#include "nicolai/russian_stress.hpp"
#include "nicolai/russian_lexicon_m44.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace nicolai {

struct FrontendWord {
    std::string source_utf8;
    std::vector<std::string> phones;
    int stress_vowel_index = -1;
    std::string stress_source; // explicit / yo / dictionary / legacy-lexicon-m44 / dictionary-ika-m43 / builtin / heuristic
};

enum class FrontendBoundaryKind {
    Word,
    Comma,
    Semicolon,
    Colon,
    Hyphen,
    Dash,
    Sentence,
    Question,
    Exclamation,
    Other
};

struct FrontendBoundary {
    std::size_t phone_index = 0; // index of the # token after the word
    FrontendBoundaryKind kind = FrontendBoundaryKind::Word;
};

struct RussianFrontendResult {
    bool valid = false;
    std::string error;
    std::string normalized_utf8;
    std::vector<FrontendWord> words;
    std::vector<std::string> phones; // includes # word boundaries
    std::vector<FrontendBoundary> boundaries; // punctuation/word-break metadata for # tokens
};

struct RussianFrontendOptions {
    const RussianStressDictionary* stress_dictionary = nullptr;
    bool use_builtin_stress_fallback = true;
    bool enable_vowel_reduction = true;
    bool enable_consonant_assimilation = true;
    bool enable_fixed_ika_stress_m43 = false;
    const RussianLexiconM44* lexicon_m44 = nullptr; // null keeps all old profiles unchanged
};

// M17 independent Russian grapheme-to-phone frontend. It now resolves lexical
// stress (explicit acute / ё / optional legacy exc_rus.txt / small fallback),
// selects Nicolai's full vs reduced vowel variants, and applies a conservative
// obstruent voicing/devoicing pass. It is still not claimed bit-identical with
// SpeechCube's proprietary Russian NLP/prosody.
RussianFrontendResult russian_text_to_nicolai_phones(
    const std::string& utf8_text,
    const RussianFrontendOptions& options = {});

} // namespace nicolai
