#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace nicolai {
// Actual 20-byte payload at word_block + 20*candidate_index + 4 (one based).
// The legacy M46 20-byte window begins four bytes earlier and is NOT this type.
using RussianCandidatePayloadM51 = std::array<std::uint8_t, 20>;

// Isolated equivalent of pinned 0x10217830. This does not rank candidates,
// choose stress, or implement the surrounding morphology/context pipeline.
bool russian_candidate_pronunciation_conflict_m51(
    const std::vector<RussianCandidatePayloadM51>& candidates);

// Candidate-only lane of 0x101a1830: normalize form byte and clear score.
// Does not implement that function's punctuation cleanup or context scoring.
void normalize_russian_candidate_forms_m51(std::vector<RussianCandidatePayloadM51>& candidates);
}
