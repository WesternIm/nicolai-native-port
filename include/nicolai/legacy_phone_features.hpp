#pragma once

namespace nicolai {

struct LegacyPhoneDurationM36 {
    bool valid = false;
    int previous_right_support = 0;
    int next_left_support = 0;
    int combined_source_support = 0;
    int duration_q11 = 0;
};

// Proven positive-domain slice of original 0x101a2780.
// Source support spans previous-right + next-left:
//   (previous.last - previous.split) + (next.split - next.first)
// Duration is total feature duration in source-sample units, scaled to Q11.
// A mathematically zero positive result falls back to 1, matching the traced
// builder. Invalid geometry/inputs return valid=false instead of emulating
// original divide faults or unproven overflow/wrap behavior.
LegacyPhoneDurationM36 legacy_phone_duration_m36(
    int previous_split,
    int previous_last,
    int next_first,
    int next_split,
    int total_feature_duration);

} // namespace nicolai
