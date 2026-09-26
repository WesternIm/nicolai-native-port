#include "nicolai/legacy_runtime_state.hpp"

#include <cassert>
#include <iostream>

int main() {
    using nicolai::LegacyRuntimeStateM36;

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.selection_a = 7;
        s.selection_b = 9;
        nicolai::legacy_runtime_checkpoint_m36(s);
        assert(s.saved_cursor == 120);
        assert(s.saved_selection_a == 7);
        assert(s.saved_selection_b == 9);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.saved_cursor = 80;
        s.word26 = 5;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 2, 3, 5, 0, 0);
        assert(r.valid && !r.dropped && !r.rewound);
        assert(s.cursor == 120);
        assert(s.word26 == 5);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.saved_cursor = 80;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 1, 5, 0, 0);
        assert(r.valid && r.dropped && !r.rewound);
        assert(s.cursor == 120);
        assert(s.word26 == 1);
    }

    for (int gate_a : {0, 1}) for (int gate_b : {0, 1}) {
        if (gate_a == 0 && gate_b == 0) continue;
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.saved_cursor = 80;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, gate_a, gate_b);
        assert(r.valid && r.dropped && !r.rewound);
        assert(s.cursor == 120);
        assert(s.word26 == 1);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.clock_a = 100;
        s.clock_b = 20;
        s.selection_a = 10;
        s.selection_b = 11;
        s.saved_cursor = 80;
        s.saved_selection_a = 30;
        s.saved_selection_b = 31;
        s.end_cursor = 200;
        s.word24 = 41;
        s.word28 = 43;
        s.word2c = -1;
        s.word2e = -1;
        s.field30 = 99;

        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, 0, 0);
        assert(r.valid && r.dropped && r.rewound);
        assert(r.cursor_delta == -40);
        assert(s.cursor == 80);
        assert(s.clock_a == 60);
        assert(s.clock_b == 0); // original clamps negative clocks after rewind
        assert(s.selection_a == 30);
        assert(s.selection_b == 31);
        assert(s.end_cursor == 80);
        assert(s.word2c == 41);
        assert(s.word2e == 43);
        assert(s.word26 == 1);
        assert(s.field30 == 0);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120;
        s.clock_a = 200;
        s.clock_b = 300;
        s.saved_cursor = 140;
        s.end_cursor = 100;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, 0, 0);
        assert(r.rewound);
        assert(r.cursor_delta == 20);
        assert(s.cursor == 140);
        assert(s.clock_a == 220);
        assert(s.clock_b == 320);
        assert(s.end_cursor == 100); // +0x80 is untouched when saved >= marker
    }

    std::cout << "legacy_runtime_state_test: PASSED\n";
}
