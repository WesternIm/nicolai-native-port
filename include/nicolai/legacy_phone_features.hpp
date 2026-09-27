#pragma once

#include <cstdint>
#include <vector>

namespace nicolai {

struct LegacyPhoneDurationM36 {
    bool valid = false;
    int previous_right_support = 0;
    int next_left_support = 0;
    int combined_source_support = 0;
    int duration_q11 = 0;
};

// Proven positive-domain duration slice of original 0x101a2780.
LegacyPhoneDurationM36 legacy_phone_duration_m36(
    int previous_split,
    int previous_last,
    int next_first,
    int next_split,
    int total_feature_duration);

struct LegacyPhoneFeatureRecordM36 {
    // Original record layout recovered statically from 0x101a2780 / caller:
    // count at +0, count-1 signed WORD interval durations from +4, and count
    // signed WORD pitch anchors from +0x18.
    std::vector<std::int16_t> interval_duration;
    std::vector<std::int16_t> pitch_anchor;
};

struct LegacyPhoneDescriptorM36 {
    // Original descriptor fields used by 0x101a2780:
    // node count +0x0c, split +0x10, positions +0x14, voicing +0xfb4,
    // duration Q11 +0x1784, pitch Q11 +0x1f54.
    std::vector<int> source_position;
    int split_index = 0;
    std::vector<std::int16_t> voicing;
    std::vector<std::int16_t> duration_q11;
    std::vector<std::int16_t> pitch_q11;
};

struct LegacyPhoneFeatureBuildM36 {
    bool valid = false;
    int feature_intervals = 0;
    int previous_intervals_written = 0;
    int next_intervals_written = 0;
    int combined_source_support = 0;
    int duration_q11 = 0;
};

// Static reconstruction of original 0x101a2780 on a guarded positive domain.
// It mutates zero pitch anchors like the original and writes only the
// previous-right / next-left coefficient lanes. Production synthesis does not
// call this yet; the local Win32 original-DLL oracle remains the promotion gate.
LegacyPhoneFeatureBuildM36 legacy_phone_features_m36(
    LegacyPhoneFeatureRecordM36& features,
    LegacyPhoneDescriptorM36& previous,
    LegacyPhoneDescriptorM36& next);

} // namespace nicolai
