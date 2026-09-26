#include "nicolai/td_psola.hpp"
#include "nicolai/legacy_tds.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const auto step = nicolai::legacy_tds_step_m33(100,2048,2048,100,2048,0);
    assert(step.valid && step.count==1 && step.first_period==100 && step.carry==0);
    const auto drop = nicolai::legacy_tds_step_m33(100,2048,1024,100,2048,0);
    assert(drop.valid && drop.count==0 && drop.carry==50);
    const auto carried = nicolai::legacy_tds_step_m33(100,2048,1024,100,2048,drop.carry);
    assert(carried.valid && carried.count==1 && carried.carry==0);
    const auto repeated = nicolai::legacy_tds_step_m33(100,2048,4096,100,2048,0);
    assert(repeated.valid && repeated.count==2 && repeated.carry==0);
    assert(!nicolai::legacy_tds_step_m33(100,0,2048,100,2048,0).valid);
    assert(!nicolai::legacy_tds_step_m33(1,16384,1,100,2048,0).valid);
    const auto rising = nicolai::legacy_tds_step_m33(160,2048,3072,180,2048,17);
    assert(rising.valid && rising.count==2 && rising.delta_q11==20480 && rising.carry==-75);
    const auto falling = nicolai::legacy_tds_step_m33(180,2048,3072,160,2048,-9);
    assert(falling.valid && falling.count==1 && falling.delta_q11==-32768 && falling.carry==81);
    const auto clamp = nicolai::legacy_tds_step_m33(100,2048,2048,1000,2048,0);
    assert(clamp.valid && clamp.delta_q11==32767);
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
