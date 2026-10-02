#include "nicolai/russian_candidate_selection_m51.hpp"
#include <iostream>
#include <stdexcept>

int main() try {
    using namespace nicolai;
    std::size_t checks = 0;
    const unsigned offsets[]{81,0,8,19,45,49,59};
    const unsigned special_input[]{0,1,26,27,52,53,78,79,108,109,112,113,114,255};
    const unsigned special_output[]{0,20,45,20,45,20,45,20,49,46,49,73,114,255};
    for (unsigned kind = 0; kind < 256; ++kind)
        for (unsigned form = 0; form < 256; ++form) {
            RussianCandidatePayloadM51 row{};
            for (unsigned field = 0; field < 20; ++field) row[field] = (field * 11) & 255;
            row[3] = kind; row[5] = form;
            auto expected = row;
            if (kind < 7) expected[5] = (form + offsets[kind]) & 255;
            else if (kind == 7) {
                // Every byte is exercised; boundary results have explicit fixtures.
                if (form >= 1 && form <= 26) expected[5] = form + 19;
                else if (form >= 27 && form <= 52) expected[5] = form - 7;
                else if (form >= 53 && form <= 78) expected[5] = form - 33;
                else if (form >= 79 && form <= 108) expected[5] = form - 59;
                else if (form >= 109 && form <= 112) expected[5] = form - 63;
                else if (form == 113) expected[5] = form - 40;
                for (unsigned i = 0; i < 14; ++i)
                    if (form == special_input[i] && expected[5] != special_output[i])
                        throw std::runtime_error("special_form_boundary_fixture");
            }
            for (unsigned field = 8; field < 12; ++field) expected[field] = 0;
            std::vector<RussianCandidatePayloadM51> rows{row};
            normalize_russian_candidate_forms_m51(rows);
            if (rows.front() != expected) throw std::runtime_error("normalization_payload_mismatch");
        }
    for (std::size_t count = 0; count <= 70; ++count) {
        std::vector<RussianCandidatePayloadM51> rows(count);
        for (std::size_t i = 0; i < count; ++i) {
            rows[i][0] = 4; rows[i][1] = 2; rows[i][2] = 9;
            // Scores, grammar and dictionary identity must be ignored.
            for (std::size_t f = 3; f < 20; ++f) rows[i][f] = (i * 37 + f) & 255;
        }
        if (russian_candidate_pronunciation_conflict_m51(rows))
            throw std::runtime_error("equal_triples_conflict");
        ++checks;
        if (count < 2) continue;
        for (std::size_t f = 0; f < 3; ++f)
            for (std::size_t i : {std::size_t{0}, count / 2, count - 1}) {
                auto changed = rows; changed[i][f] ^= 128;
                if (!russian_candidate_pronunciation_conflict_m51(changed))
                    throw std::runtime_error("different_triple_not_detected");
                ++checks;
            }
    }
    try { russian_candidate_pronunciation_conflict_m51(std::vector<RussianCandidatePayloadM51>(71)); }
    catch (const std::invalid_argument&) {
        std::vector<RussianCandidatePayloadM51> oversized(71);
        const auto saved = oversized;
        try { normalize_russian_candidate_forms_m51(oversized); }
        catch (const std::invalid_argument&) {
            if (oversized != saved) throw std::runtime_error("normalizer_mutated_refused_input");
            std::cout << "synthetic conflict checks=" << checks
                      << " normalized_payload_checks=65536 capacity_refusals=2\n";
            return 0;
        }
    }
    throw std::runtime_error("invalid_capacity_accepted");
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
