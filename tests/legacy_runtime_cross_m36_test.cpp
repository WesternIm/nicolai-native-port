#include "nicolai/legacy_runtime_cross_m36.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using nicolai::legacy_runtime_cross_overlap_sample_m36;

    // Two nearly-unity Q15 products are summed and shifted by 16, so the
    // result is approximately one input amplitude, not two as a Q15 writer
    // would produce for the same pair.
    assert(legacy_runtime_cross_overlap_sample_m36(
        10000, 32767, 10000, 32767) == 9999);

    // Arithmetic right shift rounds negative values toward minus infinity.
    assert(legacy_runtime_cross_overlap_sample_m36(
        -10000, 32767, 0, 0) == -5000);

    // The original adds in 32-bit registers before SAR 16. This exact pair
    // reaches 0x80000000 and therefore wraps to INT32_MIN.
    assert(legacy_runtime_cross_overlap_sample_m36(
        -32768, -32768, -32768, -32768) == -32768);

    assert(legacy_runtime_cross_overlap_sample_m36(0, 32767, 0, 32767) == 0);

    std::cout << "legacy_runtime_cross_m36_test: PASSED\n";
}
