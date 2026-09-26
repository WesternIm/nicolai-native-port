#pragma once

#include <cstdint>
#include <vector>

namespace nicolai {

// Exact positive-domain geometry of the repeated-grain loop at
// mtsyc32.dll 0x10108564..0x1010869e. This deliberately stops before source
// pointer construction: the window/period ownership is proven independently
// of the still-being-named pointer offsets.
struct LegacyRuntimeOrdinaryGrainM36 {
    bool valid = false;
    int ordinal = 0;
    int period = 0;
    int current_interval_width = 0;
    int next_interval_width = 0;
    int left_window_length = 0;
    int right_window_length = 0;
    bool terminal_interval = false;
};

LegacyRuntimeOrdinaryGrainM36 legacy_runtime_ordinary_grain_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    int first_period,
    int delta_q11,
    int ordinal);

} // namespace nicolai
