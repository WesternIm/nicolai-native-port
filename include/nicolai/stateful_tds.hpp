#pragma once
#include "nicolai/engine.hpp"
#include "nicolai/seg_schedule.hpp"
#include "nicolai/td_psola.hpp"
#include <cstdint>

namespace nicolai {
struct StatefulTdsM34 {
    std::int16_t carry = 0;
    std::size_t intervals = 0, grains = 0, dropped = 0;
};
// Experimental adapter, NOT the complete PC selection/rollback state machine.
// Exact SEG nodes + Q11 step + Q15 writer; analytic M14 windows and simplified
// current-interval source selection. Caller owns carry across diphones.
Pcm16Mono resynthesize_stateful_m34(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double left_duration, double right_duration,
    double left_energy, double right_energy, StatefulTdsM34& state);
}
