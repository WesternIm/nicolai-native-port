#pragma once

#include <cstdint>
#include <vector>

#include "nicolai/legacy_runtime_state.hpp"

namespace nicolai {

enum class LegacyRuntimeCrossShoulderM36 {
    None = 0,
    Previous,
    Current,
};

enum class LegacyRuntimeCrossSourceM36 {
    Invalid = 0,
    PreviousPcm,
    CurrentPcm,
    PrimaryTemp,
    SecondaryTemp,
};

// One exact call-plan for legacy_tds_write_m34 / original 0x10109980.
// left/right are writer arg2/arg3 ownership; offsets are sample coordinates
// inside the selected source kind (absolute source coordinates for raw PCM,
// zero-based offsets for temporary buffers).
struct LegacyRuntimeCrossWriterPlanM36 {
    bool valid = false;
    int period = 0;
    LegacyRuntimeCrossSourceM36 left_source = LegacyRuntimeCrossSourceM36::Invalid;
    int left_offset = 0;
    int left_window_length = 0;
    LegacyRuntimeCrossSourceM36 right_source = LegacyRuntimeCrossSourceM36::Invalid;
    int right_offset = 0;
    int right_window_length = 0;
};

// Region ownership of the first temporary PCM buffer in the nonzero branch of
// 0x10108cf0. The buffer length is previous_interval_width and partitions into
// zero-prefix, at most one single-source shoulder, then the two-source overlap.
struct LegacyRuntimeCrossPrimaryLayoutM36 {
    bool valid = false;
    int total_length = 0;
    int zero_prefix_length = 0;
    int shoulder_length = 0;
    int overlap_length = 0;
    LegacyRuntimeCrossShoulderM36 shoulder = LegacyRuntimeCrossShoulderM36::None;
};

// Region ownership of the second temporary PCM buffer. Here the order is the
// mirror image: two-source overlap first, optional one-source shoulder, then a
// zero suffix. previous_extent=min(previous_width,current_width) and
// current_extent=min(current_width,current_next_width).
struct LegacyRuntimeCrossSecondaryLayoutM36 {
    bool valid = false;
    int total_length = 0;
    int previous_extent = 0;
    int current_extent = 0;
    int overlap_length = 0;
    int shoulder_length = 0;
    int zero_suffix_length = 0;
    LegacyRuntimeCrossShoulderM36 shoulder = LegacyRuntimeCrossShoulderM36::None;
};

// Exact temporary PCM buffers constructed by the nonzero branch of
// 0x10108cf0. They are kept separate from the runtime executor: primary is
// consumed by the buffered-step writer phases, while secondary bridges into
// the current descriptor.
struct LegacyRuntimeCrossBuffersM36 {
    bool valid = false;
    std::vector<std::int16_t> primary;
    std::vector<std::int16_t> secondary;
};

// Value-level binding of the two descriptor-owned PCM bases used by original
// 0x10108cf0. The original receives previous/current descriptor pointers and
// reads source positions from +0x30 and PCM bases from +0x4c. Portable code
// supplies the already-materialized vectors instead of exposing those raw
// proprietary pointers; this result preserves the recovered owner/index and
// writer-boundary relationship.
struct LegacyRuntimeCrossSourceContextM36 {
    bool valid = false;
    bool used_saved_interval = false;
    int previous_interval_index = 0;
    int current_interval_index = 0;
    int previous_boundary = 0;
    int current_boundary = 0;
    int previous_pcm_samples = 0;
    int current_pcm_samples = 0;
    LegacyRuntimeCrossGeometryM36 geometry;
};

LegacyRuntimeCrossPrimaryLayoutM36 legacy_runtime_cross_primary_layout_m36(
    int previous_interval_width,
    int previous_window_length,
    int current_window_length);

LegacyRuntimeCrossSecondaryLayoutM36 legacy_runtime_cross_secondary_layout_m36(
    int previous_interval_width,
    int current_interval_width,
    int current_next_width);

// Materialize both temporary buffers from the source spans selected by
// legacy_runtime_cross_geometry_m36(). Exact recovered M36 windows are used.
// The function is deliberately pure and does not mutate renderer/runtime
// state or write output PCM.
LegacyRuntimeCrossBuffersM36 legacy_runtime_cross_buffers_m36(
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossGeometryM36& geometry);

// Bind the caller-owned previous/current descriptor source context before a
// nonzero cross executor is called. This is deliberately a pure value-level
// contract: it validates the exact source spans needed by the recovered
// temporary buffers and derives the raw writer boundaries from the selected
// descriptor intervals. It does not mutate runtime state or output PCM.
LegacyRuntimeCrossSourceContextM36 legacy_runtime_cross_source_context_m36(
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int32_t>& previous_positions,
    const std::vector<std::int16_t>& current_pcm,
    const std::vector<std::int32_t>& current_positions,
    const LegacyRuntimeStateM36& state,
    int buffered_interval_index,
    int current_interval_index);

// Writer phase 1, 0x1010944a..0x10109480: raw previous PCM at its selected
// boundary against the tail of primary temp.
LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_entry_plan_m36(
    int previous_interval_width,
    int previous_boundary,
    int first_period);

// Writer phase 2, 0x101094be..0x101095d0: secondary temp against the tail of
// primary temp for later grains of the buffered step record.
LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_buffered_repeat_plan_m36(
    int previous_interval_width,
    int current_interval_width,
    int first_period,
    int delta_q11,
    int ordinal);

// Writer phase 3, 0x101095d6..0x1010967c: secondary temp against the tail of
// current raw PCM for the first grain of the current step record.
LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_current_entry_plan_m36(
    int current_interval_width,
    int current_boundary,
    int first_period);

// Writer phase 4, 0x101096b7..0x101097cb: current raw PCM on both sides for
// later current-step grains, in the same writer orientation as ordinary repeat.
LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_current_repeat_plan_m36(
    int current_interval_width,
    int current_next_width,
    int current_boundary,
    int first_period,
    int delta_q11,
    int ordinal);

// Central two-source overlap kernel from the nonzero branch of 0x10108cf0,
// specifically 0x10109098..0x101090e0. Unlike the ordinary Q15 writer, the
// two products are added in wrapped 32-bit arithmetic and shifted by 16.
std::int16_t legacy_runtime_cross_overlap_sample_m36(
    std::int16_t left_sample,
    std::int16_t left_window_q15,
    std::int16_t right_sample,
    std::int16_t right_window_q15);

} // namespace nicolai
