#pragma once
#include <cstdint>

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

// Snapshot emitted by 0x10107e9f..0x10107ebb before the ordinary grain path.
void legacy_runtime_checkpoint_m36(LegacyRuntimeStateM36& state);

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

} // namespace nicolai
