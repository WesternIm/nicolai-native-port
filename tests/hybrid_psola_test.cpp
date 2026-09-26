#include "nicolai/hybrid_psola.hpp"
#include "nicolai/stateful_tds.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

int main() {
    nicolai::Pcm16Mono regular; regular.sample_rate=16000; regular.samples.resize(301);
    for(std::size_t i=0;i<regular.samples.size();++i) regular.samples[i]=static_cast<std::int16_t>(i*20-3000);
    nicolai::SegSourceTimelineM33 timeline;
    timeline.valid=true; timeline.split_node=1;
    timeline.nodes={{0,true},{100,true},{200,true},{300,true}};
    nicolai::StatefulTdsM34 state;
    auto stateful=nicolai::resynthesize_stateful_m34(regular,timeline,{},0.5,0.5,1,1,state);
    assert(stateful.samples.size()==100 && state.carry==50 && state.dropped==2);
    assert(state.target_samples==150 && state.budget_consumed_samples==100 && state.emitted_samples==100);
    auto continued=nicolai::resynthesize_stateful_m34(regular,timeline,{},0.5,0.5,1,1,state);
    assert(continued.samples.size()==200 && state.carry==0 && state.intervals==6);
    assert(state.target_samples==300 && state.budget_consumed_samples==300 && state.emitted_samples==300);
    nicolai::StatefulTdsM34 shaped_state,flat_state,energy_state;
    nicolai::TdPsolaConfig state_contour; state_contour.use_three_point_pitch=true;
    state_contour.pitch_scale_start=1.2; state_contour.pitch_scale_mid=1.0; state_contour.pitch_scale_end=0.8;
    const auto sc=nicolai::resynthesize_stateful_m34(regular,timeline,state_contour,1,1,1,1,shaped_state);
    const auto sf=nicolai::resynthesize_stateful_m34(regular,timeline,{},1,1,1,1,flat_state);
    const auto se=nicolai::resynthesize_stateful_m34(regular,timeline,{},1,1,0.5,1,energy_state);
    assert(!sc.samples.empty() && sc.samples!=sf.samples && se.samples.size()==sf.samples.size());
    assert(shaped_state.target_samples-shaped_state.carry==shaped_state.budget_consumed_samples);
    assert(shaped_state.emitted_samples==static_cast<std::int64_t>(sc.samples.size()));
    assert(energy_state.target_samples==flat_state.target_samples && energy_state.emitted_samples==flat_state.emitted_samples);
    assert(se.samples.front()!=sf.samples.front() && se.samples.back()==sf.samples.back());
    const auto before=state;
    timeline.nodes[1].sample=0;
    assert(nicolai::resynthesize_stateful_m34(regular,timeline,{},1,1,1,1,state).samples.empty());
    assert(state.carry==before.carry && state.intervals==before.intervals);
    assert(state.target_samples==before.target_samples && state.emitted_samples==before.emitted_samples);
    nicolai::DiphoneUnit u;
    u.metadata = {-2,17,10,-9,8,-104,105,149,171,172,198,200,220};
    const auto s = nicolai::parse_seg_schedule_m15(u);
    assert(s.valid);
    const auto l = nicolai::layout_seg_runs_m15(s, 2994);
    assert(l.valid);

    nicolai::Pcm16Mono p;
    p.sample_rate = 16000;
    p.samples.resize(2994);
    for (std::size_t i=0;i<p.samples.size();++i) {
        // deterministic unvoiced-ish first region, voiced-ish second region
        if (i < l.runs[0].source_end)
            p.samples[i] = static_cast<std::int16_t>(((i*1103515245u + 12345u)>>16) % 8000 - 4000);
        else
            p.samples[i] = static_cast<std::int16_t>(9000.0 * std::sin(2.0*3.141592653589793*i/170.0));
    }

    nicolai::M15UnitDiagnostics d;
    auto q = nicolai::resynthesize_seg_m15(p, s, l, {1.15,1.20}, &d);
    assert(!q.samples.empty());
    assert(d.runs.size() == 2);
    assert(d.runs[0].used_unvoiced_stretch);
    assert(d.runs[1].used_td_psola);
    assert(d.runs[1].signed_period_reset);
    assert(d.internal_joins.size() == 1);

    // M32's zero-effect dispatch must preserve the existing M15 renderer
    // exactly, including the M24 three-point pitch contour.
    nicolai::TdPsolaConfig contour;
    contour.duration_scale = 1.10;
    contour.use_three_point_pitch = true;
    contour.pitch_scale_start = 1.20;
    contour.pitch_scale_mid = 1.00;
    contour.pitch_scale_end = 0.80;
    contour.pitch_scale = 1.00;
    auto baseline = nicolai::resynthesize_seg_m15(p, s, l, contour);
    auto exact = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, l, contour, 1.10, 1.10, 1.0, 1.0);
    assert(!baseline.samples.empty() && exact.samples == baseline.samples);

    // Unequal phone-side timing must retain the contour instead of collapsing
    // to the old M23 single-pitch fallback.
    nicolai::TdPsolaConfig flat = contour;
    flat.use_three_point_pitch = false;
    flat.pitch_scale = 1.0;
    auto shaped = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, l, contour, 0.85, 1.20, 1.0, 1.0);
    auto flat_shaped = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, l, flat, 0.85, 1.20, 1.0, 1.0);
    assert(!shaped.samples.empty() && !flat_shaped.samples.empty());
    assert(shaped.samples != flat_shaped.samples);

    // Energy is consumed locally: attenuating only the left phone side leaves
    // the tail unchanged while reducing the leading quarter.
    auto unity = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, l, contour, 1.0, 1.0, 1.0, 1.0);
    auto local_energy = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, l, contour, 1.0, 1.0, 0.5, 1.0);
    assert(local_energy.samples.size() == unity.samples.size());
    long long lead_unity=0, lead_local=0, tail_unity=0, tail_local=0;
    const std::size_t quarter=unity.samples.size()/4;
    for(std::size_t i=0;i<quarter;++i){
        lead_unity += std::abs(static_cast<int>(unity.samples[i]));
        lead_local += std::abs(static_cast<int>(local_energy.samples[i]));
    }
    for(std::size_t i=unity.samples.size()-quarter;i<unity.samples.size();++i){
        tail_unity += std::abs(static_cast<int>(unity.samples[i]));
        tail_local += std::abs(static_cast<int>(local_energy.samples[i]));
    }
    assert(lead_local < lead_unity * 3 / 4);
    assert(std::llabs(tail_local-tail_unity) <= static_cast<long long>(quarter));
    auto right_only_layout = l;
    right_only_layout.split_sample_estimate = 0;
    auto right_only = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, right_only_layout, contour, 1.0, 1.0, 0.5, 1.0);
    auto right_only_unity = nicolai::resynthesize_seg_m32_phone_sides(
        p, s, right_only_layout, contour, 1.0, 1.0, 1.0, 1.0);
    assert(right_only.samples == right_only_unity.samples);
    const auto pc_layout = nicolai::layout_seg_runs_m33(s,p.samples.size(),true);
    assert(pc_layout.valid);
    auto pc_contour = nicolai::resynthesize_seg_m32_phone_sides(
        p,s,pc_layout,contour,0.85,1.20,0.5,1.0);
    auto pc_flat = nicolai::resynthesize_seg_m32_phone_sides(
        p,s,pc_layout,flat,0.85,1.20,0.5,1.0);
    assert(!pc_contour.samples.empty() && pc_contour.samples!=pc_flat.samples);
    std::cout << "hybrid_psola_test: PASSED output=" << q.samples.size() << "\n";
}
