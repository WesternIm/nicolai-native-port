#include "nicolai/legacy_tds.hpp"
#include <algorithm>
#include <limits>
namespace nicolai {
namespace {
// Explicit x86 arithmetic shift / signed WORD truncation, portable to ARM.
int sar11(int value) { return value >= 0 ? value / 2048 : -((-value + 2047) / 2048); }
std::int16_t word(int value) {
    const auto u = static_cast<std::uint16_t>(value);
    return static_cast<std::int16_t>(u <= 32767 ? static_cast<int>(u) : static_cast<int>(u) - 65536);
}
}
LegacyTdsStepM33 legacy_tds_step_m33(
    int width, int pitch, int duration, int next_width, int next_pitch, int carry) {
    LegacyTdsStepM33 out;
    // Reject inputs outside the positive runtime domain instead of emulating
    // x86 divide faults or a pathological signed-WORD loop overflow.
    if (width < 1 || width > 3200 || next_width < 1 || next_width > 3200 ||
        pitch < 256 || pitch > 16384 || next_pitch < 256 || next_pitch > 16384 ||
        duration < 1 || duration > 8192 || carry < -3200 || carry > 3200) return out;
    const int period = width * 2048 / pitch;
    if (period <= 0) return out;
    int target = sar11(width * duration);
    if (target == 0) target = period;
    out.carry = word(carry);
    if (period > 3200) {
        out.count = 1; out.first_period = 3200; out.valid = true; return out;
    }
    const int next_period = next_width * 2048 / next_pitch;
    const int budget = carry + target;
    if (4 * budget < 3 * period) {
        out.carry = word(carry + target); out.valid = true; return out;
    }
    const int difference = next_period - period;
    const int slope = difference * 2048 / budget;
    int count = 1, sum = period;
    while (3 * sum < 2 * budget) {
        const int hop = period + sar11(word(slope) * sum);
        if (hop <= 0 || count >= 32767 || sum + hop > 32767) return {};
        sum += hop; ++count;
    }
    out.count = word(count);
    out.first_period = word(period);
    out.delta_q11 = word(std::clamp(difference * 2048 / count, -32768, 32767));
    out.carry = word(carry - sum + target);
    out.valid = true;
    return out;
}
int legacy_reciprocal_pitch_m34(int start, int length, int end, int position) {
    if (start < 1 || end < 1 || length < 1 || position < 0 || position > length) return 0;
    const int a = 0x10000000 / start;
    const int slope = (0x10000000 / end - a) / length;
    return a + slope * position;
}
bool legacy_tds_write_m34(std::vector<std::int16_t>& output, std::size_t cursor,
    int period, const std::vector<std::int16_t>& left,
    const std::vector<std::int16_t>& right,
    const std::vector<std::int16_t>& lw,
    const std::vector<std::int16_t>& rw) {
    if (period < 1 || period > 3200 || lw.size() > static_cast<std::size_t>(period) ||
        rw.size() > static_cast<std::size_t>(period) || left.size() < lw.size() ||
        right.size() < rw.size() || cursor > output.size() ||
        cursor > std::numeric_limits<std::size_t>::max() - period) return false;
    output.resize(std::max(output.size(), cursor + period));
    const int right_start = period - static_cast<int>(rw.size());
    for (int i = 0; i < period; ++i) {
        // Two signed WORD products fit in int64 even for -32768 inputs.
        std::int64_t sum = 0;
        if (i < static_cast<int>(lw.size())) sum += static_cast<int>(left[i]) * lw[i];
        if (i >= right_start) {
            const auto j = static_cast<std::size_t>(i - right_start);
            sum += static_cast<int>(right[j]) * rw[rw.size() - 1 - j];
        }
        const auto shifted = sum >= 0 ? sum / 32768 : -((-sum + 32767) / 32768);
        output[cursor + i] = word(static_cast<int>(shifted));
    }
    return true;
}
} // namespace nicolai
