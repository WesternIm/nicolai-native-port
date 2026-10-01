#include "nicolai/td_psola.hpp"
#include "nicolai/legacy_tds.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <limits>

int main() try {
    assert(nicolai::legacy_reciprocal_pitch_m34(100,100,200,0)==2684354);
    assert(nicolai::legacy_reciprocal_pitch_m34(100,100,200,50)==2013304);
    assert(nicolai::legacy_reciprocal_pitch_m34(0,100,200,50)==0);
    std::vector<std::int16_t> written{77};
    assert(nicolai::legacy_tds_write_m34(written,1,3,{100,-100},{200,-200},{32767,32767},{32767,32767}));
    assert((written==std::vector<std::int16_t>{77,99,99,-200}));
    const auto saved=written;
    assert(!nicolai::legacy_tds_write_m34(written,0,1,{1,2},{},{1,2},{}));
    assert(written==saved);
    std::vector<std::int16_t> gap;
    assert(nicolai::legacy_tds_write_m34(gap,0,5,{100},{200},{32767},{32767}));
    assert((gap==std::vector<std::int16_t>{99,0,0,0,199}));
    assert(!nicolai::legacy_tds_write_m34(gap,6,1,{},{},{},{}));
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

    // M40: do not leave a full-scale step where grain coverage ends and the
    // original time-mapped source takes over. The default path is untouched.
    nicolai::Pcm16Mono seam;
    seam.sample_rate = 16000;
    seam.samples = {0, 0, -12000, -12000, -12000, 10000, 10000, 10000};
    const std::vector<std::int16_t> mapped(8, 10000);
    const std::vector<double> weights{1, 1, 1, 1, 1, 0, 0, 0};
    auto all_covered = seam;
    nicolai::blend_uncovered_edges_m40(all_covered, mapped,
        std::vector<double>(8, 1.0), 4);
    assert(all_covered.samples == seam.samples);
    nicolai::blend_uncovered_edges_m40(seam, mapped, weights, 4);
    assert(seam.samples[0] == 0 && seam.samples[5] == 10000 && seam.samples[7] == 10000);
    int largest_step = 0;
    for (std::size_t i = 1; i < seam.samples.size(); ++i)
        largest_step = std::max(largest_step,
            std::abs(static_cast<int>(seam.samples[i]) - seam.samples[i - 1]));
    assert(largest_step < 22000);
    auto require = [](bool ok) { if (!ok) throw std::runtime_error("M41 uncovered interpolation contract failed"); };
    nicolai::Pcm16Mono ramp;
    ramp.sample_rate=16000;
    for(int i=0;i<256;++i) ramp.samples.push_back(static_cast<std::int16_t>(-12800+100*i));
    require(nicolai::sample_pcm16_linear(ramp,10.5)==-11750);
    require(nicolai::sample_pcm16_linear(ramp,-1)==ramp.samples.front());
    require(nicolai::sample_pcm16_linear(ramp,300)==ramp.samples.back());
    require(nicolai::sample_pcm16_linear({},0)==0);
    require(nicolai::sample_pcm16_linear(ramp,std::numeric_limits<double>::quiet_NaN())==0);
    nicolai::SegPitchSchedule short_schedule;
    short_schedule.valid=true; short_schedule.marks={40,80}; short_schedule.periods={40};
    nicolai::TdPsolaConfig mapping;
    mapping.duration_scale=0.73;
    const auto old_edge=nicolai::td_psola_resynthesize(ramp,short_schedule,mapping);
    mapping.interpolate_uncovered_m41=true;
    const auto fractional_edge=nicolai::td_psola_resynthesize(ramp,short_schedule,mapping);
    require(old_edge.samples.size()==fractional_edge.samples.size());
    const double tail=static_cast<double>(fractional_edge.samples.size()-1)/mapping.duration_scale;
    require(old_edge.samples.back()==ramp.samples[static_cast<std::size_t>(tail)]);
    require(fractional_edge.samples.back()==nicolai::sample_pcm16_linear(ramp,tail));
    require(old_edge.samples[50]==fractional_edge.samples[50]);
    require(old_edge.samples.back()!=fractional_edge.samples.back());
    // M42: reconcile the two target periods of a shared phone, not the
    // ratios themselves (the source recordings have different periods).
    for(double strength : {0.25,0.5,1.0}) {
        nicolai::TdPsolaConfig left, right;
        require(nicolai::reconcile_join_pitch_m42(left,right,160,220,strength));
        const double lp=160/nicolai::td_psola_pitch_scale_at(left,1);
        const double rp=220/nicolai::td_psola_pitch_scale_at(right,0);
        const double residual=std::abs(1200*std::log2(lp/rp));
        require(std::abs(residual-(1-strength)*std::abs(1200*std::log2(160.0/220)))<1e-8);
        require(left.pitch_scale_mid==1 && right.pitch_scale_mid==1);
        require(left.pitch_scale_start==1 && right.pitch_scale_end==1);
        if(strength==1) require(std::abs(lp-std::sqrt(160.0*220))<1e-8 && std::abs(lp-rp)<1e-8);
    }
    for(const auto periods : {std::pair<double,double>{0,200}, {160,0}, {160,160},
        {80,160}, {10,200}, {500,550}, {std::numeric_limits<double>::quiet_NaN(),200}}) {
        nicolai::TdPsolaConfig left, right;
        require(!nicolai::reconcile_join_pitch_m42(left,right,periods.first,periods.second,1));
        require(!left.use_three_point_pitch && !right.use_three_point_pitch);
        require(left.pitch_scale_end==1 && right.pitch_scale_start==1);
    }
    nicolai::TdPsolaConfig keep_left, keep_right;
    require(!nicolai::reconcile_join_pitch_m42(keep_left,keep_right,160,220,0));
    require(!keep_left.use_three_point_pitch && !keep_right.use_three_point_pitch);
    require(!nicolai::reconcile_join_pitch_m42(keep_left,keep_right,160,220,
        std::numeric_limits<double>::quiet_NaN()));
    require(!keep_left.use_three_point_pitch && !keep_right.use_three_point_pitch);
    keep_left.use_three_point_pitch=true;
    keep_left.pitch_scale_mid=std::numeric_limits<double>::quiet_NaN();
    require(!nicolai::reconcile_join_pitch_m42(keep_left,keep_right,160,220,1));
    require(keep_left.pitch_scale_end==1 && !keep_right.use_three_point_pitch);
    nicolai::TdPsolaConfig shaped_left, shaped_right;
    shaped_left.use_three_point_pitch=shaped_right.use_three_point_pitch=true;
    shaped_left.pitch_scale_start=0.9; shaped_left.pitch_scale_mid=1.1;
    shaped_left.pitch_scale_end=1.2;
    shaped_right.pitch_scale_start=0.8; shaped_right.pitch_scale_mid=1.3;
    shaped_right.pitch_scale_end=1.4;
    require(nicolai::reconcile_join_pitch_m42(shaped_left,shaped_right,180,160,2));
    require(std::abs(180/shaped_left.pitch_scale_end-160/shaped_right.pitch_scale_start)<1e-8);
    require(shaped_left.pitch_scale_start==0.9 && shaped_left.pitch_scale_mid==1.1);
    require(shaped_right.pitch_scale_mid==1.3 && shaped_right.pitch_scale_end==1.4);
    std::cout << "td_psola_test: PASSED source_marks=" << d.source_marks
              << " synth_marks=" << d.synthesis_marks
              << " target_period=" << d.mean_target_period << "\n";
} catch(const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
