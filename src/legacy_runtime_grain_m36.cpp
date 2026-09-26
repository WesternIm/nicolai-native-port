#include "nicolai/legacy_runtime_grain_m36.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace nicolai {
namespace {

std::int16_t word(std::int64_t value) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}

int sar11(std::int64_t value) {
    return value >= 0 ? static_cast<int>(value / 2048) :
        -static_cast<int>((-value + 2047) / 2048);
}

int word_delta(std::int32_t right, std::int32_t left) {
    const auto r = static_cast<std::uint16_t>(right);
    const auto l = static_cast<std::uint16_t>(left);
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(r - l));
}

int repeated_period(int first_period, int delta_q11, int ordinal) {
    if (ordinal < 1 || ordinal > 32767 ||
        first_period < -32768 || first_period > 32767 ||
        delta_q11 < -32768 || delta_q11 > 32767) return 0;
    const std::int64_t scaled =
        static_cast<std::int64_t>(delta_q11) * ordinal + 1024;
    return word(static_cast<std::int64_t>(sar11(scaled)) + first_period);
}

bool int32_coordinate(std::int64_t value) {
    return value >= std::numeric_limits<std::int32_t>::min() &&
        value <= std::numeric_limits<std::int32_t>::max();
}

} // namespace

LegacyRuntimeOrdinaryGrainM36 legacy_runtime_ordinary_grain_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    int first_period,
    int delta_q11,
    int ordinal) {
    LegacyRuntimeOrdinaryGrainM36 out;
    if (source_positions.size() < 2 || interval_index < 0 ||
        interval_index >= static_cast<int>(source_positions.size()) - 1)
        return out;

    const int current_width = word_delta(
        source_positions[static_cast<std::size_t>(interval_index + 1)],
        source_positions[static_cast<std::size_t>(interval_index)]);
    if (current_width <= 0) return out;

    const int last_interval = static_cast<int>(source_positions.size()) - 2;
    const bool terminal = interval_index == last_interval;
    int next_width = current_width;
    if (!terminal) {
        next_width = word_delta(
            source_positions[static_cast<std::size_t>(interval_index + 2)],
            source_positions[static_cast<std::size_t>(interval_index + 1)]);
        if (next_width <= 0) return out;
    }

    const int period = repeated_period(first_period, delta_q11, ordinal);
    if (period <= 0) return out;

    // Writer call 0x10108616..0x10108652:
    // arg7 (left length) is min(next_width, period), while arg9 (right length)
    // is min(current_width, period). arg2 starts at the boundary and arg3 is
    // the current/past tail ending at that boundary.
    const int left_length = std::min(next_width, period);
    const int right_length = std::min(current_width, period);
    const std::int64_t boundary =
        source_positions[static_cast<std::size_t>(interval_index + 1)];
    const std::int64_t right_source = boundary - right_length;
    if (!int32_coordinate(right_source) || !int32_coordinate(boundary)) return out;

    out.valid = true;
    out.ordinal = ordinal;
    out.period = period;
    out.current_interval_width = current_width;
    out.next_interval_width = next_width;
    out.left_window_length = left_length;
    out.right_window_length = right_length;
    out.left_source_position = static_cast<int>(boundary);
    out.right_source_position = static_cast<int>(right_source);
    out.terminal_interval = terminal;
    return out;
}

LegacyRuntimeInitialRepeatedGrainM36 legacy_runtime_initial_repeated_grain_m36(
    const std::vector<std::int32_t>& previous_positions,
    const std::vector<std::int32_t>& next_positions,
    int buffered_interval_index,
    bool cross_descriptor,
    int first_period,
    int delta_q11,
    int ordinal) {
    LegacyRuntimeInitialRepeatedGrainM36 out;
    if (previous_positions.size() < 2 || buffered_interval_index < 0 ||
        buffered_interval_index >= static_cast<int>(previous_positions.size()) - 1)
        return out;

    const int current_width = word_delta(
        previous_positions[static_cast<std::size_t>(buffered_interval_index + 1)],
        previous_positions[static_cast<std::size_t>(buffered_interval_index)]);
    if (current_width <= 0) return out;

    int next_width = 0;
    std::int64_t left_source = 0;
    if (cross_descriptor) {
        if (next_positions.size() < 2) return out;
        next_width = word_delta(next_positions[1], next_positions[0]);
        left_source = next_positions[0];
    } else {
        if (buffered_interval_index + 2 >=
            static_cast<int>(previous_positions.size())) return out;
        next_width = word_delta(
            previous_positions[static_cast<std::size_t>(buffered_interval_index + 2)],
            previous_positions[static_cast<std::size_t>(buffered_interval_index + 1)]);
        left_source =
            previous_positions[static_cast<std::size_t>(buffered_interval_index + 1)];
    }
    if (next_width <= 0) return out;

    const int period = repeated_period(first_period, delta_q11, ordinal);
    if (period <= 0) return out;
    const int left_length = std::min(next_width, period);
    const int right_length = std::min(current_width, period);

    const std::int64_t previous_boundary =
        previous_positions[static_cast<std::size_t>(buffered_interval_index + 1)];
    const std::int64_t right_source = previous_boundary - right_length;
    if (!int32_coordinate(left_source) || !int32_coordinate(right_source))
        return out;

    out.valid = true;
    out.cross_descriptor = cross_descriptor;
    out.ordinal = ordinal;
    out.period = period;
    out.current_interval_width = current_width;
    out.next_interval_width = next_width;
    out.left_window_length = left_length;
    out.right_window_length = right_length;
    out.left_source_position = static_cast<int>(left_source);
    out.right_source_position = static_cast<int>(right_source);
    return out;
}

} // namespace nicolai
