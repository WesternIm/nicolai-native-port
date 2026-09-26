#pragma once

#include "nicolai/diphone_catalog.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace nicolai {

// M15 structural interpretation of Nicolai SEG metadata.
//
// Every one of the 2665 supplied diphones follows this grammar:
//   mode, total_slots, split_index,
//   run, [pitch...], run, [pitch...] ...
//
// run > 0 : voiced run; exactly run signed pitch values follow.
// run < 0 : unvoiced run; no pitch values follow.
// Sum(abs(run)) == total_slots.
//
// A negative pitch value is only observed as the first pitch value of a voiced
// run. When another value follows, abs(first) differs from second by at most
// one sample. M15 preserves the sign as a transition/phase-reset marker and
// uses abs(value) as the period magnitude. The exact legacy semantic name of
// the sign bit is intentionally not claimed yet.
struct SegRun {
    bool voiced = false;
    int slots = 0;
    std::vector<int> signed_periods;
    std::vector<int> periods;
    bool signed_period_reset = false;
};

struct SegScheduleM15 {
    bool valid = false;
    std::string error;
    int mode = 0;          // observed +2 / -2
    int total_slots = 0;
    int split_index = 0;
    int voiced_slots = 0;
    int unvoiced_slots = 0;
    int signed_period_resets = 0;
    std::vector<SegRun> runs;
};

SegScheduleM15 parse_seg_schedule_m15(
    const DiphoneUnit& unit,
    int min_period = 20,
    int max_period = 500);

struct SegRunSpan {
    bool voiced = false;
    int slots = 0;
    std::size_t source_begin = 0;
    std::size_t source_end = 0; // exclusive
    std::vector<int> signed_periods;
    std::vector<int> periods;
    bool signed_period_reset = false;
    // Experimental M33 adapter supplies exact PC node offsets instead of
    // M15's symmetric-margin estimate. Empty is meaningful when explicitly set.
    bool use_explicit_marks = false;
    std::vector<std::size_t> source_marks;
};

struct SegSpanLayout {
    bool valid = false;
    std::string error;
    std::size_t pcm_samples = 0;
    std::size_t voiced_period_samples = 0;
    std::size_t unvoiced_budget_samples = 0;
    std::size_t split_sample_estimate = 0;
    std::vector<SegRunSpan> runs;
};

// Convert SEG run counts into a complete, contiguous source-sample partition.
// Voiced run lengths come from the sum of their recorded period magnitudes.
// Remaining samples are distributed proportionally across unvoiced slots.
// Fully voiced records keep their legacy edge slack as symmetric margins.
SegSpanLayout layout_seg_runs_m15(
    const SegScheduleM15& schedule,
    std::size_t pcm_sample_count);

struct SegSourceNodeM33 {
    std::size_t sample = 0;
    bool voiced = false;
};

struct SegSourceTimelineM33 {
    bool valid = false;
    std::string error;
    std::vector<SegSourceNodeM33> nodes; // 0, N slot ends, PCM last sample
    std::size_t split_node = 0;         // SEG split + 1
    std::size_t unvoiced_slot_samples = 0;
};

// Proven PCM/A-law branch of mtsyc32.dll:0x101a30b0. Negative period signs
// set the corresponding node's voicing flag to zero; they are not discarded.
SegSourceTimelineM33 source_timeline_seg_m33(
    const SegScheduleM15& schedule, std::size_t pcm_sample_count,
    bool terminal_voiced, int sample_rate = 16000);

// Adapter to the portable run renderer, NOT the complete Windows TDS join.
// Exact node offsets are retained, but run slicing/OLA remain approximations.
SegSpanLayout layout_seg_runs_m33(
    const SegScheduleM15& schedule, std::size_t pcm_sample_count,
    bool terminal_voiced, int sample_rate = 16000);

} // namespace nicolai
