#pragma once

#include <cstdint>

namespace nicolai {

// Central two-source overlap kernel from the nonzero branch of 0x10108cf0,
// specifically 0x10109098..0x101090e0. Unlike the ordinary Q15 writer, the
// two products are added in wrapped 32-bit arithmetic and shifted by 16.
std::int16_t legacy_runtime_cross_overlap_sample_m36(
    std::int16_t left_sample,
    std::int16_t left_window_q15,
    std::int16_t right_sample,
    std::int16_t right_window_q15);

} // namespace nicolai
