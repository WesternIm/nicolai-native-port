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

    std::cout << "legacy_runtime_executor_m36_test: PASSED\n";
}
