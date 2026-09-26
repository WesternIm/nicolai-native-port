#pragma once
#include <cstdint>
#include <vector>
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

// 0x101a2c00: interpolate a positive reciprocal pitch ordinate in Q28;
// integer divisions truncate. Returns zero for invalid inputs. The helper's
// arithmetic is proven; naming the upstream feature WORD units is separate.
int legacy_reciprocal_pitch_m34(int start, int length, int end, int position);

// Positive runtime domain of 0x10109980. Writes exactly period samples;
// overlap products are summed BEFORE arithmetic >>15, then WORD-wrapped.
// right_window is read backwards. No wsum normalization or phase search.
bool legacy_tds_write_m34(std::vector<std::int16_t>& output, std::size_t cursor,
    int period, const std::vector<std::int16_t>& left,
    const std::vector<std::int16_t>& right,
    const std::vector<std::int16_t>& left_window,
    const std::vector<std::int16_t>& right_window);
} // namespace nicolai
