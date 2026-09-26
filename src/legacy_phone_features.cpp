#include "nicolai/legacy_phone_features.hpp"

#include <cstdint>
#include <limits>

namespace nicolai {

LegacyPhoneDurationM36 legacy_phone_duration_m36(
    int previous_split,
    int previous_last,
    int next_first,
    int next_split,
    int total_feature_duration) {
    LegacyPhoneDurationM36 out;
    if (previous_last < previous_split || next_split < next_first ||
        total_feature_duration <= 0) {
        return out;
    }

    const std::int64_t previous_right =
        static_cast<std::int64_t>(previous_last) - previous_split;
    const std::int64_t next_left =
        static_cast<std::int64_t>(next_split) - next_first;
    const std::int64_t support = previous_right + next_left;
    if (support <= 0 || support > std::numeric_limits<int>::max()) {
        return out;
    }

    const std::int64_t scaled =
        static_cast<std::int64_t>(total_feature_duration) * 2048;
    if (scaled <= 0) {
        return out;
    }

    std::int64_t q11 = scaled / support;
    if (q11 == 0) q11 = 1;
    if (q11 > std::numeric_limits<int>::max()) {
        return out;
    }

    out.valid = true;
    out.previous_right_support = static_cast<int>(previous_right);
    out.next_left_support = static_cast<int>(next_left);
    out.combined_source_support = static_cast<int>(support);
    out.duration_q11 = static_cast<int>(q11);
    return out;
}

} // namespace nicolai
