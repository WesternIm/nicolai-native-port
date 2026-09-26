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

} // namespace nicolai
