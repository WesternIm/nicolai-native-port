#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace nicolai {
struct WordRhythmUnitM43 {
    double left_source_samples = 0, right_source_samples = 0;
    double left_scale = 1, right_scale = 1;
};
struct WordRhythmBudgetM43 {
    std::size_t first_phone = 0, last_phone = 0;
    double baseline_samples = 0, trial_samples = 0, effective_strength = 0;
};
struct WordRhythmPlanM43 {
    bool valid = false;
    std::vector<WordRhythmUnitM43> units;
    std::vector<WordRhythmBudgetM43> words;
    std::size_t skipped_words = 0;
};
// Experimental duration allocation, not original feature-builder parity.
// A phone owns previous-right + next-left support. Nominal duration weights
// redistribute each word's existing spoken budget; # sides are untouched.
// One common blend strength per word keeps that budget while respecting
// 0.2..3 scales and a maximum 1.5x change per nonempty source side.
WordRhythmPlanM43 plan_word_rhythm_m43(const std::vector<std::string>& phones,
    const std::vector<WordRhythmUnitM43>& units,
    const std::vector<double>& nominal_weights, double strength);
}
