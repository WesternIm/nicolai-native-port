#include "nicolai/stateful_tds.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    nicolai::Pcm16Mono source;
    source.sample_rate = 16000;
    source.samples.resize(401);
    for (std::size_t i = 0; i < source.samples.size(); ++i)
        source.samples[i] = static_cast<std::int16_t>(
            static_cast<int>((i * 97) % 20000) - 10000);

    nicolai::SegSourceTimelineM33 timeline;
    timeline.valid = true;
    timeline.split_node = 2;
    timeline.nodes = {
        {0, true}, {80, true}, {160, true}, {260, true}, {400, true}
    };

    nicolai::TdPsolaConfig pitch;
    nicolai::StatefulTdsM34 m36_state;
    const auto m36 = nicolai::resynthesize_stateful_m36_experimental(
        source, timeline, pitch, 1.0, 1.0, 1.0, 1.0, m36_state);
    assert(!m36.samples.empty());
    assert(m36_state.intervals == 4);
    assert(m36_state.m36_initial_paths == 1);
    assert(m36_state.m36_terminal_flushes == 1);
    assert(m36_state.m36_cross_paths == 0);
    assert(m36_state.m36_fallbacks == 0);
    assert(m36_state.grains > 0);
    assert(m36_state.emitted_samples == static_cast<std::int64_t>(m36.samples.size()));

    // The experimental path must be measurably different from the old M34
    // front/tail + analytic-half-Hann adapter on a nonconstant source.
    nicolai::StatefulTdsM34 m34_state;
    const auto m34 = nicolai::resynthesize_stateful_m34(
        source, timeline, pitch, 1.0, 1.0, 1.0, 1.0, m34_state);
    assert(!m34.samples.empty());
    assert(m36.samples != m34.samples);

    // M36 local executor currently does not claim phone-local energy parity.
    // Reject such a request rather than silently measuring a different policy.
    auto before = m36_state;
    const auto unsupported = nicolai::resynthesize_stateful_m36_experimental(
        source, timeline, pitch, 1.0, 1.0, 0.9, 1.0, m36_state);
    assert(unsupported.samples.empty());
    assert(m36_state.intervals == before.intervals);
    assert(m36_state.grains == before.grains);

    std::cout << "stateful_tds_m36_test: PASSED\n";
}
