#include "nicolai/engine.hpp"
#include "nicolai/diphone_catalog.hpp"
#include "nicolai/hybrid_psola.hpp"
#include "nicolai/legacy_prosody.hpp"
#include "nicolai/pc_parity.hpp"
#include "nicolai/russian_frontend.hpp"

#include <utility>

namespace nicolai {

Engine::Engine(VoiceDb db) : db_(std::move(db)) {}
Engine::Engine(VoiceDb db, RussianStressDictionary stress_dictionary)
    : db_(std::move(db)), stress_dictionary_(std::move(stress_dictionary)) {}
Engine::Engine(VoiceDb db,
               RussianStressDictionary stress_dictionary,
               LegacyAbbreviationDictionary abbreviation_dictionary,
               LegacyExceptionDictionary exception_dictionary)
    : db_(std::move(db)),
      stress_dictionary_(std::move(stress_dictionary)),
      abbreviation_dictionary_(std::move(abbreviation_dictionary)),
      exception_dictionary_(std::move(exception_dictionary)) {}

SynthResult Engine::synthesize(const std::string& utf8_text) const {
    if (utf8_text.empty()) return {SynthStatus::InvalidInput, "text is empty", {}};
    if (!db_.metadata().has_nicolai_name)
        return {SynthStatus::VoiceDatabaseNotLoaded, "Nicolai voice marker was not found", {}};

    const auto norm = normalize_russian_legacy_text(
        utf8_text,
        abbreviation_dictionary_.valid ? &abbreviation_dictionary_ : nullptr,
        exception_dictionary_.valid ? &exception_dictionary_ : nullptr);
    if (!norm.valid) return {SynthStatus::InvalidInput, norm.error, {}};

    RussianFrontendOptions fo;
    if (stress_dictionary_.valid) fo.stress_dictionary = &stress_dictionary_;
    const auto front = russian_text_to_nicolai_phones(norm.normalized_utf8, fo);
    if (!front.valid) return {SynthStatus::InvalidInput, front.error, {}};

    const auto catalog = parse_nicolai_diphone_catalog(db_.bytes(), db_.metadata().edat);
    if (!catalog.valid) return {SynthStatus::VoiceDatabaseNotLoaded, catalog.error, {}};

    const int rate = db_.metadata().sample_rate_hint ? db_.metadata().sample_rate_hint : 16000;
    const auto duration = parse_legacy_russian_phone_durations(db_.bytes(), db_.metadata().edat);
    const auto wordstr = parse_legacy_russian_wordstr(db_.bytes(), db_.metadata().edat);
    const auto physical = parse_legacy_russian_physical(db_.bytes(), db_.metadata().edat);
    if (duration.valid) {
        const auto audio = synthesize_diphone_chain_legacy_duration(
            db_.bytes(), catalog, front.phones, duration, wordstr.valid ? &wordstr : nullptr,
            1.0, rate, front.boundaries, {}, physical.valid ? &physical : nullptr, &front);
        if (!audio.valid) return {SynthStatus::InvalidInput, audio.error, {}};
        auto pcm = audio.pcm;
        apply_pc_reference_output_gain(pcm);
        if (wordstr.valid) append_legacy_pc_terminal_silence(pcm, wordstr, rate);
        return {SynthStatus::Ok,
                "M28 PC-parity: recovered physical marker selector + annotation placement",
                std::move(pcm)};
    }

    const auto audio = synthesize_diphone_chain_m15(
        db_.bytes(), catalog, front.phones, {1.0, 1.0}, rate);
    if (!audio.valid) return {SynthStatus::InvalidInput, audio.error, {}};
    auto pcm = audio.pcm;
    apply_pc_reference_output_gain(pcm);
    return {SynthStatus::Ok,
            "M20 PC-parity pass 2: frontend fallback + calibrated PC reference gain",
            std::move(pcm)};
}

const VoiceDbMetadata& Engine::voice_metadata() const noexcept { return db_.metadata(); }

const char* to_string(SynthStatus status) noexcept {
    switch (status) {
        case SynthStatus::Ok: return "ok";
        case SynthStatus::VoiceDatabaseNotLoaded: return "voice_database_not_loaded";
        case SynthStatus::NativeCoreNotImplemented: return "native_core_not_implemented";
        case SynthStatus::InvalidInput: return "invalid_input";
    }
    return "unknown";
}

} // namespace nicolai
