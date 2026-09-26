#include "nicolai/legacy_window_m36.hpp"

#include <cassert>
#include <iostream>
#include <vector>

int main() {
    const std::vector<int> expected{
        20, 24, 29, 35, 43, 52, 63, 77, 94,
        115, 141, 173, 212, 260, 319, 392, 400};
    assert(nicolai::legacy_window_m36_anchors() == expected);

    const auto w20 = nicolai::legacy_window_m36_direct(20);
    assert(w20.valid && w20.exact_anchor);
    assert(w20.q15.size() == 20);
    assert(w20.q15.front() == 32767);
    for (std::size_t i = 1; i < w20.q15.size(); ++i)
        assert(w20.q15[i] <= w20.q15[i - 1]);

    // Anchor 24 is built from the previous 20-sample cosine core, centered by
    // two unity samples on the left and two zero samples on the right.
    const auto w24 = nicolai::legacy_window_m36_direct(24);
    assert(w24.valid && w24.exact_anchor);
    assert(w24.q15.size() == 24);
    assert(w24.q15[0] == 32767 && w24.q15[1] == 32767);
    assert(w24.q15[2] == w20.q15[0]);
    assert(w24.q15[21] == w20.q15[19]);
    assert(w24.q15[22] == 0 && w24.q15[23] == 0);

    // Non-anchor entries are pointer slices of the packed adjacent anchor
    // buffers, not separately computed Hann windows.
    const auto w21 = nicolai::legacy_window_m36_direct(21);
    assert(w21.valid && !w21.exact_anchor);
    assert(w21.lower_anchor == 20 && w21.upper_anchor == 24);
    assert(w21.q15.size() == 21);
    assert(w21.q15[0] == w24.q15[1]);
    for (int i = 1; i < 21; ++i)
        assert(w21.q15[static_cast<std::size_t>(i)] ==
            w24.q15[static_cast<std::size_t>(i + 1)]);

    const auto w23 = nicolai::legacy_window_m36_direct(23);
    assert(w23.valid && !w23.exact_anchor);
    assert(w23.q15.size() == 23);
    assert(w23.q15[0] == w20.q15.back());
    assert(w23.q15[1] == w24.q15.front());

    const auto w100 = nicolai::legacy_window_m36_direct(100);
    assert(w100.valid && !w100.exact_anchor);
    assert(w100.lower_anchor == 94 && w100.upper_anchor == 115);
    assert(w100.q15.size() == 100);

    // 0x10109be0 direct cache test is min <= n < max. 400 and out-of-range
    // lengths deliberately remain for the recovered resampling fallback.
    assert(!nicolai::legacy_window_m36_direct(19).valid);
    assert(!nicolai::legacy_window_m36_direct(400).valid);
    assert(!nicolai::legacy_window_m36_direct(401).valid);

    std::cout << "legacy_window_m36_test: PASSED\n";
}
