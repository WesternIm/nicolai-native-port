#include "talker_backend.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/diphone_catalog.hpp"
#include "nicolai/legacy_prosody.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"
#include <cstdlib>
#include <stdexcept>

namespace nicolai::test_talker {
Profile parse_profile(const std::string& value) {
    if (value == "stable") return Profile::Stable;
    if (value == "m36-local") return Profile::M36Local;
    if (value == "m36-chain") return Profile::M36Chain;
    if (value == "m38-boundary") return Profile::M38Boundary;
    if (value == "m40-transient") return Profile::M40Transient;
    if (value == "m41-preserve") return Profile::M41Preserve;
    if (value == "m42-join-pitch") return Profile::M42JoinPitch;
    if (value == "m43-word-rhythm") return Profile::M43WordRhythm;
    if (value == "m44-lexicon") return Profile::M44Lexicon;
    if (value == "m47-lexicon") return Profile::M47Lexicon;
    if (value == "m48-lexicon") return Profile::M48Lexicon;
    throw std::runtime_error("unknown test profile");
}
const char* profile_name(Profile profile) {
    switch (profile) {
    case Profile::Stable: return "stable";
    case Profile::M36Local: return "m36-local";
    case Profile::M36Chain: return "m36-chain";
    case Profile::M38Boundary: return "m38-boundary";
    case Profile::M40Transient: return "m40-transient";
    case Profile::M41Preserve: return "m41-preserve";
    case Profile::M42JoinPitch: return "m42-join-pitch";
    case Profile::M43WordRhythm: return "m43-word-rhythm";
    case Profile::M44Lexicon: return "m44-lexicon";
    case Profile::M47Lexicon: return "m47-lexicon";
    case Profile::M48Lexicon: return "m48-lexicon";
    }
    throw std::runtime_error("unknown test profile");
}
LegacyTimingPolicy timing_policy(Profile profile) {
    profile_name(profile); // Reject invalid enum values, never default silently.
    if(profile==Profile::M44Lexicon || profile==Profile::M47Lexicon || profile==Profile::M48Lexicon) return timing_policy(Profile::M43WordRhythm);
    if(profile==Profile::M43WordRhythm) {
        auto policy=timing_policy(Profile::M42JoinPitch);
        policy.word_rhythm_strength_m43=0.25;
        return policy;
    }
    LegacyTimingPolicy policy;
    if (profile == Profile::M36Local || profile == Profile::M36Chain) {
        policy.use_stateful_tds_m34 = true;
        policy.shared_phone_duration_m34 = true;
    }
    if (profile == Profile::M38Boundary)
        policy.word_boundary_speech_share_m38 = 0.5;
    if (profile == Profile::M40Transient || profile == Profile::M41Preserve || profile == Profile::M42JoinPitch)
        policy.blend_uncovered_edges_m40 = true;
    if (profile == Profile::M41Preserve || profile == Profile::M42JoinPitch) {
        policy.preserve_unvoiced_joins_m41 = true;
        policy.interpolate_uncovered_m41 = true;
        // Internal run protection remains an independent audit ablation:
        // its duration/active-waveform regressions do not justify UI use.
    }
    if (profile == Profile::M42JoinPitch) {
        policy.join_period_continuity_m42 = 0.5;
        policy.local_join_pitch_m42 = true;
    }
    return policy;
}
void set_profile_environment(Profile profile) {
    profile_name(profile);
    auto set = [](const char* name, const char* value) {
#ifdef _WIN32
        if (_putenv_s(name, value) != 0) throw std::runtime_error("cannot set test profile environment");
#else
        if (setenv(name, value, 1) != 0) throw std::runtime_error("cannot set test profile environment");
#endif
    };
    set("NICOLAI_M36_TRANSITION_EXECUTOR", profile == Profile::M36Local ? "1" : "0");
    set("NICOLAI_M36_CHAIN_EXECUTOR", profile == Profile::M36Chain ? "1" : "0");
}
void validate_voice_directory(const std::filesystem::path& directory) {
    for (const auto* name : {"nicolai16.dat", "exc_rus.txt", "abb_rus.txt"}) {
        const auto file = directory / name;
        if (!std::filesystem::is_regular_file(file) || std::filesystem::file_size(file) == 0)
            throw std::runtime_error(std::string("missing/empty voice file: ") + name);
    }
}
DiphoneChainLegacyResult render(const std::filesystem::path& directory,
                               const std::string& text, Profile profile,
                               RussianFrontendResult* frontend_diagnostic) {
    if (text.empty() || text.size() > 64000) throw std::runtime_error("text must contain 1..64000 UTF-8 bytes");
    validate_voice_directory(directory);
    set_profile_environment(profile);
    auto db = VoiceDb::load(directory / "nicolai16.dat");
    const auto stress = load_exc_rus_cp1251(directory / "exc_rus.txt");
    const auto exceptions = load_exc_rus_replacements_cp1251(directory / "exc_rus.txt");
    const auto abbreviations = load_abb_rus_cp1251(directory / "abb_rus.txt");
    const auto catalog = parse_nicolai_diphone_catalog(db.bytes(), db.metadata().edat);
    const auto duration = parse_legacy_russian_phone_durations(db.bytes(), db.metadata().edat);
    const auto wordstr = parse_legacy_russian_wordstr(db.bytes(), db.metadata().edat);
    const auto physical = parse_legacy_russian_physical(db.bytes(), db.metadata().edat);
    if (!catalog.valid || !duration.valid || !stress.valid || !exceptions.valid || !abbreviations.valid)
        throw std::runtime_error("voice database or dictionaries failed validation");
    const auto normalized = normalize_russian_legacy_text(text, &abbreviations, &exceptions);
    if (!normalized.valid) throw std::runtime_error(normalized.error);
    RussianFrontendOptions options;
    options.stress_dictionary = &stress;
    options.enable_fixed_ika_stress_m43 = profile == Profile::M43WordRhythm || profile == Profile::M44Lexicon || profile == Profile::M47Lexicon || profile == Profile::M48Lexicon;
    options.enable_lexicon_stress_m47 = profile == Profile::M47Lexicon;
    options.enable_lexicon_stress_m48 = profile == Profile::M48Lexicon;
    RussianLexiconM44 lexicon;
    if(profile==Profile::M44Lexicon || profile==Profile::M47Lexicon || profile==Profile::M48Lexicon) {
        lexicon=parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
        if(!lexicon.valid) throw std::runtime_error(lexicon.error);
        options.lexicon_m44=&lexicon;
    }
    const auto front = russian_text_to_nicolai_phones(normalized.normalized_utf8, options);
    if (!front.valid) throw std::runtime_error(front.error);
    auto result = synthesize_diphone_chain_legacy_duration(db.bytes(), catalog,
        front.phones, duration, wordstr.valid ? &wordstr : nullptr, 1.0, 16000,
        front.boundaries, timing_policy(profile), physical.valid ? &physical : nullptr, &front);
    if (!result.valid || result.pcm.samples.empty()) throw std::runtime_error(result.error.empty() ? "no PCM generated" : result.error);
    apply_pc_reference_output_gain(result.pcm);
    if (wordstr.valid) append_legacy_pc_terminal_silence(result.pcm, wordstr, 16000);
    if (frontend_diagnostic) *frontend_diagnostic = front;
    return result;
}
}
