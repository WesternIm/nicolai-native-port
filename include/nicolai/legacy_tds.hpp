#pragma once
#include <cstdint>
namespace nicolai {
struct LegacyTdsStepM33 {
    bool valid = false;
    std::int16_t count = 0;
    std::int16_t first_period = 0;
    std::int16_t delta_q11 = 0;
    std::int16_t carry = 0;
};
// Recovered 0x10109800 primitive, independent of the portable renderer.
// duration_q11 scales source support; pitch_q11 divides the source period.
// carry is the prior record's signed WORD +8, not a floating-point phase.
LegacyTdsStepM33 legacy_tds_step_m33(int source_width, int pitch_q11,
    int duration_q11, int next_source_width, int next_pitch_q11, int carry);
} // namespace nicolai
