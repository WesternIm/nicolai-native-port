#pragma once

#include <cstdint>

namespace nicolai {

enum class LegacyRuntimeCrossShoulderM36 {
    None = 0,
    Previous,
    Current,
};

// Region ownership of the first temporary PCM buffer in the nonzero branch of
// 0x10108cf0. The buffer length is previous_interval_width and partitions into
// zero-prefix, at most one single-source shoulder, then the two-source overlap.
struct LegacyRuntimeCrossPrimaryLayoutM36 {
    bool valid = false;
    int total_length = 0;
    int zero_prefix_length = 0;
    int shoulder_length = 0;
    int overlap_length = 0;
    LegacyRuntimeCrossShoulderM36 shoulder = LegacyRuntimeCrossShoulderM36::None;
};

LegacyRuntimeCrossPrimaryLayoutM36 legacy_runtime_cross_primary_layout_m36(
    int previous_interval_width,
    int previous_window_length,
    int current_window_length);

// Central two-source overlap kernel from the nonzero branch of 0x10108cf0,
// specifically 0x10109098..0x101090e0. Unlike the ordinary Q15 writer, the
// two products are added in wrapped 32-bit arithmetic and shifted by 16.
std::int16_t legacy_runtime_cross_overlap_sample_m36(
    std::int16_t left_sample,
    std::int16_t left_window_q15,
    std::int16_t right_sample,
    std::int16_t right_window_q15);

} // namespace nicolai
