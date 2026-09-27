#include "nicolai/legacy_runtime_grain_m36.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    using nicolai::legacy_runtime_ordinary_grain_m36;
    using nicolai::legacy_runtime_initial_repeated_grain_m36;

    {
        const std::vector<std::int32_t> p{0, 80, 180, 310};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 80, 1024, 1);
        assert(g.valid && !g.terminal_interval);
        assert(g.period == 81);
        assert(g.current_interval_width == 80);
        assert(g.next_interval_width == 100);
        assert(g.left_window_length == 81);
        assert(g.right_window_length == 80);
        assert(g.left_source_position == 80);
        assert(g.right_source_position == 0);
    }

    {
        const std::vector<std::int32_t> p{0, 120, 260};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 90, -2048, 2);
        assert(g.valid && g.period == 88);
        assert(g.left_window_length == 88 && g.right_window_length == 88);
        assert(g.left_source_position == 120 && g.right_source_position == 32);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 2, 100, 0, 1);
        assert(g.valid && !g.terminal_interval);
        assert(g.left_window_length == 100 && g.right_window_length == 100);
        assert(g.left_source_position == 260 && g.right_source_position == 160);
    }

    {
        const std::vector<std::int32_t> p{0, 80, 180, 310};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 2, 100, 0, 1);
        assert(g.valid && g.terminal_interval);
        assert(g.current_interval_width == 130 && g.next_interval_width == 130);
        assert(g.left_window_length == 100 && g.right_window_length == 100);
        assert(g.left_source_position == 310 && g.right_source_position == 210);
    }

    {
        const std::vector<std::int32_t> p{65536, 65616, 65716};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 80, 0, 1);
        assert(g.valid && g.current_interval_width == 80 &&
            g.next_interval_width == 100);
        assert(g.left_source_position == 65616 &&
            g.right_source_position == 65536);
    }

    // Initial-transition repeated grain staying inside the buffered descriptor.
    {
        const std::vector<std::int32_t> previous{0, 80, 180, 310};
        const std::vector<std::int32_t> next{1000, 1090, 1200};
        const auto g = legacy_runtime_initial_repeated_grain_m36(
            previous, next, 1, false, 80, 0, 1);
        assert(g.valid && !g.cross_descriptor);
        assert(g.current_interval_width == 100 && g.next_interval_width == 130);
        assert(g.left_window_length == 80 && g.right_window_length == 80);
        assert(g.left_source_position == 180 && g.right_source_position == 100);
    }

    // state+0x30 nonzero switches writer arg2 / the left-window input to the
    // next descriptor's first source interval. Writer arg3 remains the tail of
    // the buffered previous interval.
    {
        const std::vector<std::int32_t> previous{0, 80, 180, 310};
        const std::vector<std::int32_t> next{1000, 1090, 1200};
        const auto g = legacy_runtime_initial_repeated_grain_m36(
            previous, next, 1, true, 120, 0, 1);
        assert(g.valid && g.cross_descriptor);
        assert(g.current_interval_width == 100 && g.next_interval_width == 90);
        assert(g.left_window_length == 90 && g.right_window_length == 100);
        assert(g.left_source_position == 1000 && g.right_source_position == 80);
    }

    {
        const std::vector<std::int32_t> previous{0, 80};
        const std::vector<std::int32_t> next{1000, 1090};
        assert(!legacy_runtime_initial_repeated_grain_m36(
            previous, next, 0, false, 80, 0, 1).valid);
        assert(legacy_runtime_initial_repeated_grain_m36(
            previous, next, 0, true, 80, 0, 1).valid);
    }

    {
        const std::vector<std::int32_t> p{0, 100, 200};
        const auto g = legacy_runtime_ordinary_grain_m36(
            p, 0, 32760, 32767, 1);
        assert(!g.valid);
    }

    {
        const std::vector<std::int32_t> p{0, 100};
        assert(!legacy_runtime_ordinary_grain_m36(p, 1, 80, 0, 1).valid);
        assert(!legacy_runtime_ordinary_grain_m36(p, 0, 80, 0, 0).valid);
    }

    // After the buffered step, 0x101086c0 emits a current-step entry grain.
    // field30 decides whether writer-left starts at current[0] or remains on
    // the previous descriptor boundary; writer-right is always current.
    {
        const std::vector<std::int32_t> previous{0, 50, 140, 260, 400};
        const std::vector<std::int32_t> current{0, 80, 190, 320, 470};
        const auto crossed =
            nicolai::legacy_runtime_initial_current_entry_m36(
                previous, current, 2, 1, true, 90);
        assert(crossed.valid && crossed.left_from_current);
        assert(crossed.left_interval_width == 80 &&
            crossed.current_interval_width == 110);
        assert(crossed.left_window_length == 80 &&
            crossed.right_window_length == 90);
        assert(crossed.left_source_position == 0 &&
            crossed.right_source_position == 100);

        const auto stayed =
            nicolai::legacy_runtime_initial_current_entry_m36(
                previous, current, 2, 1, false, 90);
        assert(stayed.valid && !stayed.left_from_current);
        assert(stayed.left_interval_width == 140 &&
            stayed.current_interval_width == 110);
        assert(stayed.left_window_length == 90 &&
            stayed.right_window_length == 90);
        assert(stayed.left_source_position == 260 &&
            stayed.right_source_position == 100);

        assert(!nicolai::legacy_runtime_initial_current_entry_m36(
            previous, current, 3, 1, false, 90).valid);
        assert(!nicolai::legacy_runtime_initial_current_entry_m36(
            previous, current, 2, 1, true, 0).valid);
    }

    std::cout << "legacy_runtime_grain_m36_test: PASSED\n";
}
