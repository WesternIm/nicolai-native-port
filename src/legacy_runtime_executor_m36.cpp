#include "nicolai/legacy_runtime_executor_m36.hpp"
#include "nicolai/legacy_window_m36.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace nicolai {
namespace {

struct PlannedWriteM36 {
    LegacyRuntimeCrossWriterPlanM36 plan;
    bool buffered = false;
};

bool slice_fits(
    const std::vector<std::int16_t>& source,
    int offset,
    int length) {
    if (offset < 0 || length <= 0) return false;
    const auto begin = static_cast<std::size_t>(offset);
    const auto count = static_cast<std::size_t>(length);
    return begin <= source.size() && count <= source.size() - begin;
}

const std::vector<std::int16_t>* source_for(
    LegacyRuntimeCrossSourceM36 source,
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossBuffersM36& buffers) {
    switch (source) {
    case LegacyRuntimeCrossSourceM36::PreviousPcm:
        return &previous_pcm;
    case LegacyRuntimeCrossSourceM36::CurrentPcm:
        return &current_pcm;
    case LegacyRuntimeCrossSourceM36::PrimaryTemp:
        return &buffers.primary;
    case LegacyRuntimeCrossSourceM36::SecondaryTemp:
        return &buffers.secondary;
    default:
        return nullptr;
    }
}

bool validate_plan(
    const PlannedWriteM36& write,
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossBuffersM36& buffers) {
    const auto& plan = write.plan;
    if (!plan.valid || plan.period <= 0 || plan.period > 3200 ||
        plan.left_window_length <= 0 ||
        plan.right_window_length <= 0 ||
        plan.left_window_length > plan.period ||
        plan.right_window_length > plan.period)
        return false;

    const auto* left = source_for(
        plan.left_source, previous_pcm, current_pcm, buffers);
    const auto* right = source_for(
        plan.right_source, previous_pcm, current_pcm, buffers);
    if (left == nullptr || right == nullptr ||
        !slice_fits(*left, plan.left_offset, plan.left_window_length) ||
        !slice_fits(*right, plan.right_offset, plan.right_window_length))
        return false;

    const auto left_window = legacy_window_m36_lookup(plan.left_window_length);
    const auto right_window = legacy_window_m36_lookup(plan.right_window_length);
    return left_window.valid && right_window.valid &&
        left_window.q15.size() ==
            static_cast<std::size_t>(plan.left_window_length) &&
        right_window.q15.size() ==
            static_cast<std::size_t>(plan.right_window_length);
}

bool execute_plan(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const PlannedWriteM36& write,
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossBuffersM36& buffers) {
    const auto& plan = write.plan;
    const auto* left_source = source_for(
        plan.left_source, previous_pcm, current_pcm, buffers);
    const auto* right_source = source_for(
        plan.right_source, previous_pcm, current_pcm, buffers);
    if (left_source == nullptr || right_source == nullptr || state.cursor < 0)
        return false;

    const auto left_begin = left_source->begin() + plan.left_offset;
    const auto right_begin = right_source->begin() + plan.right_offset;
    const std::vector<std::int16_t> left(
        left_begin, left_begin + plan.left_window_length);
    const std::vector<std::int16_t> right(
        right_begin, right_begin + plan.right_window_length);
    const auto left_window = legacy_window_m36_lookup(plan.left_window_length);
    const auto right_window = legacy_window_m36_lookup(plan.right_window_length);
    if (!legacy_tds_write_m34(
            output,
            static_cast<std::size_t>(state.cursor),
            plan.period,
            left,
            right,
            left_window.q15,
            right_window.q15))
        return false;
    return legacy_runtime_post_write_m36(state, plan.period).valid;
}

} // namespace

LegacyRuntimeCrossExecutionM36 legacy_runtime_execute_cross_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossGeometryM36& geometry,
    int previous_boundary,
    int current_boundary,
    const LegacyTdsStepM33& buffered_step,
    const LegacyTdsStepM33& current_step) {
    LegacyRuntimeCrossExecutionM36 result;
    if (!buffered_step.valid || !current_step.valid ||
        buffered_step.count <= 0 || current_step.count <= 0 ||
        state.cursor < 0 ||
        static_cast<std::size_t>(state.cursor) > output.size())
        return result;

    const auto buffers = legacy_runtime_cross_buffers_m36(
        previous_pcm, current_pcm, geometry);
    if (!buffers.valid) return result;

    std::vector<PlannedWriteM36> writes;
    writes.reserve(static_cast<std::size_t>(buffered_step.count) +
        static_cast<std::size_t>(current_step.count));
    writes.push_back({legacy_runtime_cross_entry_plan_m36(
        geometry.previous_interval_width,
        previous_boundary,
        buffered_step.first_period), true});
    for (int ordinal = 1; ordinal < buffered_step.count; ++ordinal) {
        writes.push_back({legacy_runtime_cross_buffered_repeat_plan_m36(
            geometry.previous_interval_width,
            geometry.current_interval_width,
            buffered_step.first_period,
            buffered_step.delta_q11,
            ordinal), true});
    }
    writes.push_back({legacy_runtime_cross_current_entry_plan_m36(
        geometry.current_interval_width,
        current_boundary,
        current_step.first_period), false});
    for (int ordinal = 1; ordinal < current_step.count; ++ordinal) {
        writes.push_back({legacy_runtime_cross_current_repeat_plan_m36(
            geometry.current_interval_width,
            geometry.current_next_width,
            current_boundary,
            current_step.first_period,
            current_step.delta_q11,
            ordinal), false});
    }

    std::int64_t final_cursor = state.cursor;
    for (const auto& write : writes) {
        if (!validate_plan(
                write, previous_pcm, current_pcm, buffers))
            return result;
        final_cursor += write.plan.period;
        if (final_cursor > std::numeric_limits<std::int32_t>::max())
            return result;
    }

    auto staged_output = output;
    auto staged_state = state;
    int buffered_written = 0;
    int current_written = 0;
    for (const auto& write : writes) {
        if (!execute_plan(staged_output, staged_state, write,
                previous_pcm, current_pcm, buffers))
            return result;
        if (write.buffered) ++buffered_written;
        else ++current_written;
    }

    result.valid = true;
    result.buffered_grains_written = buffered_written;
    result.current_grains_written = current_written;
    result.total_grains_written = buffered_written + current_written;
    result.total_samples_written = staged_state.cursor - state.cursor;
    result.start_cursor = state.cursor;
    result.end_cursor = staged_state.cursor;
    output = std::move(staged_output);
    state = staged_state;
    return result;
}

} // namespace nicolai
