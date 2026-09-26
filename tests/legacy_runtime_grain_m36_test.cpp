#include "nicolai/legacy_runtime_grain_m36.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    using nicolai::legacy_runtime_ordinary_grain_m36;

    {
        const std::vector<std::int32_t> p{0, 80, 180, 310};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 80, 1024, 1);
        assert(g.valid && !g.terminal_interval);
        assert(g.period == 81);
        assert(g.current_interval_width == 80);
        assert(g.next_interval_width == 100);
        assert(g.left_window_length == 80);
        assert(g.right_window_length == 81);
        assert(g.left_source_position == 0);
        assert(g.right_source_position == 80);
    }

    // Q11 correction is arithmetic-shifted after the original +1024 bias.
    {
        const std::vector<std::int32_t> p{0, 120, 260};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 90, -2048, 2);
        assert(g.valid && g.period == 88);
        assert(g.left_window_length == 88 && g.right_window_length == 88);
        assert(g.left_source_position == 32 && g.right_source_position == 120);
    }

    // Repeated grains are centred at pos[i+1]: the left source ends at that
    // boundary while the right source begins there. This is intentionally
    // different from the first-grain front/tail selection.
    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 2, 100, 0, 1);
        assert(g.valid && !g.terminal_interval);
        assert(g.left_window_length == 100 && g.right_window_length == 100);
        assert(g.left_source_position == 160 && g.right_source_position == 260);
    }

    // At the last interval, 0x1010858f..0x101085a8 reuses current width as
    // next width instead of reading beyond the source-position array.
    {
        const std::vector<std::int32_t> p{0, 80, 180, 310};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 2, 100, 0, 1);
        assert(g.valid && g.terminal_interval);
        assert(g.current_interval_width == 130 && g.next_interval_width == 130);
        assert(g.left_window_length == 100 && g.right_window_length == 100);
        assert(g.left_source_position == 210 && g.right_source_position == 310);
    }

    // Width subtraction in the original is a WORD operation even though the
    // source-position slots themselves are DWORDs.
    {
        const std::vector<std::int32_t> p{65536, 65616, 65716};
        const auto g = legacy_runtime_ordinary_grain_m36(p, 0, 80, 0, 1);
        assert(g.valid && g.current_interval_width == 80 &&
            g.next_interval_width == 100);
        assert(g.left_source_position == 65536 &&
            g.right_source_position == 65616);
    }

    // The period add is also a WORD add. An idealized int implementation would
    // return 32776 here; the x86 path wraps negative, outside this positive
    // portable contract, and must therefore be rejected.
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

    std::cout << "legacy_runtime_grain_m36_test: PASSED\n";
}
