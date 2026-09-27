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

    // Small fallback: len 10 immediately selects factor 2 and table slot 20,
    // which is the cached length-40 pointer, then copies every second WORD.
    const auto w40 = nicolai::legacy_window_m36_direct(40);
    const auto w10 = nicolai::legacy_window_m36_lookup(10);
    assert(w40.valid && w10.valid && w10.resampled);
    assert(w10.resample_factor == 2 && w10.source_cache_length == 40);
    assert(w10.q15.size() == 10);
    for (int i = 0; i < 10; ++i)
        assert(w10.q15[static_cast<std::size_t>(i)] ==
            w40.q15[static_cast<std::size_t>(i * 2)]);

    // len 1 repeatedly doubles the decimation factor until 2+factor > 20.
    const auto w52 = nicolai::legacy_window_m36_direct(52);
    const auto w1 = nicolai::legacy_window_m36_lookup(1);
    assert(w52.valid && w1.valid && w1.resampled);
    assert(w1.resample_factor == 32 && w1.source_cache_length == 52);
    assert(w1.q15.size() == 1 && w1.q15[0] == w52.q15[0]);

    // max=400 is excluded from direct lookup and enters the >=max branch.
    // The first factor is 2, selecting slot 200 => nominal cached length 220.
    const auto w220 = nicolai::legacy_window_m36_direct(220);
    const auto w400 = nicolai::legacy_window_m36_lookup(400);
    assert(w220.valid && w400.valid && w400.resampled);
    assert(w400.resample_factor == 2 && w400.source_cache_length == 220);
    assert(w400.q15.size() == 400);
    for (int i = 0; i < 200; ++i) {
        assert(w400.q15[static_cast<std::size_t>(2 * i)] ==
            w220.q15[static_cast<std::size_t>(i)]);
        const int sum = static_cast<int>(w220.q15[static_cast<std::size_t>(i)]) +
            static_cast<int>(w220.q15[static_cast<std::size_t>(i + 1)]);
        assert(w400.q15[static_cast<std::size_t>(2 * i + 1)] ==
            static_cast<std::int16_t>(sum / 2));
    }

    assert(!nicolai::legacy_window_m36_direct(19).valid);
    assert(!nicolai::legacy_window_m36_direct(400).valid);
    assert(!nicolai::legacy_window_m36_direct(401).valid);
    assert(!nicolai::legacy_window_m36_lookup(0).valid);
    assert(!nicolai::legacy_window_m36_lookup(401).valid);

    std::cout << "legacy_window_m36_test: PASSED\n";
}
