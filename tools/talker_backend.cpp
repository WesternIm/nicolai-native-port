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
    throw std::runtime_error("unknown test profile");
}
const char* profile_name(Profile profile) {
    switch (profile) {
    case Profile::Stable: return "stable";
    case Profile::M36Local: return "m36-local";
    case Profile::M36Chain: return "m36-chain";
    }
    throw std::runtime_error("unknown test profile");
}
LegacyTimingPolicy timing_policy(Profile profile) {
    profile_name(profile); // Reject invalid enum values, never default silently.
    LegacyTimingPolicy policy;
    if (profile != Profile::Stable) {
        policy.use_stateful_tds_m34 = true;
        policy.shared_phone_duration_m34 = true;
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
                               const std::string& text, Profile profile) {
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
    const auto front = russian_text_to_nicolai_phones(normalized.normalized_utf8, options);
    if (!front.valid) throw std::runtime_error(front.error);
    auto result = synthesize_diphone_chain_legacy_duration(db.bytes(), catalog,
        front.phones, duration, wordstr.valid ? &wordstr : nullptr, 1.0, 16000,
        front.boundaries, timing_policy(profile), physical.valid ? &physical : nullptr, &front);
    if (!result.valid || result.pcm.samples.empty()) throw std::runtime_error(result.error.empty() ? "no PCM generated" : result.error);
    apply_pc_reference_output_gain(result.pcm);
    if (wordstr.valid) append_legacy_pc_terminal_silence(result.pcm, wordstr, 16000);
    return result;
}
}
