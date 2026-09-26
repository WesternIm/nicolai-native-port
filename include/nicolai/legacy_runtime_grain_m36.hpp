#pragma once

#include <cstdint>
#include <vector>

namespace nicolai {

// Exact positive-domain geometry of the repeated-grain loop at
// mtsyc32.dll 0x10108564..0x1010869e. left/right refer to the actual arg2/arg3
// source inputs and arg6..arg9 window ownership of legacy_tds_write_m34(), not
// merely to chronological source sides.
struct LegacyRuntimeOrdinaryGrainM36 {
    bool valid = false;
    int ordinal = 0;
    int period = 0;
    int current_interval_width = 0;
    int next_interval_width = 0;
    int left_window_length = 0;   // writer arg7, next/future interval extent
    int right_window_length = 0;  // writer arg9, current/past interval extent
    int left_source_position = 0; // writer arg2
    int right_source_position = 0;// writer arg3
    bool terminal_interval = false;
};

// Repeated grains in the initial-transition path at 0x10108892..0x10108a1c.
// The writer's right input remains the buffered/past descriptor tail. The left
// input starts at the following boundary; when cross_descriptor is true that
// left input belongs to next_positions and starts at next_positions[0].
struct LegacyRuntimeInitialRepeatedGrainM36 {
    bool valid = false;
    bool cross_descriptor = false;
    int ordinal = 0;
    int period = 0;
    int current_interval_width = 0;
    int next_interval_width = 0;
    int left_window_length = 0;   // writer arg7, next/future extent
    int right_window_length = 0;  // writer arg9, current/past extent
    int left_source_position = 0; // writer arg2
    int right_source_position = 0;// writer arg3
};

LegacyRuntimeOrdinaryGrainM36 legacy_runtime_ordinary_grain_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    int first_period,
    int delta_q11,
    int ordinal);

LegacyRuntimeInitialRepeatedGrainM36 legacy_runtime_initial_repeated_grain_m36(
    const std::vector<std::int32_t>& previous_positions,
    const std::vector<std::int32_t>& next_positions,
    int buffered_interval_index,
    bool cross_descriptor,
    int first_period,
    int delta_q11,
    int ordinal);

} // namespace nicolai
