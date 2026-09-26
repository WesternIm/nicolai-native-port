#pragma once

#include <cstdint>
#include <vector>

namespace nicolai {

// Exact positive-domain geometry of the repeated-grain loop at
// mtsyc32.dll 0x10108564..0x1010869e. It includes the source-coordinate
// starts passed to the Q15 writer, but not the surrounding transition state.
struct LegacyRuntimeOrdinaryGrainM36 {
    bool valid = false;
    int ordinal = 0;
    int period = 0;
    int current_interval_width = 0;
    int next_interval_width = 0;
    int left_window_length = 0;
    int right_window_length = 0;
    int left_source_position = 0;
    int right_source_position = 0;
    bool terminal_interval = false;
};

// Repeated grains in the initial-transition path at 0x10108892..0x10108a1c.
// When cross_descriptor is false both sides remain in previous_positions. When
// true, the right side starts at next_positions[0] while the left side still
// ends at the buffered boundary in previous_positions.
struct LegacyRuntimeInitialRepeatedGrainM36 {
    bool valid = false;
    bool cross_descriptor = false;
    int ordinal = 0;
    int period = 0;
    int current_interval_width = 0;
    int next_interval_width = 0;
    int left_window_length = 0;
    int right_window_length = 0;
    int left_source_position = 0;
    int right_source_position = 0;
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
