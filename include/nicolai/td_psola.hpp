#pragma once

#include "nicolai/diphone_catalog.hpp"
#include "nicolai/engine.hpp"
#include "nicolai/psola_join.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

// Clean voiced SEG layout observed in Nicolai:
//   [2, period_count, split_index, period_count, p0, p1, ...]
// where each pi is a pitch period in samples at 16 kHz. Mixed/unvoiced
// records use additional signed control tokens and are intentionally rejected
// here until that grammar is recovered.
struct SegPitchSchedule {
    bool valid = false;
    std::string error;
    int mode = 0;
    int declared_periods = 0;
    int split_index = 0;
    int repeated_period_count = 0;
    std::vector<int> periods;
    std::vector<std::size_t> marks;
    std::size_t left_margin = 0;
    std::size_t right_margin = 0;
};

SegPitchSchedule parse_clean_voiced_seg_schedule(
    const DiphoneUnit& unit,
    std::size_t pcm_sample_count,
    int min_period = 40,
    int max_period = 400);

// Exact mathematical shape recovered from the legacy Hanning builder:
// 0.5 * (1 + cos(pi*x/period)) * 32767, x=0..period.
// This is the descending Q15 half-window used as a building block by the old
// TDS code. M14 also uses a symmetric Hann grain derived from the same cosine.
std::vector<std::int16_t> legacy_half_hann_q15(std::size_t period);

struct TdPsolaConfig {
    double pitch_scale = 1.0;    // >1 raises pitch; legacy constant fallback
    double duration_scale = 1.0; // >1 lengthens the unit

    // M24: the PC prosody record carries three pitch values per phonetic
    // element (+0x190/+0x194/+0x198).  When enabled, portable TD-PSOLA
    // linearly interpolates a three-point pitch contour instead of forcing one
    // pitch ratio over the complete voiced span. Values are absolute ratios
    // relative to the source recording, just like pitch_scale.
    bool use_three_point_pitch = false;
    double pitch_scale_start = 1.0;
    double pitch_scale_mid = 1.0;
    double pitch_scale_end = 1.0;
    // M33 ablation: disable M13's sample-correlation phase trims, without
    // claiming to reproduce the PC's synthesis-mark-aligned join.
    bool search_join_phase = true;
};

// Evaluate the M24 three-point contour at normalized source position [0,1].
// With use_three_point_pitch=false this is exactly config.pitch_scale.
double td_psola_pitch_scale_at(const TdPsolaConfig& config, double position);

struct TdPsolaDiagnostics {
    bool valid = false;
    std::size_t source_marks = 0;
    std::size_t synthesis_marks = 0;
    std::size_t grains_added = 0;
    std::size_t output_samples = 0;
    double pitch_scale = 1.0;
    double duration_scale = 1.0;
    double mean_source_period = 0.0;
    double mean_target_period = 0.0;
};

Pcm16Mono td_psola_resynthesize(
    const Pcm16Mono& source,
    const SegPitchSchedule& schedule,
    const TdPsolaConfig& config,
    TdPsolaDiagnostics* diagnostics = nullptr);

struct M14UnitDiagnostics {
    std::string label;
    bool used_td_psola = false;
    SegPitchSchedule schedule;
    TdPsolaDiagnostics psola;
};

struct DiphoneChainM14Result {
    bool valid = false;
    std::string error;
    Pcm16Mono pcm;
    std::vector<std::string> diphone_labels;
    std::vector<M14UnitDiagnostics> units;
    std::vector<OlaJoinDiagnostics> joins;
};

// M14 performs real pitch-synchronous resynthesis for clean voiced units using
// SEG-derived pitch periods and Hann-windowed grains. Mixed/unvoiced units are
// preserved as their decoded A-law PCM until the signed SEG control grammar is
// recovered. Unit boundaries are then joined with the proven M13 phase-aligned
// OLA stage.
DiphoneChainM14Result synthesize_diphone_chain_m14(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    const TdPsolaConfig& config = {},
    int sample_rate = 16000);

} // namespace nicolai
