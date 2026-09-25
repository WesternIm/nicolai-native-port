#pragma once

#include "nicolai/engine.hpp"
#include "nicolai/psola_join.hpp"
#include "nicolai/seg_schedule.hpp"
#include "nicolai/td_psola.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct M15RunDiagnostics {
    bool voiced = false;
    bool used_td_psola = false;
    bool used_unvoiced_stretch = false;
    bool signed_period_reset = false;
    int slots = 0;
    std::size_t source_begin = 0;
    std::size_t source_end = 0;
    std::size_t output_samples = 0;
    std::vector<int> periods;
    TdPsolaDiagnostics psola;
};

struct M15UnitDiagnostics {
    std::string label;
    SegScheduleM15 schedule;
    SegSpanLayout layout;
    std::vector<M15RunDiagnostics> runs;
    std::vector<OlaJoinDiagnostics> internal_joins;
    int left_period_hint = 0;
    int right_period_hint = 0;
};

struct DiphoneChainM15Result {
    bool valid = false;
    std::string error;
    Pcm16Mono pcm;
    std::vector<std::string> diphone_labels;
    std::vector<M15UnitDiagnostics> units;
    std::vector<OlaJoinDiagnostics> joins;
};

// Duration-only short-time Hann OLA used for unvoiced/noise runs. Frame
// contents are not resampled, so duration changes do not impose the spectral
// pitch shift produced by naive linear resampling. Pitch scaling is never
// applied to these runs. At duration_scale==1 this is byte-for-byte PCM copy.
Pcm16Mono stretch_unvoiced_ola(
    const Pcm16Mono& source,
    double duration_scale);

// M15 renderer: parses the complete signed SEG run grammar, applies TD-PSOLA
// independently to each voiced run, duration-stretches unvoiced runs, and
// rejoins runs with raised-cosine OLA. This removes M14's whole-unit raw
// fallback for mixed voiced/unvoiced diphones.
Pcm16Mono resynthesize_seg_m15(
    const Pcm16Mono& source,
    const SegScheduleM15& schedule,
    const SegSpanLayout& layout,
    const TdPsolaConfig& config,
    M15UnitDiagnostics* diagnostics = nullptr);

// M23: position-dependent duration mapping across the recovered SEG phone
// transition.  left_duration_scale applies to source samples before
// layout.split_sample_estimate and right_duration_scale after it.  Voiced
// runs use an invertible piecewise-linear time map inside TD-PSOLA; unvoiced
// runs use the same map for their Hann-OLA synthesis positions.  Equal scales
// dispatch to M15 exactly, preserving the proven baseline bit-for-bit.
Pcm16Mono resynthesize_seg_m23_phone_sides(
    const Pcm16Mono& source,
    const SegScheduleM15& schedule,
    const SegSpanLayout& layout,
    double pitch_scale,
    double left_duration_scale,
    double right_duration_scale,
    M15UnitDiagnostics* diagnostics = nullptr);

DiphoneChainM15Result synthesize_diphone_chain_m15(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    const TdPsolaConfig& config = {},
    int sample_rate = 16000);

} // namespace nicolai
