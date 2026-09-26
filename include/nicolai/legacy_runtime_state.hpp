#pragma once
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

// Per-interval dispatcher visible at 0x10107d54..0x10107ebe.
// DeferredTerminal means the positive-count final interval is not sent through
// the ordinary writer in-loop; the original handles terminal work after the
// interval loop through a separate path.
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

// First-grain source ownership for ordinary and initial-transition paths.
// Positions are source sample coordinates from descriptor +0x30. Window lengths
// are the source support lengths passed to 0x10109980 after clamping by period.
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

// Snapshot emitted by 0x10107e9f..0x10107ebb before the ordinary grain path.
void legacy_runtime_checkpoint_m36(LegacyRuntimeStateM36& state);

// Exact branch selection after 0x10109800 produced a step record. The original
// tests positive/zero count first, then an outer cross-descriptor gate, then an
// utterance-local "already started" flag, and only then the terminal interval.
LegacyRuntimeRouteM36 legacy_runtime_route_m36(
    int step_count,
    int interval_index,
    int node_count,
    bool cross_descriptor_pending,
    bool already_started);

// Exact state mutation visible in the dropped-step branch at 0x10107f65.
// gate_a is the low WORD tested from the third caller argument; gate_b is the
// WORD cached from the caller-side +0x7c field. Their higher-level semantics
// remain intentionally unnamed until a runtime oracle labels them.
LegacyRuntimeRollbackM36 legacy_runtime_drop_rollback_m36(
    LegacyRuntimeStateM36& state,
    int step_count,
    int interval_index,
    int node_count,
    int gate_a,
    int gate_b);

// Exact first write selection at 0x10108426..0x10108516 for the ordinary
// 0x101083b0 path. When +0x26 records a preceding drop, the left source bridges
// from source[+0x2a+1] instead of source[current].
LegacyRuntimeSourceSelectionM36 legacy_runtime_normal_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyRuntimeStateM36& state,
    int first_period);

// Exact first write selection at 0x10108765..0x10108841 in 0x101086c0.
// The left source starts at source[state.word2e+1]. Its support normally uses
// the following interval (word2e+1 -> word2e+2), falling back to the current
// interval at the terminal boundary. The right source is the tail ending at
// source[buffered_interval+1].
LegacyRuntimeSourceSelectionM36 legacy_runtime_initial_source_selection_m36(
    const std::vector<std::int32_t>& source_positions,
    const LegacyRuntimeStateM36& state,
    int buffered_interval_index,
    int buffered_first_period);

} // namespace nicolai
