#pragma once
#include "nicolai/engine.hpp"
#include "nicolai/legacy_tds.hpp"
#include "nicolai/seg_schedule.hpp"
#include "nicolai/td_psola.hpp"
#include <cstdint>
#include <vector>

namespace nicolai {
struct StatefulTdsM34 {
    std::int16_t carry = 0;
    std::size_t intervals = 0, grains = 0, dropped = 0;
    std::int64_t target_samples = 0, budget_consumed_samples = 0, emitted_samples = 0;
    std::size_t clamped_delta_records = 0;

    // Reserved streaming context (the whole-chain API owns its own context).
    // Both experimental APIs return with no retained pending PCM. Diagnostics
    // count local initial entries / chain zero-cross starts, successful exact
    // nonzero crossings, terminal flushes and compatibility fallback requests.
    bool m36_has_pending_terminal = false;
    int m36_pending_interval = -1;
    LegacyTdsStepM33 m36_pending_step;
    std::vector<std::int16_t> m36_pending_pcm;
    std::vector<std::int32_t> m36_pending_positions;
    std::size_t m36_initial_paths = 0;
    std::size_t m36_cross_paths = 0;
    std::size_t m36_terminal_flushes = 0;
    std::size_t m36_fallbacks = 0;
};
// Experimental adapter, NOT the complete PC selection/rollback state machine.
// Exact SEG nodes + Q11 step + Q15 writer; analytic M14 windows and simplified
// current-interval source selection. Caller owns carry across diphones.
Pcm16Mono resynthesize_stateful_m34(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double left_duration, double right_duration,
    double left_energy, double right_energy, StatefulTdsM34& state);

// M36 A/B-only local transition experiment. It composes recovered M36
// initial, ordinary and deferred-terminal PCM behavior plus exact M36 windows
// and source selection. Nonzero cross-descriptor execution is intentionally
// excluded until caller step-buffer ownership and source context outside one
// portable diphone slice are proven. This path is opt-in and is not production.
Pcm16Mono resynthesize_stateful_m36_experimental(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double left_duration, double right_duration,
    double left_energy, double right_energy, StatefulTdsM34& state);

// Explicit multi-descriptor A/B input. No implicit environment dispatch in
// this API. cross_from_previous selects the recovered nonzero transition;
// false flushes the old tail and starts a zero-cross fade-in. Inferring the
// original descriptor flags/flush gates from portable voicing is experimental.
struct StatefulTdsUnitM36 {
    Pcm16Mono source;
    SegSourceTimelineM33 timeline;
    TdPsolaConfig pitch;
    double left_duration = 1.0, right_duration = 1.0;
    double left_energy = 1.0, right_energy = 1.0;
    bool cross_from_previous = true;
};

// Whole-chain transaction: a late invalid unit leaves caller diagnostics
// unchanged. Unsupported exact-window requests use counted M34 compatibility
// writes, never a partial exact write followed by duplicate fallback PCM.
Pcm16Mono resynthesize_stateful_m36_chain_experimental(
    const std::vector<StatefulTdsUnitM36>& units, StatefulTdsM34& state);
}
