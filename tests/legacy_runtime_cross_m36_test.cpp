#include "nicolai/legacy_runtime_cross_m36.hpp"
#include "nicolai/legacy_runtime_state.hpp"
#include "nicolai/legacy_window_m36.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using nicolai::LegacyRuntimeCrossShoulderM36;
    using nicolai::LegacyRuntimeCrossSourceM36;
    using nicolai::legacy_runtime_cross_buffered_repeat_plan_m36;
    using nicolai::legacy_runtime_cross_buffers_m36;
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

    // Exact nonzero-cross temporary buffers. Primary aligns the two source
    // spans at the tail and reads both windows backwards. Secondary aligns at
    // the front and reads its windows forwards.
    {
        const std::vector<std::int32_t> previous_positions{0, 50, 140, 260, 400};
        const std::vector<std::int32_t> current_positions{0, 80, 190, 320, 470};
        nicolai::LegacyRuntimeStateM36 state;
        const auto geometry = nicolai::legacy_runtime_cross_geometry_m36(
            previous_positions, current_positions, state, 2, 1);
        assert(geometry.valid);

        std::vector<std::int16_t> previous_pcm(400, -123);
        std::vector<std::int16_t> current_pcm(470, -456);
        for (int i = 50; i < 140; ++i) previous_pcm[i] = 10000;
        for (int i = 140; i < 250; ++i) previous_pcm[i] = 12000;
        for (int i = 80; i < 190; ++i) current_pcm[i] = 20000;
        for (int i = 190; i < 300; ++i) current_pcm[i] = 22000;
        const auto buffers = legacy_runtime_cross_buffers_m36(
            previous_pcm, current_pcm, geometry);
        assert(buffers.valid && buffers.primary.size() == 120);
        assert(buffers.secondary.size() == 110);

        const auto previous90 = nicolai::legacy_window_m36_lookup(90);
        const auto current110 = nicolai::legacy_window_m36_lookup(110);
        assert(previous90.valid && current110.valid);
        for (int i = 0; i < 10; ++i)
            assert(buffers.primary[static_cast<std::size_t>(i)] == 0);
        assert(buffers.primary[10] == static_cast<std::int16_t>(
            (20000 * static_cast<int>(current110.q15[109])) >> 15));
        assert(buffers.primary[29] == static_cast<std::int16_t>(
            (20000 * static_cast<int>(current110.q15[90])) >> 15));
        assert(buffers.primary[30] == legacy_runtime_cross_overlap_sample_m36(
            10000, previous90.q15[89], 20000, current110.q15[89]));
        assert(buffers.primary[119] == legacy_runtime_cross_overlap_sample_m36(
            10000, previous90.q15[0], 20000, current110.q15[0]));

        // Equal forward extents make all of secondary a SAR16 overlap.
        assert(buffers.secondary[0] == legacy_runtime_cross_overlap_sample_m36(
            12000, current110.q15[0], 22000, current110.q15[0]));
        assert(buffers.secondary[109] == legacy_runtime_cross_overlap_sample_m36(
            12000, current110.q15[109], 22000, current110.q15[109]));
    }

    // Current-owned forward shoulder followed by the exact zero suffix.
    {
        const std::vector<std::int32_t> previous_positions{0, 50, 110, 200};
        const std::vector<std::int32_t> current_positions{0, 100, 190};
        nicolai::LegacyRuntimeStateM36 state;
        const auto geometry = nicolai::legacy_runtime_cross_geometry_m36(
            previous_positions, current_positions, state, 1, 0);
        std::vector<std::int16_t> previous_pcm(200, 1000);
        std::vector<std::int16_t> current_pcm(190, -2000);
        const auto buffers = legacy_runtime_cross_buffers_m36(
            previous_pcm, current_pcm, geometry);
        assert(buffers.valid && buffers.secondary.size() == 100);
        const auto previous60 = nicolai::legacy_window_m36_lookup(60);
        const auto current90 = nicolai::legacy_window_m36_lookup(90);
        assert(previous60.valid && current90.valid);
        assert(buffers.secondary[59] == legacy_runtime_cross_overlap_sample_m36(
            1000, previous60.q15[59], -2000, current90.q15[59]));
        assert(buffers.secondary[60] == static_cast<std::int16_t>(
            -((2000 * static_cast<int>(current90.q15[60]) + 32767) / 32768)));
        assert(buffers.secondary[89] == static_cast<std::int16_t>(
            -((2000 * static_cast<int>(current90.q15[89]) + 32767) / 32768)));
        for (int i = 90; i < 100; ++i)
            assert(buffers.secondary[static_cast<std::size_t>(i)] == 0);
    }

    // Previous-owned forward shoulder, plus guarded rejection when a PCM span
    // cannot satisfy the statically recovered source offsets.
    {
        const std::vector<std::int32_t> previous_positions{0, 80, 200, 300};
        const std::vector<std::int32_t> current_positions{0, 100, 180};
        nicolai::LegacyRuntimeStateM36 state;
        const auto geometry = nicolai::legacy_runtime_cross_geometry_m36(
            previous_positions, current_positions, state, 1, 0);
        std::vector<std::int16_t> previous_pcm(300, 3000);
        std::vector<std::int16_t> current_pcm(180, 4000);
        const auto buffers = legacy_runtime_cross_buffers_m36(
            previous_pcm, current_pcm, geometry);
        assert(buffers.valid && buffers.secondary.size() == 100);
        const auto previous100 = nicolai::legacy_window_m36_lookup(100);
        assert(previous100.valid);
        assert(buffers.secondary[80] == static_cast<std::int16_t>(
            (3000 * static_cast<int>(previous100.q15[80])) >> 15));
        assert(buffers.secondary[99] == static_cast<std::int16_t>(
            (3000 * static_cast<int>(previous100.q15[99])) >> 15));
        current_pcm.resize(100);
        assert(!legacy_runtime_cross_buffers_m36(
            previous_pcm, current_pcm, geometry).valid);
    }

    {
        nicolai::LegacyRuntimeCrossGeometryM36 invalid;
        assert(!legacy_runtime_cross_buffers_m36({}, {}, invalid).valid);
    }

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
