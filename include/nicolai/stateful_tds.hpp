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

    // M36 opt-in experiment. These fields are dormant for the M34 path and
    // only preserve the deferred terminal record/source needed by the next
    // descriptor's recovered cross-transition executor.
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

// M36 A/B-only route experiment. It composes the recovered M36 initial,
// ordinary, nonzero cross and terminal PCM primitives while retaining the M34
// carry/diagnostic owner. The caller interface cannot yet signal utterance-end,
// so a final deferred terminal may remain pending; this path is deliberately
// opt-in and must not be promoted to production from corpus metrics alone.
Pcm16Mono resynthesize_stateful_m36_experimental(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double left_duration, double right_duration,
    double left_energy, double right_energy, StatefulTdsM34& state);
}