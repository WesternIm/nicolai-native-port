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

    // Reserved M36 route/cross diagnostics and future caller-owned context.
    // The first runnable A/B adapter deliberately keeps cross-descriptor
    // ownership disabled until the original step-buffer/source context is
    // proven, so these remain zero/empty in the current local experiment.
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
}