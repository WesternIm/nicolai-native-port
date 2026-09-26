#include "nicolai/legacy_runtime_cross_m36.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using nicolai::LegacyRuntimeCrossShoulderM36;
    using nicolai::LegacyRuntimeCrossSourceM36;
    using nicolai::legacy_runtime_cross_buffered_repeat_plan_m36;
    using nicolai::legacy_runtime_cross_current_entry_plan_m36;
    using nicolai::legacy_runtime_cross_current_repeat_plan_m36;
    using nicolai::legacy_runtime_cross_entry_plan_m36;
    using nicolai::legacy_runtime_cross_overlap_sample_m36;
    using nicolai::legacy_runtime_cross_primary_layout_m36;
    using nicolai::legacy_runtime_cross_secondary_layout_m36;

    {
        const auto q = legacy_runtime_cross_primary_layout_m36(120, 90, 110);
        assert(q.valid && q.total_length == 120);
        assert(q.zero_prefix_length == 10);
        assert(q.shoulder_length == 20);
        assert(q.overlap_length == 90);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::Current);
        assert(q.zero_prefix_length + q.shoulder_length + q.overlap_length ==
            q.total_length);
    }

    {
        const auto q = legacy_runtime_cross_primary_layout_m36(120, 110, 90);
        assert(q.valid && q.zero_prefix_length == 10);
        assert(q.shoulder_length == 20 && q.overlap_length == 90);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::Previous);
    }

    {
        const auto q = legacy_runtime_cross_primary_layout_m36(120, 100, 100);
        assert(q.valid && q.zero_prefix_length == 20);
        assert(q.shoulder_length == 0 && q.overlap_length == 100);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::None);
    }

    assert(!legacy_runtime_cross_primary_layout_m36(100, 101, 80).valid);
    assert(!legacy_runtime_cross_primary_layout_m36(0, 0, 0).valid);

    {
        const auto q = legacy_runtime_cross_secondary_layout_m36(120, 100, 80);
        assert(q.valid && q.total_length == 100);
        assert(q.previous_extent == 100 && q.current_extent == 80);
        assert(q.overlap_length == 80 && q.shoulder_length == 20);
        assert(q.zero_suffix_length == 0);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::Previous);
    }

    {
        const auto q = legacy_runtime_cross_secondary_layout_m36(60, 100, 90);
        assert(q.valid && q.previous_extent == 60 && q.current_extent == 90);
        assert(q.overlap_length == 60 && q.shoulder_length == 30);
        assert(q.zero_suffix_length == 10);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::Current);
        assert(q.overlap_length + q.shoulder_length + q.zero_suffix_length ==
            q.total_length);
    }

    {
        const auto q = legacy_runtime_cross_secondary_layout_m36(120, 100, 140);
        assert(q.valid && q.previous_extent == 100 && q.current_extent == 100);
        assert(q.overlap_length == 100 && q.shoulder_length == 0);
        assert(q.zero_suffix_length == 0);
        assert(q.shoulder == LegacyRuntimeCrossShoulderM36::None);
    }

    assert(!legacy_runtime_cross_secondary_layout_m36(120, 0, 80).valid);

    // Phase 1: previous raw boundary against the tail of primary temp.
    {
        const auto p = legacy_runtime_cross_entry_plan_m36(120, 400, 100);
        assert(p.valid && p.period == 100);
        assert(p.left_source == LegacyRuntimeCrossSourceM36::PreviousPcm);
        assert(p.left_offset == 400 && p.left_window_length == 100);
        assert(p.right_source == LegacyRuntimeCrossSourceM36::PrimaryTemp);
        assert(p.right_offset == 20 && p.right_window_length == 100);
    }

    // Phase 2: later buffered-step grain = secondary temp against primary tail.
    {
        const auto p = legacy_runtime_cross_buffered_repeat_plan_m36(
            120, 90, 80, 2048, 2);
        assert(p.valid && p.period == 82);
        assert(p.left_source == LegacyRuntimeCrossSourceM36::SecondaryTemp);
        assert(p.left_offset == 0 && p.left_window_length == 82);
        assert(p.right_source == LegacyRuntimeCrossSourceM36::PrimaryTemp);
        assert(p.right_offset == 38 && p.right_window_length == 82);
    }

    // Phase 3: first current-step grain = secondary temp against current tail.
    {
        const auto p = legacy_runtime_cross_current_entry_plan_m36(90, 1000, 100);
        assert(p.valid && p.period == 100);
        assert(p.left_source == LegacyRuntimeCrossSourceM36::SecondaryTemp);
        assert(p.left_offset == 0 && p.left_window_length == 90);
        assert(p.right_source == LegacyRuntimeCrossSourceM36::CurrentPcm);
        assert(p.right_offset == 910 && p.right_window_length == 90);
    }

    // Phase 4: later current-step grain = current future boundary vs past tail.
    {
        const auto p = legacy_runtime_cross_current_repeat_plan_m36(
            90, 130, 1000, 80, 1024, 1);
        assert(p.valid && p.period == 81);
        assert(p.left_source == LegacyRuntimeCrossSourceM36::CurrentPcm);
        assert(p.left_offset == 1000 && p.left_window_length == 81);
        assert(p.right_source == LegacyRuntimeCrossSourceM36::CurrentPcm);
        assert(p.right_offset == 919 && p.right_window_length == 81);
    }

    // Writer plan period reconstruction keeps signed-WORD wrap semantics.
    assert(!legacy_runtime_cross_buffered_repeat_plan_m36(
        100, 100, 32760, 32767, 1).valid);
    assert(!legacy_runtime_cross_current_entry_plan_m36(0, 100, 80).valid);

    // Two nearly-unity Q15 products are summed and shifted by 16, so the
    // result is approximately one input amplitude, not two as a Q15 writer
    // would produce for the same pair.
    assert(legacy_runtime_cross_overlap_sample_m36(
        10000, 32767, 10000, 32767) == 9999);

    assert(legacy_runtime_cross_overlap_sample_m36(
        -10000, 32767, 0, 0) == -5000);

    assert(legacy_runtime_cross_overlap_sample_m36(
        -32768, -32768, -32768, -32768) == -32768);

    assert(legacy_runtime_cross_overlap_sample_m36(0, 32767, 0, 32767) == 0);

    std::cout << "legacy_runtime_cross_m36_test: PASSED\n";
}
