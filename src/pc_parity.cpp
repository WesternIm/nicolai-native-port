#include "nicolai/pc_parity.hpp"
#include <algorithm>
#include <cmath>

namespace nicolai {

void apply_pc_reference_output_gain(Pcm16Mono& pcm, double gain) noexcept {
    if (!(gain > 0.0) || std::abs(gain - 1.0) < 1e-12) return;
    for (auto& s : pcm.samples) {
        double v = std::lrint(static_cast<double>(s) * gain);
        v = std::max(-32768.0, std::min(32767.0, v));
        s = static_cast<std::int16_t>(v);
    }
}

} // namespace nicolai
