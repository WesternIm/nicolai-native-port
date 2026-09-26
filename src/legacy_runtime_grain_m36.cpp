#include "nicolai/legacy_runtime_grain_m36.hpp"

#include <algorithm>
#include <cstdint>

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

} // namespace

LegacyRuntimeOrdinaryGrainM36 legacy_runtime_ordinary_grain_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    int first_period,
    int delta_q11,
    int ordinal) {
    LegacyRuntimeOrdinaryGrainM36 out;
    if (source_positions.size() < 2 || interval_index < 0 ||
        interval_index >= static_cast<int>(source_positions.size()) - 1 ||
        ordinal < 1 || ordinal > 32767 ||
        first_period < -32768 || first_period > 32767 ||
        delta_q11 < -32768 || delta_q11 > 32767) return out;

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

    // 0x10108568..0x10108585:
    //   movsx delta_q11; imul ordinal; add 1024; sar 11;
    //   add DI, first_period_WORD
    // The final add is explicitly 16-bit and therefore wraps as a WORD.
    const std::int64_t scaled =
        static_cast<std::int64_t>(delta_q11) * ordinal + 1024;
    const int correction = sar11(scaled);
    const int period = word(static_cast<std::int64_t>(correction) + first_period);
    if (period <= 0) return out;

    const int left_length = std::min(current_width, period);
    const int right_length = std::min(next_width, period);

    // 0x1010862f..0x10108648 uses the boundary source position as the right
    // source start and backs the left source up by the selected left length:
    //   left  = pos[i] - left_len + current_width == pos[i+1] - left_len
    //   right = pos[i+1]
    const std::int64_t boundary =
        source_positions[static_cast<std::size_t>(interval_index + 1)];
    const std::int64_t left_source = boundary - left_length;
    if (left_source < std::numeric_limits<std::int32_t>::min() ||
        left_source > std::numeric_limits<std::int32_t>::max()) return out;

    out.valid = true;
    out.ordinal = ordinal;
    out.period = period;
    out.current_interval_width = current_width;
    out.next_interval_width = next_width;
    out.left_window_length = left_length;
    out.right_window_length = right_length;
    out.left_source_position = static_cast<int>(left_source);
    out.right_source_position = static_cast<int>(boundary);
    out.terminal_interval = terminal;
    return out;
}

} // namespace nicolai
