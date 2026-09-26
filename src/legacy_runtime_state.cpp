#include "nicolai/legacy_runtime_state.hpp"
#include "nicolai/legacy_window_m36.hpp"

#include <algorithm>
#include <cstdint>

namespace nicolai {
namespace {
std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

int sar15(std::int64_t value) {
    return value >= 0 ? static_cast<int>(value / 32768) :
        -static_cast<int>((-value + 32767) / 32768);
}

bool monotonic_positions(const std::vector<std::int32_t>& positions) {
    if (positions.size() < 2) return false;
    for (std::size_t i = 1; i < positions.size(); ++i)
        if (positions[i] <= positions[i - 1]) return false;
    return true;
}
}

void legacy_runtime_checkpoint_m36(LegacyRuntimeStateM36& state) {
    state.saved_cursor = state.cursor;
    state.saved_selection_a = state.selection_a;
    state.saved_selection_b = state.selection_b;
}

LegacyRuntimePostWriteM36 legacy_runtime_post_write_m36(
    LegacyRuntimeStateM36& state,
    int period) {
    LegacyRuntimePostWriteM36 out;
    if (period <= 0 || period > 32767) return out;

    out.old_cursor = state.cursor;
    state.selection_b = state.selection_a;
    state.selection_a = state.cursor;
    state.cursor = wrap32(static_cast<std::int64_t>(state.cursor) + period);
    out.new_cursor = state.cursor;
    out.valid = true;
    return out;
}

LegacyRuntimeRouteM36 legacy_runtime_route_m36(
    int step_count,
    int interval_index,
    int node_count,
    bool cross_descriptor_pending,
    bool already_started) {
    LegacyRuntimeRouteM36 out;
    if (node_count < 2 || interval_index < 0 ||
        interval_index >= node_count - 1 || step_count < 0 ||
        step_count > 32767) return out;

    out.valid = true;
    if (step_count == 0) {
        out.path = LegacyRuntimeWritePathM36::Dropped;
        return out;
    }
    if (cross_descriptor_pending) {
        out.path = LegacyRuntimeWritePathM36::CrossTransition;
        return out;
    }
    if (!already_started) {
        out.path = LegacyRuntimeWritePathM36::InitialTransition;
        return out;
    }
    if (interval_index == node_count - 2) {
        out.path = LegacyRuntimeWritePathM36::DeferredTerminal;
        return out;
    }
    out.path = LegacyRuntimeWritePathM36::Ordinary;
    out.checkpoint_before_write = true;
    return out;
}

LegacyRuntimeRollbackM36 legacy_runtime_drop_rollback_m36(
    LegacyRuntimeStateM36& state,
    int step_count,
    int interval_index,
    int node_count,
    int gate_a,
    int gate_b) {
    LegacyRuntimeRollbackM36 out;
    if (node_count < 2 || interval_index < 0 ||
        interval_index >= node_count - 1 || step_count < 0 ||
        step_count > 32767) return out;

    out.valid = true;
    if (step_count != 0) return out;
    out.dropped = true;

    if (gate_a == 0 && interval_index == node_count - 2 && gate_b == 0) {
        const auto old_cursor = state.cursor;
        state.cursor = state.saved_cursor;
        const auto delta = wrap32(
            static_cast<std::int64_t>(state.saved_cursor) - old_cursor);
        out.cursor_delta = delta;
        state.clock_a = wrap32(static_cast<std::int64_t>(state.clock_a) + delta);
        state.clock_b = wrap32(static_cast<std::int64_t>(state.clock_b) + delta);
        state.selection_a = state.saved_selection_a;
        state.selection_b = state.saved_selection_b;
        if (state.saved_cursor < state.end_cursor)
            state.end_cursor = state.saved_cursor;
        if (state.clock_a < 0) state.clock_a = 0;
        if (state.clock_b < 0) state.clock_b = 0;
        state.word2e = state.word28;
        state.word2c = state.word24;
        state.field30 = 0;
        out.rewound = true;
    }
    state.word26 = 1;
    return out;
}

LegacyRuntimeSourceSelectionM36 legacy_runtime_normal_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyRuntimeStateM36& state,
    int first_period) {
    LegacyRuntimeSourceSelectionM36 out;
    if (!monotonic_positions(source_positions) || first_period <= 0 ||
        interval_index < 0 ||
        interval_index >= static_cast<int>(source_positions.size()) - 1)
        return out;

    const int current_width = static_cast<int>(
        source_positions[static_cast<std::size_t>(interval_index + 1)] -
        source_positions[static_cast<std::size_t>(interval_index)]);
    if (current_width <= 0) return out;

    int left_width = current_width;
    int left_position = source_positions[static_cast<std::size_t>(interval_index)];
    const bool bridge = state.word26 != 0;
    if (bridge) {
        const int dropped = static_cast<int>(state.word2a);
        if (dropped < 0 || dropped + 2 >= static_cast<int>(source_positions.size()))
            return out;
        left_position = source_positions[static_cast<std::size_t>(dropped + 1)];
        left_width = static_cast<int>(
            source_positions[static_cast<std::size_t>(dropped + 2)] -
            source_positions[static_cast<std::size_t>(dropped + 1)]);
        if (left_width <= 0) return out;
    }

    const int left_length = std::min(left_width, first_period);
    const int right_length = std::min(current_width, first_period);
    const int right_position = static_cast<int>(
        source_positions[static_cast<std::size_t>(interval_index + 1)]) - right_length;

    out.valid = true;
    out.bridged_drop = bridge;
    out.current_interval_width = current_width;
    out.left_interval_width = left_width;
    out.left_window_length = left_length;
    out.right_window_length = right_length;
    out.left_source_position = left_position;
    out.right_source_position = right_position;
    return out;
}

