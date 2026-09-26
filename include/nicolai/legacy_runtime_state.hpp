#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nicolai {

struct LegacyRuntimeStateM36 {
    std::int32_t cursor = 0;              // +0x48
    std::int32_t clock_a = 0;             // +0x3c
    std::int32_t clock_b = 0;             // +0x40
    std::int32_t selection_a = 0;         // +0x4c
    std::int32_t selection_b = 0;         // +0x50
    std::int32_t saved_cursor = 0;        // +0x54
    std::int32_t saved_selection_a = 0;   // +0x58
    std::int32_t saved_selection_b = 0;   // +0x5c
    std::int32_t end_cursor = 0;          // +0x80
    std::int16_t word24 = 0;
    std::int16_t word26 = 0;
    std::int16_t word28 = 0;
    std::int16_t word2a = 0;
    std::int16_t word2c = 0;
    std::int16_t word2e = 0;
    std::int32_t field30 = 0;
};

struct LegacyRuntimeRollbackM36 {
    bool valid = false;
    bool dropped = false;
    bool rewound = false;
    std::int32_t cursor_delta = 0;
};

enum class LegacyRuntimeWritePathM36 {
    Invalid = 0,
    Dropped,
    CrossTransition,
    InitialTransition,
    Ordinary,
    DeferredTerminal,
};

struct LegacyRuntimeRouteM36 {
    bool valid = false;
    LegacyRuntimeWritePathM36 path = LegacyRuntimeWritePathM36::Invalid;
    bool checkpoint_before_write = false;
};

struct LegacyRuntimeSourceSelectionM36 {
    bool valid = false;
    bool bridged_drop = false;
    int current_interval_width = 0;
    int left_interval_width = 0;
    int left_window_length = 0;
    int right_window_length = 0;
    int left_source_position = 0;
    int right_source_position = 0;
};

void legacy_runtime_checkpoint_m36(LegacyRuntimeStateM36& state);

LegacyRuntimeRouteM36 legacy_runtime_route_m36(
    int step_count,
    int interval_index,
    int node_count,
    bool cross_descriptor_pending,
    bool already_started);

LegacyRuntimeRollbackM36 legacy_runtime_drop_rollback_m36(
    LegacyRuntimeStateM36& state,
    int step_count,
    int interval_index,
    int node_count,
    int gate_a,
    int gate_b);

LegacyRuntimeSourceSelectionM36 legacy_runtime_normal_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyRuntimeStateM36& state,
    int first_period);

LegacyRuntimeSourceSelectionM36 legacy_runtime_initial_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    const LegacyRuntimeStateM36& state,
    int buffered_interval_index,
    int buffered_first_period);

// Zero-byte branch of 0x10108cf0: after ordinary 0x101083b0 rendering, the
// original multiplies the first `first_period` newly-written PCM samples by
// the recovered window in reverse order. This is the descriptor-boundary
// fade-in before 0x10109fc0 releases temporary lookup buffers.
bool legacy_runtime_cross_zero_fade_m36(
    std::vector<std::int16_t>& output,
    std::size_t start_cursor,
    int first_period);

} // namespace nicolai
