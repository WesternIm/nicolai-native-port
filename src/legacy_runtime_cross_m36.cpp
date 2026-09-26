#include "nicolai/legacy_runtime_cross_m36.hpp"
#include "nicolai/legacy_window_m36.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace nicolai {
namespace {

std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

int sar11(std::int64_t value) {
    return value >= 0 ? static_cast<int>(value / 2048) :
        -static_cast<int>((-value + 2047) / 2048);
}

int sar16(std::int32_t value) {
    const std::int64_t v = value;
    return v >= 0 ? static_cast<int>(v / 65536) :
        -static_cast<int>((-v + 65535) / 65536);
}

int sar15(std::int32_t value) {
    const std::int64_t v = value;
    return v >= 0 ? static_cast<int>(v / 32768) :
        -static_cast<int>((-v + 32767) / 32768);
}

std::int16_t word(int value) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(value));
}

int repeated_period(int first_period, int delta_q11, int ordinal) {
    if (first_period <= 0 || first_period > 32767 ||
        delta_q11 < -32768 || delta_q11 > 32767 ||
        ordinal < 1 || ordinal > 32767) return 0;
    const auto correction = sar11(
        static_cast<std::int64_t>(delta_q11) * ordinal + 1024);
    const int period = word(first_period + correction);
    return period > 0 ? period : 0;
}

bool safe_offset(std::int64_t value) {
    return value >= std::numeric_limits<int>::min() &&
        value <= std::numeric_limits<int>::max();
}

bool source_slice_fits(
    const std::vector<std::int16_t>& pcm,
    int start,
    int length) {
    if (start < 0 || length <= 0) return false;
    const auto offset = static_cast<std::size_t>(start);
    const auto count = static_cast<std::size_t>(length);
    return offset <= pcm.size() && count <= pcm.size() - offset;
}

std::int16_t q15_sample(std::int16_t sample, std::int16_t window) {
    const auto product = static_cast<std::int32_t>(sample) * window;
    return word(sar15(product));
}

LegacyRuntimeCrossShoulderM36 shoulder_for(int previous_extent, int current_extent) {
    if (previous_extent > current_extent)
        return LegacyRuntimeCrossShoulderM36::Previous;
    if (current_extent > previous_extent)
        return LegacyRuntimeCrossShoulderM36::Current;
    return LegacyRuntimeCrossShoulderM36::None;
}

} // namespace

LegacyRuntimeCrossPrimaryLayoutM36 legacy_runtime_cross_primary_layout_m36(
    int previous_interval_width,
    int previous_window_length,
    int current_window_length) {
    LegacyRuntimeCrossPrimaryLayoutM36 out;
    if (previous_interval_width <= 0 || previous_window_length < 0 ||
        current_window_length < 0 ||
        previous_window_length > previous_interval_width ||
        current_window_length > previous_interval_width)
        return out;

    const int larger = std::max(previous_window_length, current_window_length);
    const int smaller = std::min(previous_window_length, current_window_length);
    out.valid = true;
    out.total_length = previous_interval_width;
    out.zero_prefix_length = previous_interval_width - larger;
    out.shoulder_length = larger - smaller;
    out.overlap_length = smaller;
    out.shoulder = shoulder_for(previous_window_length, current_window_length);
    return out;
}

LegacyRuntimeCrossSecondaryLayoutM36 legacy_runtime_cross_secondary_layout_m36(
    int previous_interval_width,
    int current_interval_width,
    int current_next_width) {
    LegacyRuntimeCrossSecondaryLayoutM36 out;
    if (previous_interval_width <= 0 || current_interval_width <= 0 ||
        current_next_width <= 0) return out;

    const int previous_extent =
        std::min(previous_interval_width, current_interval_width);
    const int current_extent =
        std::min(current_interval_width, current_next_width);
    const int larger = std::max(previous_extent, current_extent);
    const int smaller = std::min(previous_extent, current_extent);

    out.valid = true;
    out.total_length = current_interval_width;
    out.previous_extent = previous_extent;
    out.current_extent = current_extent;
    out.overlap_length = smaller;
    out.shoulder_length = larger - smaller;
    out.zero_suffix_length = current_interval_width - larger;
    out.shoulder = shoulder_for(previous_extent, current_extent);
    return out;
}

