#include "nicolai/legacy_phone_features.hpp"

#include <cassert>
#include <climits>
#include <iostream>

int main() {
    {
        const auto r = nicolai::legacy_phone_duration_m36(80, 120, 200, 260, 100);
        assert(r.valid);
        assert(r.previous_right_support == 40);
        assert(r.next_left_support == 60);
        assert(r.combined_source_support == 100);
        assert(r.duration_q11 == 2048);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 40, 100, 160, 50);
        assert(r.valid);
        assert(r.combined_source_support == 100);
        assert(r.duration_q11 == 1024);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 2048, 0, 2048, 1);
        assert(r.valid);
        assert(r.combined_source_support == 4096);
        assert(r.duration_q11 == 1); // positive zero-result fallback
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(10, 9, 0, 10, 20);
        assert(!r.valid);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 10, 20, 10, 20);
        assert(!r.valid);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 0, 0, 0, 20);
        assert(!r.valid);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 10, 0, 10, 0);
        assert(!r.valid);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 1, 0, 1, INT_MAX);
        assert(!r.valid); // do not invent original overflow/wrap behavior
    }

    std::cout << "legacy_phone_features_test: PASSED\n";
}
