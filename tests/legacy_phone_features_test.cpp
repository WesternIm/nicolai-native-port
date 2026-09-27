#include "nicolai/legacy_phone_features.hpp"

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

namespace {
nicolai::LegacyPhoneDescriptorM36 descriptor(
    std::vector<int> positions,
    int split,
    std::vector<std::int16_t> voicing) {
    nicolai::LegacyPhoneDescriptorM36 d;
    d.source_position = std::move(positions);
    d.split_index = split;
    d.voicing = std::move(voicing);
    d.duration_q11.assign(d.source_position.size() - 1, 111);
    d.pitch_q11.assign(d.source_position.size() - 1, 222);
    return d;
}
} // namespace

int main() {
    // Scalar duration helper retained as an independently testable primitive.
    {
        const auto r = nicolai::legacy_phone_duration_m36(80, 120, 200, 260, 100);
        assert(r.valid);
        assert(r.previous_right_support == 40);
        assert(r.next_left_support == 60);
        assert(r.combined_source_support == 100);
        assert(r.duration_q11 == 2048);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 40, 100, 160, 50);
        assert(r.valid);
        assert(r.combined_source_support == 100);
        assert(r.duration_q11 == 1024);
    }
    {
        const auto r = nicolai::legacy_phone_duration_m36(0, 2048, 0, 2048, 1);
        assert(r.valid);
        assert(r.combined_source_support == 4096);
        assert(r.duration_q11 == 1);
    }
    assert(!nicolai::legacy_phone_duration_m36(10, 9, 0, 10, 20).valid);
    assert(!nicolai::legacy_phone_duration_m36(0, 10, 20, 10, 20).valid);
    assert(!nicolai::legacy_phone_duration_m36(0, 0, 0, 0, 20).valid);
    assert(!nicolai::legacy_phone_duration_m36(0, 10, 0, 10, 0).valid);
    assert(!nicolai::legacy_phone_duration_m36(0, 1, 0, 1, INT_MAX).valid);

    // count <= 0 branch writes unity only over previous-right and next-left.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f;
        auto previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
        auto next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
        const auto r = nicolai::legacy_phone_features_m36(f, previous, next);
        assert(r.valid);
        assert(r.previous_intervals_written == 2);
        assert(r.next_intervals_written == 2);
        assert(previous.duration_q11 == std::vector<std::int16_t>({111, 2048, 2048}));
        assert(previous.pitch_q11 == std::vector<std::int16_t>({222, 2048, 2048}));
        assert(next.duration_q11 == std::vector<std::int16_t>({2048, 2048, 111}));
        assert(next.pitch_q11 == std::vector<std::int16_t>({2048, 2048, 222}));
    }

    // Integer reciprocal truncation is visible: constant 80-sample anchors
    // produce 2047 rather than idealized 2048.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f{{160, 160}, {80, 80, 80}};
        auto previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
        auto next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
        const auto r = nicolai::legacy_phone_features_m36(f, previous, next);
        assert(r.valid);
        assert(r.duration_q11 == 2048);
        assert(previous.duration_q11 == std::vector<std::int16_t>({111, 2048, 2048}));
        assert(previous.pitch_q11 == std::vector<std::int16_t>({222, 2047, 2047}));
        assert(next.duration_q11 == std::vector<std::int16_t>({2048, 2048, 111}));
        assert(next.pitch_q11 == std::vector<std::int16_t>({2047, 2047, 222}));
    }

    // Missing anchor is repaired in-place before pitch interpolation.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f{{80, 80}, {80, 0, 120}};
        auto previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
        auto next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
        const auto r = nicolai::legacy_phone_features_m36(f, previous, next);
        assert(r.valid);
        assert(r.duration_q11 == 1024);
        assert(f.pitch_anchor == std::vector<std::int16_t>({80, 80, 120}));
        assert(previous.pitch_q11 == std::vector<std::int16_t>({222, 2047, 2047}));
        assert(next.pitch_q11 == std::vector<std::int16_t>({1706, 1706, 222}));
    }

    // Descriptor voicing zero forces pitch unity without altering duration.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f{{160, 160}, {100, 100, 100}};
        auto previous = descriptor({0, 80, 160, 240}, 1, {1, 0, 1});
        auto next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
        const auto r = nicolai::legacy_phone_features_m36(f, previous, next);
        assert(r.valid);
        assert(previous.pitch_q11[1] == 2048);
        assert(previous.pitch_q11[2] == 1638);
        assert(next.pitch_q11[0] == 1638);
    }

    // Unequal source widths exercise feature-boundary carry and terminal path.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f{{100, 140, 120}, {83, 0, 120, 166}};
        auto previous = descriptor({0, 73, 162, 271, 390}, 2, {1, 1, 1, 1});
        auto next = descriptor({700, 791, 905, 1030, 1160}, 2, {1, 0, 1, 1});
        const auto r = nicolai::legacy_phone_features_m36(f, previous, next);
        assert(r.valid);
        assert(r.duration_q11 == 1702);
        assert(f.pitch_anchor == std::vector<std::int16_t>({83, 83, 120, 166}));
        assert(previous.duration_q11 == std::vector<std::int16_t>({111, 111, 1702, 1702}));
        assert(previous.pitch_q11 == std::vector<std::int16_t>({222, 222, 2689, 2354}));
        assert(next.duration_q11 == std::vector<std::int16_t>({1702, 1702, 111, 111}));
        assert(next.pitch_q11 == std::vector<std::int16_t>({1460, 2048, 222, 222}));
    }

    // Guarded portable domain rejects malformed records instead of inventing
    // original divide-fault / out-of-bounds semantics.
    {
        nicolai::LegacyPhoneFeatureRecordM36 f{{100}, {80, 80, 80}};
        auto previous = descriptor({0, 80, 160}, 1, {1, 1});
        auto next = descriptor({1000, 1080, 1160}, 1, {1, 1});
        assert(!nicolai::legacy_phone_features_m36(f, previous, next).valid);
    }

    std::cout << "legacy_phone_features_test: PASSED\n";
}