LegacyRuntimeCrossBuffersM36 legacy_runtime_cross_buffers_m36(
    const std::vector<std::int16_t>& previous_pcm,
    const std::vector<std::int16_t>& current_pcm,
    const LegacyRuntimeCrossGeometryM36& geometry) {
    LegacyRuntimeCrossBuffersM36 out;
    if (!geometry.valid || geometry.previous_interval_width <= 0 ||
        geometry.previous_interval_width > 32767 ||
        geometry.current_interval_width <= 0 ||
        geometry.current_interval_width > 32767 ||
        geometry.previous_window_length <= 0 ||
        geometry.current_window_length <= 0 ||
        geometry.current_next_width <= 0)
        return out;

    const int previous_extent = std::min(
        geometry.previous_interval_width, geometry.current_interval_width);
    const int current_extent = std::min(
        geometry.current_interval_width, geometry.current_next_width);
    if (geometry.previous_window_length > geometry.previous_interval_width ||
        geometry.current_window_length > geometry.previous_interval_width ||
        !source_slice_fits(previous_pcm, geometry.previous_source_start,
            geometry.previous_window_length) ||
        !source_slice_fits(current_pcm, geometry.current_source_start,
            geometry.current_window_length) ||
        !source_slice_fits(previous_pcm, geometry.previous_forward_source_start,
            previous_extent) ||
        !source_slice_fits(current_pcm, geometry.current_forward_source_start,
            current_extent))
        return out;

    const auto previous_reverse_window =
        legacy_window_m36_lookup(geometry.previous_window_length);
    const auto current_reverse_window =
        legacy_window_m36_lookup(geometry.current_window_length);
    const auto previous_forward_window = legacy_window_m36_lookup(previous_extent);
    const auto current_forward_window = legacy_window_m36_lookup(current_extent);
    if (!previous_reverse_window.valid || !current_reverse_window.valid ||
        !previous_forward_window.valid || !current_forward_window.valid)
        return out;

    const int primary_length = geometry.previous_interval_width;
    const int previous_primary_start =
        primary_length - geometry.previous_window_length;
    const int current_primary_start =
        primary_length - geometry.current_window_length;
    out.primary.assign(static_cast<std::size_t>(primary_length), 0);
    for (int i = std::min(previous_primary_start, current_primary_start);
         i < primary_length; ++i) {
        const bool use_previous = i >= previous_primary_start;
        const bool use_current = i >= current_primary_start;
        if (use_previous && use_current) {
            const int previous_index = i - previous_primary_start;
            const int current_index = i - current_primary_start;
            out.primary[static_cast<std::size_t>(i)] =
                legacy_runtime_cross_overlap_sample_m36(
                    previous_pcm[static_cast<std::size_t>(
                        geometry.previous_source_start + previous_index)],
                    previous_reverse_window.q15[static_cast<std::size_t>(
                        geometry.previous_window_length - 1 - previous_index)],
                    current_pcm[static_cast<std::size_t>(
                        geometry.current_source_start + current_index)],
                    current_reverse_window.q15[static_cast<std::size_t>(
                        geometry.current_window_length - 1 - current_index)]);
        } else if (use_previous) {
            const int index = i - previous_primary_start;
            out.primary[static_cast<std::size_t>(i)] = q15_sample(
                previous_pcm[static_cast<std::size_t>(
                    geometry.previous_source_start + index)],
                previous_reverse_window.q15[static_cast<std::size_t>(
                    geometry.previous_window_length - 1 - index)]);
        } else {
            const int index = i - current_primary_start;
            out.primary[static_cast<std::size_t>(i)] = q15_sample(
                current_pcm[static_cast<std::size_t>(
                    geometry.current_source_start + index)],
                current_reverse_window.q15[static_cast<std::size_t>(
                    geometry.current_window_length - 1 - index)]);
        }
    }

    const int secondary_length = geometry.current_interval_width;
    out.secondary.assign(static_cast<std::size_t>(secondary_length), 0);
    for (int i = 0; i < std::max(previous_extent, current_extent); ++i) {
        const bool use_previous = i < previous_extent;
        const bool use_current = i < current_extent;
        if (use_previous && use_current) {
            out.secondary[static_cast<std::size_t>(i)] =
                legacy_runtime_cross_overlap_sample_m36(
                    previous_pcm[static_cast<std::size_t>(
                        geometry.previous_forward_source_start + i)],
                    previous_forward_window.q15[static_cast<std::size_t>(i)],
                    current_pcm[static_cast<std::size_t>(
                        geometry.current_forward_source_start + i)],
                    current_forward_window.q15[static_cast<std::size_t>(i)]);
        } else if (use_previous) {
            out.secondary[static_cast<std::size_t>(i)] = q15_sample(
                previous_pcm[static_cast<std::size_t>(
                    geometry.previous_forward_source_start + i)],
                previous_forward_window.q15[static_cast<std::size_t>(i)]);
        } else {
            out.secondary[static_cast<std::size_t>(i)] = q15_sample(
                current_pcm[static_cast<std::size_t>(
                    geometry.current_forward_source_start + i)],
                current_forward_window.q15[static_cast<std::size_t>(i)]);
        }
    }

    out.valid = true;
    return out;
}

LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_entry_plan_m36(
    int previous_interval_width,
    int previous_boundary,
    int first_period) {
    LegacyRuntimeCrossWriterPlanM36 out;
    if (previous_interval_width <= 0 || first_period <= 0 || first_period > 32767)
        return out;
    const int length = std::min(previous_interval_width, first_period);
    const int primary_tail = previous_interval_width - length;
    if (!safe_offset(previous_boundary) || primary_tail < 0) return out;

    out.valid = true;
    out.period = first_period;
    out.left_source = LegacyRuntimeCrossSourceM36::PreviousPcm;
    out.left_offset = previous_boundary;
    out.left_window_length = length;
    out.right_source = LegacyRuntimeCrossSourceM36::PrimaryTemp;
    out.right_offset = primary_tail;
    out.right_window_length = length;
    return out;
}

LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_buffered_repeat_plan_m36(
    int previous_interval_width,
    int current_interval_width,
    int first_period,
    int delta_q11,
    int ordinal) {
    LegacyRuntimeCrossWriterPlanM36 out;
    if (previous_interval_width <= 0 || current_interval_width <= 0) return out;
    const int period = repeated_period(first_period, delta_q11, ordinal);
    if (period <= 0) return out;
    const int left_length = std::min(current_interval_width, period);
    const int right_length = std::min(previous_interval_width, period);
    const int primary_tail = previous_interval_width - right_length;
    if (primary_tail < 0) return out;

    out.valid = true;
    out.period = period;
    out.left_source = LegacyRuntimeCrossSourceM36::SecondaryTemp;
    out.left_offset = 0;
    out.left_window_length = left_length;
    out.right_source = LegacyRuntimeCrossSourceM36::PrimaryTemp;
    out.right_offset = primary_tail;
    out.right_window_length = right_length;
    return out;
}

LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_current_entry_plan_m36(
    int current_interval_width,
    int current_boundary,
    int first_period) {
    LegacyRuntimeCrossWriterPlanM36 out;
    if (current_interval_width <= 0 || first_period <= 0 || first_period > 32767)
        return out;
    const int length = std::min(current_interval_width, first_period);
    const std::int64_t current_tail =
        static_cast<std::int64_t>(current_boundary) - length;
    if (!safe_offset(current_tail)) return out;

    out.valid = true;
    out.period = first_period;
    out.left_source = LegacyRuntimeCrossSourceM36::SecondaryTemp;
    out.left_offset = 0;
    out.left_window_length = length;
    out.right_source = LegacyRuntimeCrossSourceM36::CurrentPcm;
    out.right_offset = static_cast<int>(current_tail);
    out.right_window_length = length;
    return out;
}

LegacyRuntimeCrossWriterPlanM36 legacy_runtime_cross_current_repeat_plan_m36(
    int current_interval_width,
    int current_next_width,
    int current_boundary,
    int first_period,
    int delta_q11,
    int ordinal) {
    LegacyRuntimeCrossWriterPlanM36 out;
    if (current_interval_width <= 0 || current_next_width <= 0) return out;
    const int period = repeated_period(first_period, delta_q11, ordinal);
    if (period <= 0) return out;
    const int left_length = std::min(current_next_width, period);
    const int right_length = std::min(current_interval_width, period);
    const std::int64_t current_tail =
        static_cast<std::int64_t>(current_boundary) - right_length;
    if (!safe_offset(current_boundary) || !safe_offset(current_tail)) return out;

    out.valid = true;
    out.period = period;
    out.left_source = LegacyRuntimeCrossSourceM36::CurrentPcm;
    out.left_offset = current_boundary;
    out.left_window_length = left_length;
    out.right_source = LegacyRuntimeCrossSourceM36::CurrentPcm;
    out.right_offset = static_cast<int>(current_tail);
    out.right_window_length = right_length;
    return out;
}

std::int16_t legacy_runtime_cross_overlap_sample_m36(
    std::int16_t left_sample,
    std::int16_t left_window_q15,
    std::int16_t right_sample,
    std::int16_t right_window_q15) {
    const std::int32_t left_product =
        static_cast<std::int32_t>(left_sample) * left_window_q15;
    const std::int32_t right_product =
        static_cast<std::int32_t>(right_sample) * right_window_q15;
    const std::int32_t sum = wrap32(
        static_cast<std::int64_t>(left_product) + right_product);
    return word(sar16(sum));
}

} // namespace nicolai
