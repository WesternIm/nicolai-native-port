#include "nicolai/seg_schedule.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <limits>

namespace nicolai {

SegScheduleM15 parse_seg_schedule_m15(
    const DiphoneUnit& unit,
    int min_period,
    int max_period) {

    SegScheduleM15 out;
    const auto& m = unit.metadata;
    if (m.size() < 4) { out.error = "seg_too_short"; return out; }
    if (min_period <= 0 || max_period < min_period) {
        out.error = "invalid_period_range"; return out;
    }

    out.mode = m[0];
    out.total_slots = m[1];
    out.split_index = m[2];
    if (out.mode != 2 && out.mode != -2) { out.error = "unexpected_mode"; return out; }
    if (out.total_slots <= 0) { out.error = "invalid_total_slots"; return out; }
    if (out.split_index < 0 || out.split_index > out.total_slots) {
        out.error = "invalid_split_index"; return out;
    }

    std::size_t i = 3;
    int covered = 0;
    while (covered < out.total_slots) {
        if (i >= m.size()) { out.error = "truncated_run_header"; return out; }
        const int signed_count = m[i++];
        if (signed_count == 0) { out.error = "zero_run"; return out; }
        const int slots = std::abs(signed_count);
        if (slots > out.total_slots - covered) {
            out.error = "run_exceeds_total"; return out;
        }

        SegRun run;
        run.voiced = signed_count > 0;
        run.slots = slots;
        if (run.voiced) {
            if (i + static_cast<std::size_t>(slots) > m.size()) {
                out.error = "truncated_pitch_values"; return out;
            }
            run.signed_periods.reserve(static_cast<std::size_t>(slots));
            run.periods.reserve(static_cast<std::size_t>(slots));
            for (int j = 0; j < slots; ++j) {
                const int v = m[i++];
                const int p = std::abs(v);
                if (p < min_period || p > max_period) {
                    out.error = "pitch_out_of_range"; return out;
                }
                if (v < 0 && j != 0) {
                    out.error = "negative_pitch_not_first"; return out;
                }
                run.signed_periods.push_back(v);
                run.periods.push_back(p);
            }
            if (!run.signed_periods.empty() && run.signed_periods.front() < 0) {
                run.signed_period_reset = true;
                ++out.signed_period_resets;
                // This exact +/- bridge relation is observed for every
                // multi-value signed run in the supplied Nicolai database.
                if (run.periods.size() > 1 &&
                    std::abs(run.periods[0] - run.periods[1]) > 1) {
                    out.error = "signed_pitch_bridge_mismatch"; return out;
                }
            }
            out.voiced_slots += slots;
        } else {
            out.unvoiced_slots += slots;
        }
        out.runs.push_back(std::move(run));
        covered += slots;
    }

    if (covered != out.total_slots) { out.error = "slot_count_mismatch"; return out; }
    if (i != m.size()) { out.error = "trailing_seg_words"; return out; }
    out.valid = true;
    return out;
}

SegSpanLayout layout_seg_runs_m15(
    const SegScheduleM15& schedule,
    std::size_t pcm_sample_count) {

    SegSpanLayout out;
    out.pcm_samples = pcm_sample_count;
    if (!schedule.valid) { out.error = "invalid_schedule"; return out; }
    if (pcm_sample_count == 0) { out.error = "empty_pcm"; return out; }

    std::uint64_t voiced_sum = 0;
    for (const auto& r : schedule.runs) {
        if (!r.voiced) continue;
        for (const int p : r.periods) voiced_sum += static_cast<std::uint64_t>(p);
    }
    if (voiced_sum > pcm_sample_count) {
        out.error = "voiced_periods_exceed_pcm"; return out;
    }
    out.voiced_period_samples = static_cast<std::size_t>(voiced_sum);
    const std::size_t residual = pcm_sample_count - out.voiced_period_samples;

    // All-voiced records have a small amount of edge slack. Keep it symmetric
    // around the complete period chain, matching the strong M14 observation.
    if (schedule.unvoiced_slots == 0) {
        if (schedule.runs.size() != 1 || !schedule.runs.front().voiced) {
            out.error = "all_voiced_unexpected_run_shape"; return out;
        }
        const std::size_t left = residual / 2;
        SegRunSpan s;
        s.voiced = true;
        s.slots = schedule.runs.front().slots;
        s.source_begin = 0;
        s.source_end = pcm_sample_count;
        s.signed_periods = schedule.runs.front().signed_periods;
        s.periods = schedule.runs.front().periods;
        s.signed_period_reset = schedule.runs.front().signed_period_reset;
        out.runs.push_back(std::move(s));
        out.split_sample_estimate = left;
        // Estimate the phone split using the period magnitudes before the
        // split slot. This is diagnostic/prosody scaffolding, not yet a legacy
        // duration regulator clone.
        const int n = std::min(schedule.split_index, schedule.runs.front().slots);
        for (int k = 0; k < n; ++k)
            out.split_sample_estimate += static_cast<std::size_t>(schedule.runs.front().periods[k]);
        out.valid = true;
        return out;
    }

    out.unvoiced_budget_samples = residual;
    std::size_t cursor = 0;
    int slot_cursor = 0;
    std::size_t unvoiced_assigned = 0;
    int unvoiced_slots_seen = 0;

    for (std::size_t ri = 0; ri < schedule.runs.size(); ++ri) {
        const auto& r = schedule.runs[ri];
        SegRunSpan s;
        s.voiced = r.voiced;
        s.slots = r.slots;
        s.source_begin = cursor;
        s.signed_periods = r.signed_periods;
        s.periods = r.periods;
        s.signed_period_reset = r.signed_period_reset;

        std::size_t len = 0;
        if (r.voiced) {
            for (const int p : r.periods) len += static_cast<std::size_t>(p);
        } else {
            // Cumulative proportional allocation prevents rounding drift and
            // guarantees that all unvoiced runs together consume residual.
            const int next_slots = unvoiced_slots_seen + r.slots;
            const auto cumulative = static_cast<std::size_t>(
                (static_cast<std::uint64_t>(residual) * next_slots) /
                static_cast<std::uint64_t>(schedule.unvoiced_slots));
            len = cumulative - unvoiced_assigned;
            unvoiced_assigned = cumulative;
            unvoiced_slots_seen = next_slots;
        }

        if (ri + 1 == schedule.runs.size()) {
            // Absorb any final integer-rounding residue into the last run.
            len = pcm_sample_count - cursor;
        }
        if (cursor + len > pcm_sample_count) {
            out.error = "run_span_out_of_bounds"; return out;
        }
        s.source_end = cursor + len;

        // Estimate where the diphone's phone transition falls inside the PCM.
        if (schedule.split_index >= slot_cursor &&
            schedule.split_index <= slot_cursor + r.slots) {
            const int local_slots = schedule.split_index - slot_cursor;
            if (r.voiced) {
                std::size_t x = s.source_begin;
                for (int k = 0; k < local_slots && k < static_cast<int>(r.periods.size()); ++k)
                    x += static_cast<std::size_t>(r.periods[k]);
                out.split_sample_estimate = std::min(x, s.source_end);
            } else if (r.slots > 0) {
                out.split_sample_estimate = s.source_begin +
                    (len * static_cast<std::size_t>(local_slots)) /
                    static_cast<std::size_t>(r.slots);
            }
        }

        out.runs.push_back(std::move(s));
        cursor += len;
        slot_cursor += r.slots;
    }

    if (cursor != pcm_sample_count) { out.error = "span_coverage_mismatch"; return out; }
    out.valid = true;
    return out;
}

} // namespace nicolai
