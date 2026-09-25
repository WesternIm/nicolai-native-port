#include "nicolai/td_psola.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    nicolai::DiphoneUnit u;
    u.metadata = {2,4,2,4,100,100,100,100};
    auto s = nicolai::parse_clean_voiced_seg_schedule(u, 500);
    assert(s.valid);
    assert(s.periods.size() == 4);
    assert(s.marks.size() == 5);
    assert(s.left_margin == 50 && s.right_margin == 50);
    assert(s.marks[0] == 50 && s.marks.back() == 450);

    const auto h = nicolai::legacy_half_hann_q15(100);
    assert(h.size() == 101);
    assert(h.front() == 32767);
    assert(std::abs(h.back()) <= 1);
    for (std::size_t i=1;i<h.size();++i) assert(h[i] <= h[i-1]);

    nicolai::Pcm16Mono p;
    p.sample_rate = 16000;
    p.samples.resize(500);
    for (std::size_t i=0;i<p.samples.size();++i)
        p.samples[i] = static_cast<std::int16_t>(12000.0 * std::sin(2.0*3.141592653589793*i/100.0));

    nicolai::TdPsolaConfig contour;
    contour.use_three_point_pitch = true;
    contour.pitch_scale_start = 1.2;
    contour.pitch_scale_mid = 1.0;
    contour.pitch_scale_end = 0.8;
    assert(std::abs(nicolai::td_psola_pitch_scale_at(contour, 0.0) - 1.2) < 1e-12);
    assert(std::abs(nicolai::td_psola_pitch_scale_at(contour, 0.25) - 1.1) < 1e-12);
    assert(std::abs(nicolai::td_psola_pitch_scale_at(contour, 0.5) - 1.0) < 1e-12);
    assert(std::abs(nicolai::td_psola_pitch_scale_at(contour, 0.75) - 0.9) < 1e-12);
    assert(std::abs(nicolai::td_psola_pitch_scale_at(contour, 1.0) - 0.8) < 1e-12);

    nicolai::TdPsolaDiagnostics d;
    auto q = nicolai::td_psola_resynthesize(p, s, {2.0,1.5}, &d);
    assert(d.valid);
    assert(q.samples.size() == 750);
    assert(d.synthesis_marks > d.source_marks);
    assert(d.mean_target_period > 40.0 && d.mean_target_period < 60.0);
    std::cout << "td_psola_test: PASSED source_marks=" << d.source_marks
              << " synth_marks=" << d.synthesis_marks
              << " target_period=" << d.mean_target_period << "\n";
}
