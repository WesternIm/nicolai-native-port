#include "nicolai/legacy_runtime_state.hpp"

#include <algorithm>
#include <cstdint>

namespace nicolai {
namespace {
std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
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

        // 0x1010a860 returns state +0x48. In this rollback call its return is
        // assigned to +0x80 only when saved cursor is before the prior marker.
        if (state.saved_cursor < state.end_cursor)
            state.end_cursor = state.saved_cursor;

        if (state.clock_a < 0) state.clock_a = 0;
        if (state.clock_b < 0) state.clock_b = 0;
        state.word2e = state.word28;
        state.word2c = state.word24;
        state.field30 = 0;
        out.rewound = true;
    }

    // Common dropped-step tail at 0x10107ffe.
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
    int left_position =
        source_positions[static_cast<std::size_t>(interval_index)];
    const bool bridge = state.word26 != 0;
    if (bridge) {
        const int dropped = static_cast<int>(state.word2a);
        // 0x10108440 reads source[dropped+2]-source[dropped+1] and
        // 0x101084cc selects source[dropped+1] as the first source pointer.
        if (dropped < 0 ||
            dropped + 2 >= static_cast<int>(source_positions.size()))
            return out;
        left_position =
            source_positions[static_cast<std::size_t>(dropped + 1)];
        left_width = static_cast<int>(
            source_positions[static_cast<std::size_t>(dropped + 2)] -
            source_positions[static_cast<std::size_t>(dropped + 1)]);
        if (left_width <= 0) return out;
    }

    const int left_length = std::min(left_width, first_period);
    const int right_length = std::min(current_width, first_period);
    const int right_position = static_cast<int>(
        source_positions[static_cast<std::size_t>(interval_index + 1)]) -
        right_length;

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

} // namespace nicolai
