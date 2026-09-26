#include "nicolai/legacy_runtime_state.hpp"

#include <cstdint>

namespace nicolai {
namespace {
std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}
}

void legacy_runtime_checkpoint_m36(LegacyRuntimeStateM36& state) {
    state.saved_cursor = state.cursor;
    state.saved_selection_a = state.selection_a;
    state.saved_selection_b = state.selection_b;
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

} // namespace nicolai
