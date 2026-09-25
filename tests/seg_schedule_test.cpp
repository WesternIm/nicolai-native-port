#include "nicolai/seg_schedule.hpp"
#include <cassert>
#include <iostream>

int main() {
    {
        nicolai::DiphoneUnit u;
        u.metadata = {2,4,2,4,100,100,100,100};
        const auto s = nicolai::parse_seg_schedule_m15(u);
        assert(s.valid);
        assert(s.runs.size() == 1);
        assert(s.voiced_slots == 4 && s.unvoiced_slots == 0);
        const auto l = nicolai::layout_seg_runs_m15(s, 500);
        assert(l.valid && l.runs.size() == 1);
        assert(l.runs[0].source_begin == 0 && l.runs[0].source_end == 500);
        assert(l.split_sample_estimate == 250);
    }
    {
        nicolai::DiphoneUnit u;
        u.metadata = {-2,17,10,-9,8,-104,105,149,171,172,198,200,220};
        const auto s = nicolai::parse_seg_schedule_m15(u);
        assert(s.valid);
        assert(s.runs.size() == 2);
        assert(!s.runs[0].voiced && s.runs[0].slots == 9);
        assert(s.runs[1].voiced && s.runs[1].slots == 8);
        assert(s.runs[1].signed_period_reset);
        assert(s.runs[1].periods[0] == 104 && s.runs[1].periods[1] == 105);
        const auto l = nicolai::layout_seg_runs_m15(s, 2994);
        assert(l.valid && l.runs.size() == 2);
        assert(l.runs.front().source_begin == 0);
        assert(l.runs.back().source_end == 2994);
        assert(l.unvoiced_budget_samples == 1675);
    }
    {
        nicolai::DiphoneUnit u;
        u.metadata = {2,20,5,2,238,254,-7,6,-157,158,166,166,278,302,-5};
        const auto s = nicolai::parse_seg_schedule_m15(u);
        assert(s.valid);
        assert(s.runs.size() == 4);
        assert(s.runs[0].voiced && !s.runs[1].voiced && s.runs[2].voiced && !s.runs[3].voiced);
        assert(s.voiced_slots == 8 && s.unvoiced_slots == 12);
        assert(s.signed_period_resets == 1);
        const auto l = nicolai::layout_seg_runs_m15(s, 3800);
        assert(l.valid);
        assert(l.runs.back().source_end == 3800);
    }
    std::cout << "seg_schedule_test: PASSED\n";
}
