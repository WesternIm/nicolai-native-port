#include "nicolai/legacy_tds.hpp"
#include <algorithm>
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
} // namespace nicolai
