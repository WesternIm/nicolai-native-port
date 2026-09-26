#include "nicolai/legacy_runtime_executor_m36.hpp"
#include "nicolai/legacy_runtime_grain_m36.hpp"
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

struct OrdinaryWriteM36 {
    int period = 0;
    int left_offset = 0;
    int left_length = 0;
    int right_offset = 0;
    int right_length = 0;
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

std::int16_t terminal_fade_sample(std::int16_t sample, std::int16_t weight) {
    const auto product = static_cast<std::int64_t>(sample) * weight;
    const auto shifted = product >= 0 ? product / 32768 :
        -((-product + 32767) / 32768);
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(shifted));
}

bool validate_ordinary_write(
    const OrdinaryWriteM36& write,
    const std::vector<std::int16_t>& pcm) {
    if (write.period <= 0 || write.period > 3200 ||
        write.left_length <= 0 || write.right_length <= 0 ||
        write.left_length > write.period || write.right_length > write.period ||
        !slice_fits(pcm, write.left_offset, write.left_length) ||
        !slice_fits(pcm, write.right_offset, write.right_length))
        return false;
    const auto left_window = legacy_window_m36_lookup(write.left_length);
    const auto right_window = legacy_window_m36_lookup(write.right_length);
    return left_window.valid && right_window.valid &&
        left_window.q15.size() == static_cast<std::size_t>(write.left_length) &&
        right_window.q15.size() == static_cast<std::size_t>(write.right_length);
}

bool execute_ordinary_write(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const OrdinaryWriteM36& write) {
    if (state.cursor < 0) return false;
    const auto left_begin = pcm.begin() + write.left_offset;
    const auto right_begin = pcm.begin() + write.right_offset;
    const std::vector<std::int16_t> left(
        left_begin, left_begin + write.left_length);
    const std::vector<std::int16_t> right(
        right_begin, right_begin + write.right_length);
    const auto left_window = legacy_window_m36_lookup(write.left_length);
    const auto right_window = legacy_window_m36_lookup(write.right_length);
    return legacy_tds_write_m34(
               output,
               static_cast<std::size_t>(state.cursor),
               write.period,
               left,
               right,
               left_window.q15,
               right_window.q15) &&
        legacy_runtime_post_write_m36(state, write.period).valid;
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

LegacyRuntimeTerminalExecutionM36 legacy_runtime_execute_terminal_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step) {
    LegacyRuntimeTerminalExecutionM36 result;
    if (!step.valid || step.count <= 0 || step.first_period <= 0 ||
        source_positions.size() < 2 ||
        interval_index != static_cast<int>(source_positions.size()) - 2 ||
        state.cursor < 0 ||
        static_cast<std::size_t>(state.cursor) > output.size())
        return result;

    const auto selection = legacy_runtime_normal_source_selection_m36(
        source_positions, interval_index, state, step.first_period);
    if (!selection.valid ||
        !slice_fits(pcm, selection.left_source_position,
            selection.left_window_length) ||
        !slice_fits(pcm, selection.right_source_position,
            selection.right_window_length))
        return result;

    const auto left_window =
        legacy_window_m36_lookup(selection.left_window_length);
    const auto right_window =
        legacy_window_m36_lookup(selection.right_window_length);
    const auto fade_window = legacy_window_m36_lookup(step.first_period);
    if (!left_window.valid || !right_window.valid || !fade_window.valid ||
        left_window.q15.size() !=
            static_cast<std::size_t>(selection.left_window_length) ||
        right_window.q15.size() !=
            static_cast<std::size_t>(selection.right_window_length) ||
        fade_window.q15.size() != static_cast<std::size_t>(step.first_period))
        return result;

    const auto final_cursor = static_cast<std::int64_t>(state.cursor) +
        step.first_period;
    if (final_cursor > std::numeric_limits<std::int32_t>::max()) return result;

    const auto left_begin = pcm.begin() + selection.left_source_position;
    const auto right_begin = pcm.begin() + selection.right_source_position;
    const std::vector<std::int16_t> left(
        left_begin, left_begin + selection.left_window_length);
    const std::vector<std::int16_t> right(
        right_begin, right_begin + selection.right_window_length);

    auto staged_output = output;
    auto staged_state = state;
    const auto start_cursor = staged_state.cursor;
    legacy_runtime_checkpoint_m36(staged_state);
    if (!legacy_tds_write_m34(
            staged_output,
            static_cast<std::size_t>(start_cursor),
            step.first_period,
            left,
            right,
            left_window.q15,
            right_window.q15) ||
        !legacy_runtime_post_write_m36(
            staged_state, step.first_period).valid)
        return result;

    for (int i = 0; i < step.first_period; ++i) {
        const auto at = static_cast<std::size_t>(start_cursor + i);
        staged_output[at] = terminal_fade_sample(
            staged_output[at], fade_window.q15[static_cast<std::size_t>(i)]);
    }

    result.valid = true;
    result.checkpointed = true;
    result.fade_applied = true;
    result.total_samples_written = step.first_period;
    result.start_cursor = start_cursor;
    result.end_cursor = staged_state.cursor;
    output = std::move(staged_output);
    state = staged_state;
    return result;
}

