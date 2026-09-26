#pragma once

#include "nicolai/legacy_runtime_cross_m36.hpp"
#include "nicolai/legacy_runtime_state.hpp"
#include "nicolai/legacy_tds.hpp"

#include <cstdint>
#include <vector>

namespace nicolai {

// Result of the opt-in nonzero 0x10108cf0 PCM executor. The executor composes
// already-proven M36 primitives only; it is not connected to production or to
// StatefulTdsM34.
struct LegacyRuntimeCrossExecutionM36 {
    bool valid = false;
    int buffered_grains_written = 0;
    int current_grains_written = 0;
    int total_grains_written = 0;
    int total_samples_written = 0;
    std::int32_t start_cursor = 0;
    std::int32_t end_cursor = 0;
};

// Result of the deferred terminal PCM path. This covers the checkpoint,
// 0x10108210 single-grain write and the caller-side descending-window fade.
// Descriptor metadata finalization after that fade remains outside the
// portable contract.
struct LegacyRuntimeTerminalExecutionM36 {
    bool valid = false;
    bool checkpointed = false;
    bool fade_applied = false;
    int total_samples_written = 0;
    std::int32_t start_cursor = 0;
    std::int32_t end_cursor = 0;
};

// Result of the 0x101083b0 ordinary writer sequence and its zero-flag
// 0x10108cf0 wrapper. Checkpointing and caller-side bookkeeping are deliberately
// not implicit: the original ordinary and zero-cross call sites differ there.
struct LegacyRuntimeOrdinaryExecutionM36 {
    bool valid = false;
    bool cross_fade_in_applied = false;
    int grains_written = 0;
    int total_samples_written = 0;
    std::int32_t start_cursor = 0;
    std::int32_t end_cursor = 0;
};

// Execute all four writer phases of the proven nonzero cross path:
//   1) buffered entry; 2) buffered repeats;
//   3) current entry;  4) current repeats.
//
// previous_boundary and current_boundary are absolute sample coordinates in
// their respective PCM vectors. Both output and runtime state are committed
// only if buffer materialization, every writer plan, every exact window and
// every source slice validate successfully.
LegacyRuntimeCrossExecutionM36 legacy_runtime_execute_cross_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossGeometryM36& geometry,
    int previous_boundary,
    int current_boundary,
    const LegacyTdsStepM33& buffered_step,
    const LegacyTdsStepM33& current_step);

// Execute the proven deferred-terminal PCM sequence transactionally. The
// original calls 0x10108210 only for the last interval, writes one grain using
// step.first_period (regardless of a larger positive step.count), rotates the
// runtime cursor/selections, then fades that grain with the recovered window
// in its stored descending order.
LegacyRuntimeTerminalExecutionM36 legacy_runtime_execute_terminal_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step);

// Execute the positive-count body of original 0x101083b0 transactionally.
// This is the function body only: the route-level ordinary caller owns its
// pre-write checkpoint and marker/step-buffer updates.
LegacyRuntimeOrdinaryExecutionM36 legacy_runtime_execute_ordinary_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step);

// Zero-byte branch of 0x10108cf0: execute 0x101083b0 and apply the reverse
// recovered window to the first emitted period as a fade-in. The complete
// wrapper is one output/state transaction.
LegacyRuntimeOrdinaryExecutionM36 legacy_runtime_execute_zero_cross_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step);

} // namespace nicolai
