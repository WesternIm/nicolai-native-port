#include "nicolai/legacy_phone_features.hpp"

#include <cstdint>
#include <limits>

namespace nicolai {
namespace {
std::int16_t word32(std::int64_t value) {
    const auto u = static_cast<std::uint16_t>(static_cast<std::uint32_t>(value));
    return static_cast<std::int16_t>(
        u <= 32767 ? static_cast<int>(u) : static_cast<int>(u) - 65536);
}

std::int32_t wrap32(std::int64_t value) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value));
}

bool descriptor_valid(const LegacyPhoneDescriptorM36& d) {
    if (d.source_position.size() < 2 || d.split_index < 0 ||
        d.split_index >= static_cast<int>(d.source_position.size())) return false;
    const auto intervals = d.source_position.size() - 1;
    if (d.voicing.size() != intervals || d.duration_q11.size() != intervals ||
        d.pitch_q11.size() != intervals) return false;
    for (std::size_t i = 1; i < d.source_position.size(); ++i)
        if (d.source_position[i] <= d.source_position[i - 1]) return false;
    return true;
}

void repair_anchor_pair(LegacyPhoneFeatureRecordM36& f, int i) {
    auto& left = f.pitch_anchor[static_cast<std::size_t>(i)];
    auto& right = f.pitch_anchor[static_cast<std::size_t>(i + 1)];
    if (left == 0 && right != 0) left = right;
    if (left != 0 && right == 0) right = left;
}

int scaled_feature_width(std::int16_t duration, int duration_q11) {
    const auto shifted = wrap32(static_cast<std::int64_t>(duration) * 2048);
    return shifted / duration_q11;
}

// Same arithmetic as 0x101a2c00, but permits the caller positions observed in
// 0x101a2780 (including the +3 interval-boundary tolerance and terminal case).
int reciprocal_pitch_raw(int start, int length, int end, int position) {
    if (start <= 0 || end <= 0 || length == 0) return 0;
    const std::int32_t a = 0x10000000 / start;
    const std::int32_t b = 0x10000000 / end;
    const std::int32_t slope = (b - a) / length;
    return wrap32(static_cast<std::int64_t>(a) +
        static_cast<std::int64_t>(slope) * position);
}

std::int16_t pitch_q11_for_interval(int width, int left_anchor, int right_anchor,
    int feature_width, int position) {
    const auto reciprocal =
        reciprocal_pitch_raw(left_anchor, feature_width, right_anchor, position);
    // Original uses 32-bit IMUL and logical SHR 17 before storing WORD.
    const auto product = wrap32(static_cast<std::int64_t>(reciprocal) * width);
    const auto shifted = static_cast<std::uint32_t>(product) >> 17;
    return word32(shifted);
}
} // namespace

LegacyPhoneDurationM36 legacy_phone_duration_m36(
    int previous_split,
    int previous_last,
    int next_first,
    int next_split,
    int total_feature_duration) {
    LegacyPhoneDurationM36 out;
    if (previous_last < previous_split || next_split < next_first ||
        total_feature_duration <= 0) return out;

    const std::int64_t previous_right =
        static_cast<std::int64_t>(previous_last) - previous_split;
    const std::int64_t next_left =
        static_cast<std::int64_t>(next_split) - next_first;
    const std::int64_t support = previous_right + next_left;
    if (support <= 0 || support > std::numeric_limits<int>::max()) return out;

    const std::int64_t scaled =
        static_cast<std::int64_t>(total_feature_duration) * 2048;
    if (scaled <= 0) return out;
    std::int64_t q11 = scaled / support;
    if (q11 == 0) q11 = 1;
    if (q11 > std::numeric_limits<int>::max()) return out;

    out.valid = true;
    out.previous_right_support = static_cast<int>(previous_right);
    out.next_left_support = static_cast<int>(next_left);
    out.combined_source_support = static_cast<int>(support);
    out.duration_q11 = static_cast<int>(q11);
    return out;
}

