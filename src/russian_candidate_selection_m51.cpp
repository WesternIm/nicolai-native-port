#include "nicolai/russian_candidate_selection_m51.hpp"
#include <stdexcept>

namespace nicolai {
void normalize_russian_candidate_forms_m51(std::vector<RussianCandidatePayloadM51>& candidates) {
    if (candidates.size() > 70) throw std::invalid_argument("candidate_capacity_exceeded_m51");
    for (auto& row : candidates) {
        auto& form = row[5];
        switch (row[3]) {
        case 0: form = static_cast<std::uint8_t>(form + 81); break;
        case 2: form = static_cast<std::uint8_t>(form + 8); break;
        case 3: form = static_cast<std::uint8_t>(form + 19); break;
        case 4: form = static_cast<std::uint8_t>(form + 45); break;
        case 5: form = static_cast<std::uint8_t>(form + 49); break;
        case 6: form = static_cast<std::uint8_t>(form + 59); break;
        case 7:
            if (form >= 1 && form <= 26) form += 19;
            else if (form >= 27 && form <= 52) form -= 7;
            else if (form >= 53 && form <= 78) form -= 33;
            else if (form >= 79 && form <= 108) form -= 59;
            else if (form >= 109 && form <= 112) form -= 63;
            else if (form == 113) form -= 40;
            break;
        default: break;
        }
        for (std::size_t i = 8; i < 12; ++i) row[i] = 0;
    }
}

bool russian_candidate_pronunciation_conflict_m51(
    const std::vector<RussianCandidatePayloadM51>& candidates) {
    if (candidates.size() > 70) throw std::invalid_argument("candidate_capacity_exceeded_m51");
    if (candidates.empty()) return false;
    for (std::size_t i = 1; i < candidates.size(); ++i)
        for (std::size_t field = 0; field < 3; ++field)
            if (candidates[i][field] != candidates.front()[field]) return true;
    return false;
}
}