LegacyRuntimeOrdinaryExecutionM36 legacy_runtime_execute_ordinary_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step) {
    LegacyRuntimeOrdinaryExecutionM36 result;
    if (!step.valid || step.count <= 0 || state.cursor < 0 ||
        static_cast<std::size_t>(state.cursor) > output.size())
        return result;

    const auto first = legacy_runtime_normal_source_selection_m36(
        source_positions, interval_index, state, step.first_period);
    if (!first.valid) return result;

    std::vector<OrdinaryWriteM36> writes;
    writes.reserve(static_cast<std::size_t>(step.count));
    writes.push_back({
        step.first_period,
        first.left_source_position,
        first.left_window_length,
        first.right_source_position,
        first.right_window_length});
    for (int ordinal = 1; ordinal < step.count; ++ordinal) {
        const auto grain = legacy_runtime_ordinary_grain_m36(
            source_positions,
            interval_index,
            step.first_period,
            step.delta_q11,
            ordinal);
        if (!grain.valid) return result;
        writes.push_back({
            grain.period,
            grain.left_source_position,
            grain.left_window_length,
            grain.right_source_position,
            grain.right_window_length});
    }

    std::int64_t final_cursor = state.cursor;
    for (const auto& write : writes) {
        if (!validate_ordinary_write(write, pcm)) return result;
        final_cursor += write.period;
        if (final_cursor > std::numeric_limits<std::int32_t>::max())
            return result;
    }

    auto staged_output = output;
    auto staged_state = state;
    for (const auto& write : writes) {
        if (!execute_ordinary_write(
                staged_output, staged_state, pcm, write))
            return result;
    }

    result.valid = true;
    result.grains_written = static_cast<int>(writes.size());
    result.total_samples_written = staged_state.cursor - state.cursor;
    result.start_cursor = state.cursor;
    result.end_cursor = staged_state.cursor;
    output = std::move(staged_output);
    state = staged_state;
    return result;
}

LegacyRuntimeOrdinaryExecutionM36 legacy_runtime_execute_zero_cross_m36(
    std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& state,
    const std::vector<std::int16_t>& pcm,
    const std::vector<std::int32_t>& source_positions,
    int interval_index,
    const LegacyTdsStepM33& step) {
    LegacyRuntimeOrdinaryExecutionM36 result;
    auto staged_output = output;
    auto staged_state = state;
    const auto ordinary = legacy_runtime_execute_ordinary_m36(
        staged_output,
        staged_state,
        pcm,
        source_positions,
        interval_index,
        step);
    if (!ordinary.valid || !legacy_runtime_cross_zero_fade_m36(
            staged_output,
            static_cast<std::size_t>(ordinary.start_cursor),
            step.first_period))
        return result;

    result = ordinary;
    result.cross_fade_in_applied = true;
    output = std::move(staged_output);
    state = staged_state;
    return result;
}

} // namespace nicolai