LegacyRuntimeSourceSelectionM36 legacy_runtime_initial_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    const LegacyRuntimeStateM36& state,
    int buffered_interval_index,
    int buffered_first_period) {
    LegacyRuntimeSourceSelectionM36 out;
    if (!monotonic_positions(source_positions) || buffered_first_period <= 0 ||
        buffered_interval_index < 0 ||
        buffered_interval_index >= static_cast<int>(source_positions.size()) - 1)
        return out;

    const int saved = static_cast<int>(state.word2e);
    const int intervals = static_cast<int>(source_positions.size()) - 1;
    if (saved < 0 || saved >= intervals) return out;

    const int current_width = static_cast<int>(
        source_positions[static_cast<std::size_t>(buffered_interval_index + 1)] -
        source_positions[static_cast<std::size_t>(buffered_interval_index)]);
    if (current_width <= 0) return out;

    int left_width = 0;
    if (saved < intervals - 1) {
        left_width = static_cast<int>(
            source_positions[static_cast<std::size_t>(saved + 2)] -
            source_positions[static_cast<std::size_t>(saved + 1)]);
    } else {
        left_width = static_cast<int>(
            source_positions[static_cast<std::size_t>(saved + 1)] -
            source_positions[static_cast<std::size_t>(saved)]);
    }
    if (left_width <= 0) return out;

    const int left_length = std::min(left_width, buffered_first_period);
    const int right_length = std::min(current_width, buffered_first_period);

    out.valid = true;
    out.current_interval_width = current_width;
    out.left_interval_width = left_width;
    out.left_window_length = left_length;
    out.right_window_length = right_length;
    out.left_source_position =
        static_cast<int>(source_positions[static_cast<std::size_t>(saved + 1)]);
    out.right_source_position = static_cast<int>(
        source_positions[static_cast<std::size_t>(buffered_interval_index + 1)]) -
        right_length;
    return out;
}

