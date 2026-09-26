#include "nicolai/legacy_runtime_cross_m36.hpp"

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
