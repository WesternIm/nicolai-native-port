#include "talker_backend.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() try {
    using namespace nicolai::test_talker;
    auto require = [](bool ok) { if (!ok) throw std::runtime_error("test talker contract failed"); };
    require(parse_profile("stable") == Profile::Stable);
    require(parse_profile("m36-local") == Profile::M36Local);
    require(parse_profile("m36-chain") == Profile::M36Chain);
    require(parse_profile("m38-boundary") == Profile::M38Boundary);
    require(parse_profile("m40-transient") == Profile::M40Transient);
    require(parse_profile("m41-preserve") == Profile::M41Preserve);
    require(std::string(profile_name(Profile::M41Preserve)) == "m41-preserve");
    require(parse_profile("m42-join-pitch") == Profile::M42JoinPitch);
    require(std::string(profile_name(Profile::M42JoinPitch)) == "m42-join-pitch");
    require(parse_profile("m43-word-rhythm") == Profile::M43WordRhythm);
    require(std::string(profile_name(Profile::M43WordRhythm)) == "m43-word-rhythm");
    require(parse_profile("m44-lexicon") == Profile::M44Lexicon);
    require(parse_profile("m47-lexicon") == Profile::M47Lexicon);
    require(std::string(profile_name(Profile::M47Lexicon)) == "m47-lexicon");
    require(timing_policy(Profile::M47Lexicon).word_rhythm_strength_m43 == 0.25);
    require(std::string(profile_name(Profile::M44Lexicon)) == "m44-lexicon");
    require(timing_policy(Profile::M44Lexicon).word_rhythm_strength_m43 == 0.25);
    require(timing_policy(Profile::M44Lexicon).join_period_continuity_m42 == 0.5);
    const auto stable = timing_policy(Profile::Stable);
    require(!stable.use_stateful_tds_m34 && !stable.shared_phone_duration_m34);
    require(stable.word_boundary_speech_share_m38 == 0.0);
    require(!stable.blend_uncovered_edges_m40);
    const auto m38 = timing_policy(Profile::M38Boundary);
    require(!m38.use_stateful_tds_m34 && !m38.shared_phone_duration_m34);
    require(m38.word_boundary_speech_share_m38 == 0.5);
    require(!m38.blend_uncovered_edges_m40);
    const auto m40 = timing_policy(Profile::M40Transient);
    require(m40.blend_uncovered_edges_m40 && !m40.use_stateful_tds_m34);
    require(m40.word_boundary_speech_share_m38 == 0.0);
    for(auto profile : {Profile::Stable,Profile::M36Local,Profile::M36Chain,Profile::M38Boundary,Profile::M40Transient}) {
        const auto policy=timing_policy(profile);
        require(!policy.preserve_unvoiced_runs_m41 && !policy.preserve_unvoiced_joins_m41 && !policy.interpolate_uncovered_m41);
    }
    const auto m41=timing_policy(Profile::M41Preserve);
    require(m41.blend_uncovered_edges_m40 && m41.preserve_unvoiced_joins_m41 && m41.interpolate_uncovered_m41);
    require(!m41.preserve_unvoiced_runs_m41 && !m41.use_stateful_tds_m34);
    require(m41.word_boundary_speech_share_m38==0.0);
    for(auto profile : {Profile::Stable,Profile::M36Local,Profile::M36Chain,Profile::M38Boundary,Profile::M40Transient,Profile::M41Preserve}) {
        const auto policy=timing_policy(profile);
        require(policy.join_period_continuity_m42==0 && !policy.local_join_pitch_m42);
    }
    const auto m42=timing_policy(Profile::M42JoinPitch);
    require(m42.join_period_continuity_m42==0.5 && m42.local_join_pitch_m42);
    require(m42.blend_uncovered_edges_m40 && m42.preserve_unvoiced_joins_m41 && m42.interpolate_uncovered_m41);
    require(!m42.preserve_unvoiced_runs_m41 && !m42.use_stateful_tds_m34 && m42.word_boundary_speech_share_m38==0);
    for(auto profile : {Profile::Stable,Profile::M36Local,Profile::M36Chain,Profile::M38Boundary,Profile::M40Transient,Profile::M41Preserve,Profile::M42JoinPitch})
        require(timing_policy(profile).word_rhythm_strength_m43==0);
    const auto m43=timing_policy(Profile::M43WordRhythm);
    require(m43.word_rhythm_strength_m43==0.25 && m43.join_period_continuity_m42==0.5 && m43.local_join_pitch_m42);
    require(m43.blend_uncovered_edges_m40 && m43.preserve_unvoiced_joins_m41 && m43.interpolate_uncovered_m41);
    require(!m43.use_stateful_tds_m34 && !m43.use_pc_seg_timeline && !m43.preserve_unvoiced_runs_m41 && m43.word_boundary_speech_share_m38==0);
    for (auto profile : {Profile::M36Local, Profile::M36Chain}) {
        const auto experimental = timing_policy(profile);
        require(experimental.use_stateful_tds_m34 && experimental.shared_phone_duration_m34);
        set_profile_environment(profile);
        require(std::string(std::getenv("NICOLAI_M36_TRANSITION_EXECUTOR")) == (profile == Profile::M36Local ? "1" : "0"));
        require(std::string(std::getenv("NICOLAI_M36_CHAIN_EXECUTOR")) == (profile == Profile::M36Chain ? "1" : "0"));
    }
    set_profile_environment(Profile::Stable);
    require(std::string(std::getenv("NICOLAI_M36_TRANSITION_EXECUTOR")) == "0");
    require(std::string(std::getenv("NICOLAI_M36_CHAIN_EXECUTOR")) == "0");
    set_profile_environment(Profile::M38Boundary);
    require(std::string(std::getenv("NICOLAI_M36_TRANSITION_EXECUTOR")) == "0");
    require(std::string(std::getenv("NICOLAI_M36_CHAIN_EXECUTOR")) == "0");
    bool rejected = false;
    try { parse_profile("better"); } catch (...) { rejected = true; }
    require(rejected);
    rejected = false;
    try { render({}, "", Profile::Stable); } catch (...) { rejected = true; }
    require(rejected);
    rejected = false;
    try { validate_voice_directory(std::filesystem::temp_directory_path() / "nicolai-does-not-exist-contract"); }
    catch (...) { rejected = true; }
    require(rejected);
    std::cout << "talker profile and missing-input contracts passed\n";
} catch(const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