LegacyRuntimeCrossGeometryM36 legacy_runtime_cross_geometry_m36(
    const std::vector<std::int32_t>& previous_positions,
    const std::vector<std::int32_t>& current_positions,
    const LegacyRuntimeStateM36& state,
    int buffered_interval_index,
    int current_interval_index) {
    LegacyRuntimeCrossGeometryM36 out;
    if (!monotonic_positions(previous_positions) ||
        !monotonic_positions(current_positions)) return out;
    const int previous_intervals = static_cast<int>(previous_positions.size()) - 1;
    const int current_intervals = static_cast<int>(current_positions.size()) - 1;
    if (buffered_interval_index < 0 || buffered_interval_index >= previous_intervals ||
        current_interval_index < 0 || current_interval_index >= current_intervals)
        return out;

    const bool use_saved = state.word2c != 0;
    const int previous_index = use_saved ? static_cast<int>(state.word2e) + 1 :
        buffered_interval_index;
    if (previous_index < 0 || previous_index >= previous_intervals) return out;

    const int previous_width = static_cast<int>(
        previous_positions[static_cast<std::size_t>(previous_index + 1)] -
        previous_positions[static_cast<std::size_t>(previous_index)]);
    const int previous_left_width = previous_index > 0 ? static_cast<int>(
        previous_positions[static_cast<std::size_t>(previous_index)] -
        previous_positions[static_cast<std::size_t>(previous_index - 1)]) :
        previous_width;
    const int current_width = static_cast<int>(
        current_positions[static_cast<std::size_t>(current_interval_index + 1)] -
        current_positions[static_cast<std::size_t>(current_interval_index)]);
    const int current_next_width = current_interval_index < current_intervals - 1 ?
        static_cast<int>(
            current_positions[static_cast<std::size_t>(current_interval_index + 2)] -
            current_positions[static_cast<std::size_t>(current_interval_index + 1)]) :
        current_width;
    if (previous_width <= 0 || previous_left_width <= 0 || current_width <= 0 ||
        current_next_width <= 0) return out;

    const int previous_window = std::min(previous_left_width, previous_width);
    const int current_window = std::min(current_width, previous_width);

    out.valid = true;
    out.used_saved_interval = use_saved;
    out.previous_interval_index = previous_index;
    out.current_interval_index = current_interval_index;
    out.previous_interval_width = previous_width;
    out.previous_left_width = previous_left_width;
    out.previous_window_length = previous_window;
    out.current_interval_width = current_width;
    out.current_window_length = current_window;
    out.current_next_width = current_next_width;
    out.previous_source_start = std::max(0,
        static_cast<int>(previous_positions[static_cast<std::size_t>(previous_index)]) -
            previous_window);
    out.current_source_start = std::max(0,
        static_cast<int>(current_positions[static_cast<std::size_t>(current_interval_index + 1)]) -
            current_window);
    out.previous_forward_source_start = static_cast<int>(
        previous_positions[static_cast<std::size_t>(previous_index)]);
    out.current_forward_source_start = current_interval_index < current_intervals - 1 ?
        static_cast<int>(current_positions[static_cast<std::size_t>(current_interval_index + 1)]) :
        static_cast<int>(current_positions[static_cast<std::size_t>(current_interval_index)]);
    return out;
}

bool legacy_runtime_cross_zero_fade_m36(
    std::vector<std::int16_t>& output,
    std::size_t start_cursor,
    int first_period) {
    if (first_period <= 0 || first_period > 400 ||
        start_cursor > output.size() ||
        static_cast<std::size_t>(first_period) > output.size() - start_cursor)
        return false;
    const auto window = legacy_window_m36_lookup(first_period);
    if (!window.valid || window.q15.size() != static_cast<std::size_t>(first_period))
        return false;

    for (int i = 0; i < first_period; ++i) {
        const int w = window.q15[static_cast<std::size_t>(first_period - 1 - i)];
        const int sample = output[start_cursor + static_cast<std::size_t>(i)];
        output[start_cursor + static_cast<std::size_t>(i)] =
            static_cast<std::int16_t>(sar15(static_cast<std::int64_t>(sample) * w));
    }
    return true;
}

} // namespace nicolai
