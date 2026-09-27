#include "nicolai/stateful_tds.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

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

    nicolai::StatefulTdsUnitM36 unit;
    unit.source=source; unit.timeline=timeline; unit.pitch=pitch;
    nicolai::StatefulTdsM34 chain_state;
    auto chain=nicolai::resynthesize_stateful_m36_chain_experimental(
        {unit,unit,unit},chain_state);
    assert(!chain.samples.empty());
    assert(chain_state.intervals==12);
    assert(chain_state.m36_cross_paths==2);
    assert(chain_state.m36_initial_paths==1);
    assert(chain_state.m36_terminal_flushes==1);
    assert(chain_state.m36_fallbacks==0);
    assert(chain_state.emitted_samples==static_cast<std::int64_t>(chain.samples.size()));
    assert(!chain_state.m36_has_pending_terminal);

    // Explicit zero-cross boundaries flush old tails rather than losing them.
    auto disconnected=unit;
    disconnected.cross_from_previous=false;
    nicolai::StatefulTdsM34 zero_state;
    auto zero=nicolai::resynthesize_stateful_m36_chain_experimental(
        {unit,disconnected},zero_state);
    assert(!zero.samples.empty());
    assert(zero_state.m36_cross_paths==0);
    assert(zero_state.m36_terminal_flushes==2);
    assert(zero_state.m36_initial_paths==2);

    // A late invalid descriptor or non-finite gain must not commit diagnostics.
    auto bad=unit;
    bad.source.samples.pop_back();
    const auto before_chain=chain_state;
    assert(nicolai::resynthesize_stateful_m36_chain_experimental(
        {unit,unit,bad},chain_state).samples.empty());
    assert(chain_state.intervals==before_chain.intervals);
    assert(chain_state.emitted_samples==before_chain.emitted_samples);
    assert(chain_state.m36_cross_paths==before_chain.m36_cross_paths);
    bad=unit;
    bad.left_energy=std::numeric_limits<double>::quiet_NaN();
    assert(nicolai::resynthesize_stateful_m36_chain_experimental(
        {unit,bad},chain_state).samples.empty());

    // A dropped prefix does not overwrite the positive deferred step record.
    auto dropped_prefix=unit;
    dropped_prefix.left_duration=.125;
    nicolai::StatefulTdsM34 drop_state;
    auto dropped=nicolai::resynthesize_stateful_m36_chain_experimental(
        {unit,dropped_prefix},drop_state);
    assert(!dropped.samples.empty() && drop_state.dropped>0);
    assert(drop_state.m36_cross_paths==1 && drop_state.m36_fallbacks==0);

    // Out-of-cache windows take counted compatibility paths. A cross failure
    // must not append any partial exact phase before fallback emits both steps.
    auto wide=unit;
    wide.source.samples.resize(2001,1234);
    wide.timeline.nodes={{0,false},{500,false},{1000,false},{1500,false},{2000,false}};
    nicolai::StatefulTdsM34 wide_state;
    const auto fallback=nicolai::resynthesize_stateful_m36_chain_experimental(
        {wide,wide},wide_state);
    assert(fallback.samples.size()==4000);
    assert(wide_state.grains==8 && wide_state.emitted_samples==4000);
    assert(wide_state.m36_cross_paths==0 && wide_state.m36_fallbacks==7);

    std::cout << "stateful_tds_m36_test: PASSED\n";
}
