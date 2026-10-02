#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
namespace nicolai {
struct RussianYoPolicyM49 {
    bool valid=false;
    std::string error;
    std::array<std::vector<std::uint8_t>,8> paradigms,types;
};
// Private exported data, not an executable module. Payloads are never bundled.
RussianYoPolicyM49 parse_russian_yo_policy_m49(const std::vector<std::uint8_t>&);
RussianYoPolicyM49 load_russian_yo_policy_m49(const std::filesystem::path&);
// nullopt refuses an unknown noun-form filter. A false global membership or
// an explicit original form exclusion proves "keep е" without that filter.
std::optional<bool> select_russian_yo_m49(const RussianYoPolicyM49&, unsigned kind,
    unsigned paradigm,unsigned type,unsigned form,std::optional<bool> noun_form_member=std::nullopt);
struct RussianEndingChoiceM49 {
    std::size_t stress_vowel=0;
    std::optional<std::size_t> yo_letter_index; // zero-based, marker removed
};
std::optional<RussianEndingChoiceM49> decode_russian_ending_choice_m49(
    const std::string& stem_cp866,const std::string& suffix_variant_cp866,
    const RussianYoPolicyM49&,unsigned kind,unsigned paradigm,unsigned type,unsigned form,
    std::optional<bool> noun_form_member=std::nullopt);
}
