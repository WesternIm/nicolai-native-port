#include "nicolai/legacy_runtime_executor_m36.hpp"
#include "nicolai/legacy_runtime_state.hpp"
#include "nicolai/legacy_tds.hpp"
#include "nicolai/legacy_window_m36.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

nicolai::LegacyTdsStepM33 step(int count, int first_period, int delta_q11) {
    nicolai::LegacyTdsStepM33 out;
    out.valid = true;
    out.count = static_cast<std::int16_t>(count);
    out.first_period = static_cast<std::int16_t>(first_period);
    out.delta_q11 = static_cast<std::int16_t>(delta_q11);
    return out;
}

std::int16_t faded(std::int16_t sample, std::int16_t weight) {
    const auto product = static_cast<std::int64_t>(sample) * weight;
    const auto shifted = product >= 0 ? product / 32768 :
        -((-product + 32767) / 32768);
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(shifted));
}

} // namespace

int main() {
    const std::vector<std::int32_t> previous_positions{0, 50, 140, 260, 400};
    const std::vector<std::int32_t> current_positions{0, 80, 190, 320, 470};
    nicolai::LegacyRuntimeStateM36 geometry_state;
    const auto geometry = nicolai::legacy_runtime_cross_geometry_m36(
        previous_positions, current_positions, geometry_state, 2, 1);
    assert(geometry.valid);

    std::vector<std::int16_t> previous_pcm(400);
    std::vector<std::int16_t> current_pcm(470);
    for (std::size_t i = 0; i < previous_pcm.size(); ++i)
        previous_pcm[i] = static_cast<std::int16_t>(1000 + i * 17 % 9000);
    for (std::size_t i = 0; i < current_pcm.size(); ++i)
        current_pcm[i] = static_cast<std::int16_t>(-8000 + i * 19 % 12000);

    const auto buffered_step = step(3, 100, 0);
    const auto current_step = step(2, 90, 0);

    nicolai::LegacyRuntimeStateM36 state;
    state.cursor = 2;
    state.selection_a = 7;
    state.selection_b = 5;
    std::vector<std::int16_t> output{111, 222};

    // Build a one-grain reference directly from the already-proven buffer,
    // plan, window and writer primitives. The executor must produce the same
    // prefix before it advances through the remaining three phase families.
    const auto buffers = nicolai::legacy_runtime_cross_buffers_m36(
        previous_pcm, current_pcm, geometry);
    const auto first_plan = nicolai::legacy_runtime_cross_entry_plan_m36(
        geometry.previous_interval_width, 260, buffered_step.first_period);
    const auto first_left_window = nicolai::legacy_window_m36_lookup(
        first_plan.left_window_length);
    const auto first_right_window = nicolai::legacy_window_m36_lookup(
        first_plan.right_window_length);
    std::vector<std::int16_t> first_left(
        previous_pcm.begin() + first_plan.left_offset,
        previous_pcm.begin() + first_plan.left_offset +
            first_plan.left_window_length);
    std::vector<std::int16_t> first_right(
        buffers.primary.begin() + first_plan.right_offset,
        buffers.primary.begin() + first_plan.right_offset +
            first_plan.right_window_length);
    std::vector<std::int16_t> expected_first{111, 222};
    assert(nicolai::legacy_tds_write_m34(
        expected_first, 2, first_plan.period,
        first_left, first_right,
        first_left_window.q15, first_right_window.q15));

    const auto result = nicolai::legacy_runtime_execute_cross_m36(
        output, state, previous_pcm, current_pcm, geometry,
        260, 190, buffered_step, current_step);
    assert(result.valid);
    assert(result.buffered_grains_written == 3);
    assert(result.current_grains_written == 2);
    assert(result.total_grains_written == 5);
    assert(result.total_samples_written == 480);
    assert(result.start_cursor == 2 && result.end_cursor == 482);
    assert(output.size() == 482 && output[0] == 111 && output[1] == 222);
    for (std::size_t i = 0; i < expected_first.size(); ++i)
        assert(output[i] == expected_first[i]);

    // Five exact post-write rotations.
    assert(state.cursor == 482);
    assert(state.selection_a == 392);
    assert(state.selection_b == 302);

    // A source failure in the final current-repeat phase must not expose any
    // earlier staged PCM or state rotations to the caller.
    {
        nicolai::LegacyRuntimeStateM36 failed_state;
        failed_state.cursor = 2;
        failed_state.selection_a = 7;
        failed_state.selection_b = 5;
        std::vector<std::int16_t> failed_output{111, 222};
        const auto before_state = failed_state;
        const auto before_output = failed_output;
        const auto failed = nicolai::legacy_runtime_execute_cross_m36(
            failed_output, failed_state, previous_pcm, current_pcm, geometry,
            260, 450, buffered_step, current_step);
        assert(!failed.valid);
        assert(failed_output == before_output);
        assert(failed_state.cursor == before_state.cursor);
        assert(failed_state.selection_a == before_state.selection_a);
        assert(failed_state.selection_b == before_state.selection_b);
    }

    // Invalid or dropped step records are outside the nonzero cross executor.
    {
        auto dropped = buffered_step;
        dropped.count = 0;
        nicolai::LegacyRuntimeStateM36 untouched_state;
        std::vector<std::int16_t> untouched_output;
        assert(!nicolai::legacy_runtime_execute_cross_m36(
            untouched_output, untouched_state,
            previous_pcm, current_pcm, geometry,
            260, 190, dropped, current_step).valid);
        assert(untouched_output.empty() && untouched_state.cursor == 0);
    }

    // The post-loop 0x10108210 path writes exactly one terminal grain even
    // when the positive step count is larger, then applies the stored
    // descending window to that grain as a fade-out.
    {
        const std::vector<std::int32_t> terminal_positions{0, 50, 140, 260, 400};
        nicolai::LegacyRuntimeStateM36 terminal_state;
        terminal_state.cursor = 2;
        terminal_state.selection_a = 7;
        terminal_state.selection_b = 5;
        std::vector<std::int16_t> terminal_output{111, 222};
        const auto terminal_step = step(3, 100, 17);

        const auto selection =
            nicolai::legacy_runtime_normal_source_selection_m36(
                terminal_positions, 3, terminal_state,
                terminal_step.first_period);
        assert(selection.valid);
        const auto left_window = nicolai::legacy_window_m36_lookup(
            selection.left_window_length);
        const auto right_window = nicolai::legacy_window_m36_lookup(
            selection.right_window_length);
        const auto fade_window = nicolai::legacy_window_m36_lookup(
            terminal_step.first_period);
        std::vector<std::int16_t> left(
            previous_pcm.begin() + selection.left_source_position,
            previous_pcm.begin() + selection.left_source_position +
                selection.left_window_length);
        std::vector<std::int16_t> right(
            previous_pcm.begin() + selection.right_source_position,
            previous_pcm.begin() + selection.right_source_position +
                selection.right_window_length);
        std::vector<std::int16_t> expected{111, 222};
        assert(nicolai::legacy_tds_write_m34(
            expected, 2, terminal_step.first_period,
            left, right, left_window.q15, right_window.q15));
        for (int i = 0; i < terminal_step.first_period; ++i) {
            expected[static_cast<std::size_t>(2 + i)] = faded(
                expected[static_cast<std::size_t>(2 + i)],
                fade_window.q15[static_cast<std::size_t>(i)]);
        }

        const auto terminal = nicolai::legacy_runtime_execute_terminal_m36(
            terminal_output, terminal_state, previous_pcm,
            terminal_positions, 3, terminal_step);
        assert(terminal.valid && terminal.checkpointed &&
            terminal.fade_applied);
        assert(terminal.total_samples_written == 100);
        assert(terminal.start_cursor == 2 && terminal.end_cursor == 102);
        assert(terminal_output == expected);
        assert(terminal_state.cursor == 102);
        assert(terminal_state.selection_a == 2 &&
            terminal_state.selection_b == 7);
        assert(terminal_state.saved_cursor == 2 &&
            terminal_state.saved_selection_a == 7 &&
            terminal_state.saved_selection_b == 5);

        // Descending cache orientation means the terminal grain starts at
        // near-full gain and ends attenuated, opposite the cross fade-in.
        assert(fade_window.q15.front() > fade_window.q15.back());
    }

    // Dropped-source bridging also applies to 0x10108210. A late source-range
    // failure and a non-terminal request must leave PCM/state untouched.
    {
        const std::vector<std::int32_t> terminal_positions{0, 50, 140, 260, 400};
        nicolai::LegacyRuntimeStateM36 bridge_state;
        bridge_state.word26 = 1;
        bridge_state.word2a = 0;
        std::vector<std::int16_t> bridge_output;
        const auto terminal_step = step(1, 100, 0);
        const auto bridged = nicolai::legacy_runtime_execute_terminal_m36(
            bridge_output, bridge_state, previous_pcm,
            terminal_positions, 3, terminal_step);
        assert(bridged.valid && bridge_state.cursor == 100);

        std::vector<std::int16_t> short_pcm(350, 1);
        nicolai::LegacyRuntimeStateM36 failed_state;
        failed_state.cursor = 1;
        failed_state.selection_a = 9;
        std::vector<std::int16_t> failed_output{77};
        const auto before_state = failed_state;
        const auto before_output = failed_output;
        assert(!nicolai::legacy_runtime_execute_terminal_m36(
            failed_output, failed_state, short_pcm,
            terminal_positions, 3, terminal_step).valid);
        assert(failed_output == before_output);
        assert(failed_state.cursor == before_state.cursor &&
            failed_state.selection_a == before_state.selection_a &&
            failed_state.saved_cursor == before_state.saved_cursor);
        assert(!nicolai::legacy_runtime_execute_terminal_m36(
            failed_output, failed_state, previous_pcm,
            terminal_positions, 2, terminal_step).valid);
    }

    // Exact 0x101083b0 composition: first grain plus all Q11-progressed
    // repeats, each followed by the recovered cursor/selection rotation.
    {
        const std::vector<std::int32_t> positions{0, 50, 140, 260, 400};
        nicolai::LegacyRuntimeStateM36 ordinary_state;
        ordinary_state.cursor = 2;
        ordinary_state.selection_a = 7;
        ordinary_state.selection_b = 5;
        std::vector<std::int16_t> ordinary_output{111, 222};
        const auto ordinary_step = step(3, 100, 0);
        const auto ordinary = nicolai::legacy_runtime_execute_ordinary_m36(
            ordinary_output, ordinary_state, previous_pcm,
            positions, 2, ordinary_step);
        assert(ordinary.valid && !ordinary.cross_fade_in_applied);
        assert(ordinary.grains_written == 3);
        assert(ordinary.total_samples_written == 300);
        assert(ordinary.start_cursor == 2 && ordinary.end_cursor == 302);
        assert(ordinary_output.size() == 302);
        assert(ordinary_state.cursor == 302);
        assert(ordinary_state.selection_a == 202 &&
            ordinary_state.selection_b == 102);

        // The zero-byte cross wrapper is ordinary rendering followed by the
        // already-proven reverse-window fade over only the first period.
        auto expected_output = std::vector<std::int16_t>{111, 222};
        nicolai::LegacyRuntimeStateM36 expected_state;
        expected_state.cursor = 2;
        expected_state.selection_a = 7;
        expected_state.selection_b = 5;
        assert(nicolai::legacy_runtime_execute_ordinary_m36(
            expected_output, expected_state, previous_pcm,
            positions, 2, ordinary_step).valid);
        assert(nicolai::legacy_runtime_cross_zero_fade_m36(
            expected_output, 2, ordinary_step.first_period));

        auto zero_output = std::vector<std::int16_t>{111, 222};
        nicolai::LegacyRuntimeStateM36 zero_state;
        zero_state.cursor = 2;
        zero_state.selection_a = 7;
        zero_state.selection_b = 5;
        const auto zero = nicolai::legacy_runtime_execute_zero_cross_m36(
            zero_output, zero_state, previous_pcm,
            positions, 2, ordinary_step);
        assert(zero.valid && zero.cross_fade_in_applied);
        assert(zero_output == expected_output);
        assert(zero_state.cursor == expected_state.cursor &&
            zero_state.selection_a == expected_state.selection_a &&
            zero_state.selection_b == expected_state.selection_b);
    }

    // A repeat whose future-side slice falls outside the supplied PCM is a
    // late validation failure; no earlier grain or state rotation may leak.
    {
        const std::vector<std::int32_t> positions{0, 50, 140, 260, 400};
        const std::vector<std::int16_t> short_pcm(previous_pcm.begin(),
            previous_pcm.begin() + 300);
        nicolai::LegacyRuntimeStateM36 failed_state;
        failed_state.cursor = 2;
        failed_state.selection_a = 7;
        std::vector<std::int16_t> failed_output{111, 222};
        const auto before_state = failed_state;
        const auto before_output = failed_output;
        assert(!nicolai::legacy_runtime_execute_ordinary_m36(
            failed_output, failed_state, short_pcm,
            positions, 2, step(3, 100, 0)).valid);
        assert(failed_output == before_output);
        assert(failed_state.cursor == before_state.cursor &&
            failed_state.selection_a == before_state.selection_a);
    }

    std::cout << "legacy_runtime_executor_m36_test: PASSED\n";
}
