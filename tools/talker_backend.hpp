#pragma once
#include "nicolai/pc_parity.hpp"
#include "nicolai/legacy_prosody.hpp"
#include <filesystem>
#include <string>

namespace nicolai::test_talker {
enum class Profile { Stable, M36Local, M36Chain, M38Boundary, M40Transient, M41Preserve, M42JoinPitch };
Profile parse_profile(const std::string& value);
const char* profile_name(Profile profile);
LegacyTimingPolicy timing_policy(Profile profile);
// Changes process-local flags only. GUI jobs run in a separate owned process.
void set_profile_environment(Profile profile);
void validate_voice_directory(const std::filesystem::path& directory);
DiphoneChainLegacyResult render(const std::filesystem::path& directory,
                               const std::string& text, Profile profile);
}
