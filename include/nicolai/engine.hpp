#pragma once

#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"
#include "nicolai/voice_db.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct Pcm16Mono {
    int sample_rate = 16000;
    std::vector<std::int16_t> samples;
};

enum class SynthStatus {
    Ok,
    VoiceDatabaseNotLoaded,
    NativeCoreNotImplemented,
    InvalidInput
};

struct SynthResult {
    SynthStatus status = SynthStatus::NativeCoreNotImplemented;
    std::string message;
    Pcm16Mono pcm;
};

class Engine {
public:
    explicit Engine(VoiceDb db);
    Engine(VoiceDb db, RussianStressDictionary stress_dictionary);
    Engine(VoiceDb db,
           RussianStressDictionary stress_dictionary,
           LegacyAbbreviationDictionary abbreviation_dictionary,
           LegacyExceptionDictionary exception_dictionary);
    SynthResult synthesize(const std::string& utf8_text) const;
    const VoiceDbMetadata& voice_metadata() const noexcept;
    const RussianStressDictionary& stress_dictionary() const noexcept { return stress_dictionary_; }

private:
    VoiceDb db_;
    RussianStressDictionary stress_dictionary_;
    LegacyAbbreviationDictionary abbreviation_dictionary_;
    LegacyExceptionDictionary exception_dictionary_;
};

const char* to_string(SynthStatus status) noexcept;

} // namespace nicolai