LegacyPhoneFeatureBuildM36 legacy_phone_features_m36(
    LegacyPhoneFeatureRecordM36& features,
    LegacyPhoneDescriptorM36& previous,
    LegacyPhoneDescriptorM36& next) {
    LegacyPhoneFeatureBuildM36 out;
    if (!descriptor_valid(previous) || !descriptor_valid(next)) return out;
    const int previous_intervals =
        static_cast<int>(previous.source_position.size()) - 1;

    // count <= 0 branch: original writes unity into duration and pitch lanes,
    // but does not rewrite the voicing flags.
    if (features.pitch_anchor.empty() && features.interval_duration.empty()) {
        for (int j = previous.split_index; j < previous_intervals; ++j) {
            previous.duration_q11[static_cast<std::size_t>(j)] = 2048;
            previous.pitch_q11[static_cast<std::size_t>(j)] = 2048;
            ++out.previous_intervals_written;
        }
        for (int j = 0; j < next.split_index; ++j) {
            next.duration_q11[static_cast<std::size_t>(j)] = 2048;
            next.pitch_q11[static_cast<std::size_t>(j)] = 2048;
            ++out.next_intervals_written;
        }
        out.valid = true;
        out.duration_q11 = 2048;
        return out;
    }

    if (features.pitch_anchor.size() < 2 ||
        features.interval_duration.size() + 1 != features.pitch_anchor.size())
        return out;
    for (auto d : features.interval_duration) if (d <= 0) return out;
    for (auto p : features.pitch_anchor) if (p < 0) return out;

    std::int64_t total = 0;
    for (auto d : features.interval_duration) total += d;
    if (total <= 0 || total > std::numeric_limits<int>::max()) return out;

    const auto duration = legacy_phone_duration_m36(
        previous.source_position[static_cast<std::size_t>(previous.split_index)],
        previous.source_position.back(), next.source_position.front(),
        next.source_position[static_cast<std::size_t>(next.split_index)],
        static_cast<int>(total));
    if (!duration.valid) return out;

    const int duration_q11 = duration.duration_q11;
    const int feature_intervals =
        static_cast<int>(features.interval_duration.size());
    out.feature_intervals = feature_intervals;
    out.combined_source_support = duration.combined_source_support;
    out.duration_q11 = duration_q11;

    int feature_index = 0;
    int coordinate = 0;
    int feature_width =
        scaled_feature_width(features.interval_duration[0], duration_q11);
    if (feature_width <= 0) return {};
    int last_source_width = 0;
    int j = previous.split_index;

    // First half: previous descriptor from split through the last interval.
    while (feature_index < feature_intervals && j < previous_intervals) {
        feature_width = scaled_feature_width(
            features.interval_duration[static_cast<std::size_t>(feature_index)],
            duration_q11);
        if (feature_width <= 0) return {};
        repair_anchor_pair(features, feature_index);
        const int left_anchor =
            features.pitch_anchor[static_cast<std::size_t>(feature_index)];
        const int right_anchor =
            features.pitch_anchor[static_cast<std::size_t>(feature_index + 1)];

        while (j < previous_intervals) {
            if (coordinate > feature_width) {
                ++feature_index;
                feature_width += last_source_width;
                coordinate -= feature_width;
                break;
            }

            last_source_width =
                previous.source_position[static_cast<std::size_t>(j + 1)] -
                previous.source_position[static_cast<std::size_t>(j)];
            coordinate += last_source_width;
            if (coordinate > feature_width + 3) continue;

            previous.duration_q11[static_cast<std::size_t>(j)] =
                word32(duration_q11);
            if ((left_anchor == 0 && right_anchor == 0) ||
                previous.voicing[static_cast<std::size_t>(j)] == 0) {
                previous.pitch_q11[static_cast<std::size_t>(j)] = 2048;
            } else {
                previous.pitch_q11[static_cast<std::size_t>(j)] =
                    pitch_q11_for_interval(last_source_width, left_anchor,
                        right_anchor, feature_width, coordinate);
            }
            ++out.previous_intervals_written;
            ++j;
        }
    }

    if (feature_index >= feature_intervals || next.split_index <= 0) {
        out.valid = true;
        return out;
    }

    // Second half: next descriptor from interval zero to split-1. The original
    // carries feature coordinate state across the descriptor boundary.
    repair_anchor_pair(features, feature_index);
    int next_index = 0;
    while (feature_index < feature_intervals && next_index < next.split_index) {
        feature_width = scaled_feature_width(
            features.interval_duration[static_cast<std::size_t>(feature_index)],
            duration_q11);
        if (feature_width <= 0) return {};
        repair_anchor_pair(features, feature_index);
        const int left_anchor =
            features.pitch_anchor[static_cast<std::size_t>(feature_index)];
        const int right_anchor =
            features.pitch_anchor[static_cast<std::size_t>(feature_index + 1)];

        while (next_index < next.split_index) {
            if (coordinate > feature_width) {
                ++feature_index;
                feature_width += last_source_width;
                coordinate -= feature_width;
                break;
            }

            last_source_width =
                next.source_position[static_cast<std::size_t>(next_index + 1)] -
                next.source_position[static_cast<std::size_t>(next_index)];
            coordinate += last_source_width;
            if (coordinate > feature_width + 3) continue;

            next.duration_q11[static_cast<std::size_t>(next_index)] =
                word32(duration_q11);
            if ((left_anchor == 0 && right_anchor == 0) ||
                next.voicing[static_cast<std::size_t>(next_index)] == 0) {
                next.pitch_q11[static_cast<std::size_t>(next_index)] = 2048;
            } else {
                int position = coordinate;
                // Exact special case at 0x101a2af1..0x101a2b31: the final
                // next-left interval on the final feature uses width-relative
                // position instead of the running coordinate.
                if (next_index == next.split_index - 1 &&
                    feature_index == feature_intervals - 1)
                    position = feature_width - last_source_width;
                next.pitch_q11[static_cast<std::size_t>(next_index)] =
                    pitch_q11_for_interval(last_source_width, left_anchor,
                        right_anchor, feature_width, position);
            }
            ++out.next_intervals_written;
            ++next_index;
        }
    }

    out.valid = true;
    return out;
}

} // namespace nicolai
