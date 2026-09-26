#pragma once

#include "nicolai/diphone_catalog.hpp"
#include "nicolai/engine.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

// M13 deliberately calls these values pitch *hints*.  Positive SEG metadata
// values in the speech-scale range behave like pitch periods, but the complete
// legacy SEG control language (especially negative tokens) is not yet decoded.
struct PitchPeriodHint {
    bool valid = false;
    std::vector<int> periods;
    int left_period = 0;
    int right_period = 0;
};

PitchPeriodHint infer_pitch_period_hint(const DiphoneUnit& unit,
                                        int min_period = 40,
                                        int max_period = 400);

struct OlaJoinDiagnostics {
    bool valid = false;
    std::size_t overlap_samples = 0;
    std::size_t left_trim = 0;
    std::size_t right_trim = 0;
    double normalized_correlation = 0.0;
    int left_period_hint = 0;
    int right_period_hint = 0;
};

// Pitch-guided, raised-cosine overlap/add boundary join. This is the first
// portable PSOLA-like stage; it is NOT yet a bit-exact clone of legacy TDS.
Pcm16Mono hann_ola_join(const Pcm16Mono& left,
                        const Pcm16Mono& right,
                        int left_period_hint,
                        int right_period_hint,
                        OlaJoinDiagnostics* diagnostics = nullptr,
                        bool search_phase = true);

struct DiphoneChainResult {
    bool valid = false;
    std::string error;
    Pcm16Mono pcm;
    std::vector<std::string> diphone_labels;
    std::vector<OlaJoinDiagnostics> joins;
};

// Synthesize a raw phone chain from database diphones using A-law decode plus
// M13 pitch-guided Hann OLA joins. Example phones: {"#","m","a0","m","a0","#"}.
DiphoneChainResult synthesize_diphone_chain_m13(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    int sample_rate = 16000);

} // namespace nicolai
