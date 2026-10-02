#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
namespace nicolai {
struct RussianNounYoPolicyM50 {
    bool valid=false;
    std::string error;
    std::array<std::vector<std::uint8_t>,200> forms; // paradigms 3..202
};
RussianNounYoPolicyM50 parse_russian_noun_yo_policy_m50(const std::vector<std::uint8_t>&);
RussianNounYoPolicyM50 load_russian_noun_yo_policy_m50(const std::filesystem::path&);
// Only the initialized noun-owned buffers have proven membership semantics.
// Paradigms 1/2 alias FLX pointers; unsupported kind/paradigm/form is unknown.
std::optional<bool> noun_yo_form_member_m50(const RussianNounYoPolicyM50&,
    unsigned kind,unsigned paradigm,unsigned form);
}
