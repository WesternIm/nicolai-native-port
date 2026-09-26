#include "nicolai/legacy_runtime_cross_m36.hpp"

#include <algorithm>
#include <cstdint>

namespace nicolai {
namespace {

std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

int sar16(std::int32_t value) {
    const std::int64_t v = value;
    return v >= 0 ? static_cast<int>(v / 65536) :
        -static_cast<int>((-v + 65535) / 65536);
}

std::int16_t word(int value) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}

} // namespace

LegacyRuntimeCrossPrimaryLayoutM36 legacy_runtime_cross_primary_layout_m36(
    int previous_interval_width,
    int previous_window_length,
    int current_window_length) {
    LegacyRuntimeCrossPrimaryLayoutM36 out;
    if (previous_interval_width <= 0 || previous_window_length < 0 ||
        current_window_length < 0 ||
        previous_window_length > previous_interval_width ||
        current_window_length > previous_interval_width)
        return out;

    const int larger = std::max(previous_window_length, current_window_length);
    const int smaller = std::min(previous_window_length, current_window_length);
    out.valid = true;
    out.total_length = previous_interval_width;
    out.zero_prefix_length = previous_interval_width - larger;
    out.shoulder_length = larger - smaller;
    out.overlap_length = smaller;
    if (previous_window_length > current_window_length)
        out.shoulder = LegacyRuntimeCrossShoulderM36::Previous;
    else if (current_window_length > previous_window_length)
        out.shoulder = LegacyRuntimeCrossShoulderM36::Current;
    else
        out.shoulder = LegacyRuntimeCrossShoulderM36::None;
    return out;
}

std::int16_t legacy_runtime_cross_overlap_sample_m36(
    std::int16_t left_sample,
    std::int16_t left_window_q15,
    std::int16_t right_sample,
    std::int16_t right_window_q15) {
    const std::int32_t left_product =
        static_cast<std::int32_t>(left_sample) * left_window_q15;
    const std::int32_t right_product =
        static_cast<std::int32_t>(right_sample) * right_window_q15;
    const std::int32_t sum = wrap32(
        static_cast<std::int64_t>(left_product) + right_product);
    return word(sar16(sum));
}

} // namespace nicolai
