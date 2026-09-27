#include "talker_backend.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    using namespace nicolai::test_talker;
    auto require = [](bool ok) { if (!ok) throw std::runtime_error("test talker contract failed"); };
    require(parse_profile("stable") == Profile::Stable);
    require(parse_profile("m36-local") == Profile::M36Local);
    require(parse_profile("m36-chain") == Profile::M36Chain);
    const auto stable = timing_policy(Profile::Stable);
    require(!stable.use_stateful_tds_m34 && !stable.shared_phone_duration_m34);
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
}
