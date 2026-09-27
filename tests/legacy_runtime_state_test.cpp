#include "nicolai/legacy_runtime_state.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

int main() {
    using nicolai::LegacyRuntimeStateM36;
    using nicolai::LegacyRuntimeWritePathM36;

    auto step = [](int count) {
        nicolai::LegacyTdsStepM33 out;
        out.valid = true;
        out.count = static_cast<std::int16_t>(count);
        out.first_period = 100;
        out.delta_q11 = 3;
        out.carry = 7;
        return out;
    };

    {
        auto r = nicolai::legacy_runtime_route_m36(0, 1, 5, true, true);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::Dropped);
        r = nicolai::legacy_runtime_route_m36(2, 1, 5, true, false);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::CrossTransition);
        r = nicolai::legacy_runtime_route_m36(2, 1, 5, false, false);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::InitialTransition);
        r = nicolai::legacy_runtime_route_m36(2, 3, 5, false, true);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::DeferredTerminal);
        r = nicolai::legacy_runtime_route_m36(2, 1, 5, false, true);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::Ordinary);
        assert(r.checkpoint_before_write);
        r = nicolai::legacy_runtime_route_m36(-1, 1, 5, false, true);
        assert(!r.valid && r.path == LegacyRuntimeWritePathM36::Invalid);
    }

    // Ordinary writes swap once: +0x78 retains the OLD +0x74 record.
    {
        LegacyRuntimeStateM36 s;
        s.word24 = 8; s.word26 = 3; s.word28 = 11; s.word2a = 4;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        buffers.valid = true;
        buffers.at_74.interval_index = 9;
        buffers.at_78.interval_index = 8;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, step(2), false, true);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::Ordinary);
        assert(r.step_buffers_written && r.interval_markers_rotated);
        assert(!r.terminal_markers_saved && !r.dropped);
        assert(s.word24 == 3 && s.word28 == 4 && s.word2a == 1 && s.word26 == 0);
        assert(buffers.valid && buffers.at_74.interval_index == 1 &&
            buffers.at_78.interval_index == 9);
        assert(buffers.at_74.step.count == 2);
    }

    // Initial/cross execute BOTH early and common rotations/swaps.
    for (bool cross : {false, true}) {
        LegacyRuntimeStateM36 s;
        s.word26 = 5; s.word2a = 6;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, step(1), cross, false);
        assert(r.valid && r.step_buffers_written && r.interval_markers_rotated);
        assert(s.word24 == 0 && s.word28 == 1 && s.word2a == 1 && s.word26 == 0);
        assert(buffers.at_74.interval_index == 1 && buffers.at_78.interval_index == 1);
    }

    // The positive terminal path swaps once but saves the previous
    // marker pair in +0x2c/+0x2e instead of rotating the current quartet.
    {
        LegacyRuntimeStateM36 s;
        s.word24 = 2; s.word26 = 7; s.word28 = 3; s.word2a = 4;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 3, 5, step(2), false, true);
        assert(r.valid && r.path == LegacyRuntimeWritePathM36::DeferredTerminal);
        assert(r.step_buffers_written && r.terminal_markers_saved);
        assert(!r.interval_markers_rotated);
        assert(s.word24 == 2 && s.word26 == 7 && s.word28 == 3 && s.word2a == 4);
        assert(s.word2c == 7 && s.word2e == 4);
    }

    // Pending-cross drops do nothing; other drops only set the bridge flag.
    {
        LegacyRuntimeStateM36 s;
        s.word26 = 4; s.word2a = 2;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        buffers.at_74.interval_index = 9;
        const auto before = buffers;
        auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, step(0), false, false);
        assert(r.valid && r.dropped && !r.step_buffers_written);
        assert(s.word26 == 1 && buffers.at_74.interval_index == before.at_74.interval_index);

        s.word26 = 4;
        r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, step(0), true, true);
        assert(r.valid && r.dropped && !r.step_buffers_written);
        assert(s.word26 == 4 && buffers.at_74.interval_index == before.at_74.interval_index);
    }

    // A started drop preserves the last POSITIVE step/marker, not this index.
    {
        LegacyRuntimeStateM36 s;
        s.word26 = 1; s.word2a = 0;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, step(0), false, true);
        assert(r.valid && r.dropped && !r.step_buffers_written);
        assert(!r.interval_markers_rotated && !r.terminal_markers_saved);
        assert(s.word24 == 0 && s.word28 == 0 && s.word2a == 0 && s.word26 == 1);
        assert(!buffers.valid);
    }

    // Dropped terminal bookkeeping does NOT perform the positive saved tail.
    // Its optional rewind belongs to the separate rollback primitive.
    {
        LegacyRuntimeStateM36 s;
        s.word26 = 9; s.word2a = 2;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 3, 5, step(0), false, true);
        assert(r.valid && r.dropped && !r.step_buffers_written &&
            !r.terminal_markers_saved);
        assert(s.word2c == 0 && s.word2e == 0 && s.word26 == 1 && s.word2a == 2);
    }

    // A terminal cross/initial route still reaches the common terminal tail.
    for (bool cross : {false, true}) {
        LegacyRuntimeStateM36 s;
        s.word26 = 5; s.word2a = 6;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        const auto r = nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 3, 5, step(1), cross, false);
        assert(r.valid && r.interval_markers_rotated && r.terminal_markers_saved);
        assert(s.word24 == 5 && s.word28 == 6 && s.word2a == 3 && s.word26 == 0);
        assert(s.word2c == 0 && s.word2e == 3);
        assert(buffers.at_74.interval_index == 3 && buffers.at_78.interval_index == 3);
    }

    // Invalid step/route inputs are transactional and leave both state and
    // the caller-owned step slots untouched.
    {
        LegacyRuntimeStateM36 s;
        s.word26 = 6; s.word2a = 2;
        nicolai::LegacyRuntimeStepBuffersM36 buffers;
        buffers.at_74.interval_index = 7;
        const auto before_state = s;
        const auto before_buffers = buffers;
        auto invalid = step(1);
        invalid.valid = false;
        assert(!nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 1, 5, invalid, false, true).valid);
        assert(s.word26 == before_state.word26 && s.word2a == before_state.word2a);
        assert(buffers.at_74.interval_index == before_buffers.at_74.interval_index);
        assert(!nicolai::legacy_runtime_bookkeep_m36(
            s, buffers, 4, 5, step(1), false, true).valid);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.selection_a = 7; s.selection_b = 9;
        nicolai::legacy_runtime_checkpoint_m36(s);
        assert(s.saved_cursor == 120 && s.saved_selection_a == 7 &&
            s.saved_selection_b == 9);
    }

    // Every successful writer call rotates the two cursor selections before
    // advancing +0x48 by the emitted period.
    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.selection_a = 70; s.selection_b = 30;
        const auto r = nicolai::legacy_runtime_post_write_m36(s, 45);
        assert(r.valid && r.old_cursor == 120 && r.new_cursor == 165);
        assert(s.cursor == 165 && s.selection_a == 120 && s.selection_b == 70);
        const auto before = s;
        assert(!nicolai::legacy_runtime_post_write_m36(s, 0).valid);
        assert(s.cursor == before.cursor && s.selection_a == before.selection_a &&
            s.selection_b == before.selection_b);

        s.cursor = std::numeric_limits<std::int32_t>::max() - 2;
        s.selection_a = 11;
        s.selection_b = 9;
        const auto wrapped = nicolai::legacy_runtime_post_write_m36(s, 5);
        assert(wrapped.valid &&
            s.cursor == std::numeric_limits<std::int32_t>::min() + 2);
        assert(s.selection_a == std::numeric_limits<std::int32_t>::max() - 2);
        assert(s.selection_b == 11);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.saved_cursor = 80; s.word26 = 5;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 2, 3, 5, 0, 0);
        assert(r.valid && !r.dropped && !r.rewound);
        assert(s.cursor == 120 && s.word26 == 5);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.saved_cursor = 80;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 1, 5, 0, 0);
        assert(r.valid && r.dropped && !r.rewound);
        assert(s.cursor == 120 && s.word26 == 1);
    }

    for (int gate_a : {0, 1}) for (int gate_b : {0, 1}) {
        if (gate_a == 0 && gate_b == 0) continue;
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.saved_cursor = 80;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, gate_a, gate_b);
        assert(r.valid && r.dropped && !r.rewound);
        assert(s.cursor == 120 && s.word26 == 1);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.clock_a = 100; s.clock_b = 20;
        s.selection_a = 10; s.selection_b = 11;
        s.saved_cursor = 80; s.saved_selection_a = 30; s.saved_selection_b = 31;
        s.end_cursor = 200; s.word24 = 41; s.word28 = 43;
        s.word2c = -1; s.word2e = -1; s.field30 = 99;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, 0, 0);
        assert(r.valid && r.dropped && r.rewound && r.cursor_delta == -40);
        assert(s.cursor == 80 && s.clock_a == 60 && s.clock_b == 0);
        assert(s.selection_a == 30 && s.selection_b == 31 && s.end_cursor == 80);
        assert(s.word2c == 41 && s.word2e == 43 && s.word26 == 1 && s.field30 == 0);
    }

    {
        LegacyRuntimeStateM36 s;
        s.cursor = 120; s.clock_a = 200; s.clock_b = 300;
        s.saved_cursor = 140; s.end_cursor = 100;
        const auto r = nicolai::legacy_runtime_drop_rollback_m36(
            s, 0, 3, 5, 0, 0);
        assert(r.rewound && r.cursor_delta == 20);
        assert(s.cursor == 140 && s.clock_a == 220 && s.clock_b == 320);
        assert(s.end_cursor == 100);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        LegacyRuntimeStateM36 s;
        const auto q = nicolai::legacy_runtime_normal_source_selection_m36(
            p, 2, s, 100);
        assert(q.valid && !q.bridged_drop && q.current_interval_width == 120);
        assert(q.left_interval_width == 120 && q.left_window_length == 100 &&
            q.right_window_length == 100);
        assert(q.left_source_position == 140 && q.right_source_position == 160);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        LegacyRuntimeStateM36 s; s.word26 = 1; s.word2a = 0;
        const auto q = nicolai::legacy_runtime_normal_source_selection_m36(
            p, 2, s, 100);
        assert(q.valid && q.bridged_drop && q.current_interval_width == 120);
        assert(q.left_interval_width == 90 && q.left_window_length == 90 &&
            q.right_window_length == 100);
        assert(q.left_source_position == 50 && q.right_source_position == 160);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        LegacyRuntimeStateM36 s; s.word26 = 1; s.word2a = 0;
        const auto q = nicolai::legacy_runtime_normal_source_selection_m36(
            p, 2, s, 60);
        assert(q.valid && q.bridged_drop && q.left_window_length == 60 &&
            q.right_window_length == 60);
        assert(q.left_source_position == 50 && q.right_source_position == 200);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260};
        LegacyRuntimeStateM36 s; s.word26 = 1; s.word2a = 2;
        assert(!nicolai::legacy_runtime_normal_source_selection_m36(
            p, 1, s, 80).valid);
    }

    // First-grain interval widths are signed WORD subtractions in the
    // original. A monotonic DWORD span outside the positive WORD domain must
    // not be silently accepted as a large portable support.
    {
        const std::vector<std::int32_t> p{0, 40000, 40100};
        LegacyRuntimeStateM36 s;
        assert(!nicolai::legacy_runtime_normal_source_selection_m36(
            p, 0, s, 80).valid);
        s.word2e = 0;
        assert(!nicolai::legacy_runtime_initial_source_selection_m36(
            p, s, 0, 80).valid);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        LegacyRuntimeStateM36 s; s.word2e = 0;
        const auto q = nicolai::legacy_runtime_initial_source_selection_m36(
            p, s, 2, 100);
        assert(q.valid && q.current_interval_width == 120 &&
            q.left_interval_width == 90);
        assert(q.left_window_length == 90 && q.right_window_length == 100);
        assert(q.left_source_position == 50 && q.right_source_position == 160);
    }

    {
        const std::vector<std::int32_t> p{0, 50, 140, 260, 400};
        LegacyRuntimeStateM36 s; s.word2e = 3;
        const auto q = nicolai::legacy_runtime_initial_source_selection_m36(
            p, s, 1, 80);
        assert(q.valid && q.current_interval_width == 90 &&
            q.left_interval_width == 140);
        assert(q.left_window_length == 80 && q.right_window_length == 80);
        assert(q.left_source_position == 400 && q.right_source_position == 60);
    }

    // Nonzero cross-descriptor branch: without saved-state override the old
    // side comes from the buffered interval. Its left support is independently
    // limited by that interval's own width before mixing with the new side.
    {
        const std::vector<std::int32_t> previous{0, 50, 140, 260, 400};
        const std::vector<std::int32_t> current{1000, 1080, 1190, 1320, 1470};
        LegacyRuntimeStateM36 s;
        const auto q = nicolai::legacy_runtime_cross_geometry_m36(
            previous, current, s, 2, 1);
        assert(q.valid && !q.used_saved_interval);
        assert(q.previous_interval_index == 2 && q.current_interval_index == 1);
        assert(q.previous_interval_width == 120);
        assert(q.previous_left_width == 90);
        assert(q.previous_window_length == 90);
        assert(q.current_interval_width == 110);
        assert(q.current_window_length == 110);
        assert(q.current_next_width == 130);
        assert(q.previous_source_start == 50);
        assert(q.current_source_start == 1080);
        assert(q.previous_forward_source_start == 140);
        assert(q.current_forward_source_start == 1190);
    }

    // With word2c set, cross transition ignores the buffered interval index and
    // uses word2e+1 from the saved previous descriptor state.
    {
        const std::vector<std::int32_t> previous{0, 50, 140, 260, 400};
        const std::vector<std::int32_t> current{1000, 1080, 1190, 1320, 1470};
        LegacyRuntimeStateM36 s;
        s.word2c = 1;
        s.word2e = 0;
        const auto q = nicolai::legacy_runtime_cross_geometry_m36(
            previous, current, s, 2, 2);
        assert(q.valid && q.used_saved_interval);
        assert(q.previous_interval_index == 1 && q.current_interval_index == 2);
        assert(q.previous_interval_width == 90);
        assert(q.previous_left_width == 50);
        assert(q.previous_window_length == 50);
        assert(q.current_interval_width == 130);
        assert(q.current_window_length == 90);
        assert(q.current_next_width == 150);
        assert(q.previous_source_start == 0);
        assert(q.current_source_start == 1230);
        assert(q.previous_forward_source_start == 50);
        assert(q.current_forward_source_start == 1320);
    }

    // At the terminal current interval 0x10108cf0 keeps the forward source on
    // the interval's left marker instead of reading a nonexistent next marker.
    {
        const std::vector<std::int32_t> previous{0, 50, 140, 260, 400};
        const std::vector<std::int32_t> current{1000, 1080, 1190, 1320, 1470};
        LegacyRuntimeStateM36 s;
        const auto q = nicolai::legacy_runtime_cross_geometry_m36(
            previous, current, s, 2, 3);
        assert(q.valid && q.current_interval_index == 3);
        assert(q.current_interval_width == 150);
        assert(q.current_next_width == 150);
        assert(q.current_forward_source_start == 1320);
    }

    {
        const std::vector<std::int32_t> previous{0, 50, 140, 260};
        const std::vector<std::int32_t> current{1000, 1080, 1190};
        LegacyRuntimeStateM36 s;
        s.word2c = 1;
        s.word2e = 2; // word2e+1 would be outside previous interval domain
        assert(!nicolai::legacy_runtime_cross_geometry_m36(
            previous, current, s, 0, 0).valid);
    }

    // 0x10108d51..0x10108da7 applies the reverse lookup window to the first
    // period written by the ordinary path. It is a fade-in: early samples are
    // strongly attenuated, the final sample is nearly unchanged, and data
    // outside that exact period remains untouched.
    {
        std::vector<std::int16_t> pcm(30, 16000);
        assert(nicolai::legacy_runtime_cross_zero_fade_m36(pcm, 5, 20));
        assert(pcm[4] == 16000 && pcm[25] == 16000);
        assert(pcm[5] >= 0 && pcm[5] < 1000);
        assert(pcm[24] > 15000 && pcm[24] <= 16000);
        for (std::size_t i = 6; i <= 24; ++i)
            assert(pcm[i] >= pcm[i - 1]);
        const auto before = pcm;
        assert(!nicolai::legacy_runtime_cross_zero_fade_m36(pcm, 20, 20));
        assert(pcm == before);
    }

    std::cout << "legacy_runtime_state_test: PASSED\n";
}
