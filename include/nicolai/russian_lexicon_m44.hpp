#pragma once

#include "nicolai/edat.hpp"
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace nicolai {

struct RussianLexiconPayloadsM44 {
    std::vector<std::uint8_t> dictionary, index, nonverb, nonverb_index,
        verb, verb_index, types;
};
struct RussianLexiconEntryM44 {
    std::string stem_cp866;
    std::vector<std::uint8_t> metadata;
    std::size_t block = 0, record = 0;
};
struct RussianLexiconM44 {
    bool valid = false;
    std::string error;
    std::size_t blocks = 0, records = 0;
    std::unordered_map<std::string, std::vector<RussianLexiconEntryM44>> stems;
    std::array<std::vector<std::string>, 256> nonverb_forms, verb_forms;
    std::array<std::uint8_t, 3680> types{};
};
struct RussianLexiconStressM44 {
    std::optional<std::size_t> stress_vowel;
    std::size_t candidates = 0;
    std::string status = "no-match";
};

// Pure, bounded resource readers. Never call or redistribute the original DLL.
RussianLexiconM44 parse_russian_lexicon_payloads_m44(const RussianLexiconPayloadsM44&);
RussianLexiconM44 parse_russian_lexicon_m44(
    const std::vector<std::uint8_t>&, const EdatLayout&);

// Partial analysis: supported noun forms and non-reflexive main verb forms,
// '+' (stem-stress) only. Known competing forms can veto the result. No guessed
// ending-stress, context disambiguation, reflexive or adjective integration.
RussianLexiconStressM44 lookup_russian_lexicon_stress_m44(
    const RussianLexiconM44&, const std::string& lowercase_utf8_word);

struct RussianExactCandidateM47 {
    std::uint8_t stress = 0, auxiliary = 0, kind = 0, tag = 0, form = 0;
};
// The four-byte exact-entry lane. Unmapped/reserved selectors are refused.
std::optional<RussianExactCandidateM47> decode_russian_exact_candidate_m47(
    const std::vector<std::uint8_t>& metadata);
// Opt-in extension: exact entries and '+' adjective/short-form candidates.
// Accept only a unanimous, fully supported stress; no contextual disambiguation
// or ending-stress authoring. The M44 entry point remains unchanged.
RussianLexiconStressM44 lookup_russian_lexicon_stress_m47(
    const RussianLexiconM44&, const std::string& lowercase_utf8_word);

} // namespace nicolai
