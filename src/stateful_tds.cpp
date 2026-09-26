#include "nicolai/stateful_tds.hpp"
#include "nicolai/legacy_tds.hpp"
#include <algorithm>
#include <cmath>

namespace nicolai {
namespace {
int q11(double x) { return static_cast<int>(std::lround(x * 2048.0)); }
int rounded_delta(int delta, int ordinal) {
    const int v = delta * ordinal + 1024;
    return v >= 0 ? v / 2048 : -((-v + 2047) / 2048);
}
std::vector<std::int16_t> window(int length) {
    if (!length) return {};
    auto w = legacy_half_hann_q15(length);
    w.resize(length); // PC writer consumes length WORDs, no endpoint.
    return w;
}
}
Pcm16Mono resynthesize_stateful_m34(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double ld, double rd, double le, double re, StatefulTdsM34& state) {
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    if (source.sample_rate <= 0 || !timeline.valid || timeline.nodes.size() < 2 ||
        timeline.split_node >= timeline.nodes.size() || source.samples.size() < 2 ||
        timeline.nodes.back().sample != source.samples.size()-1 ||
        !(ld >= 0.125 && ld <= 4.0 && rd >= 0.125 && rd <= 4.0) ||
        !(le > 0.0 && le < 8.0 && re > 0.0 && re < 8.0)) return out;
    // Validate before mutating caller-owned state; malformed intervals never
    // silently fall back to the stable renderer inside an experimental run.
    for (std::size_t i=0; i+1<timeline.nodes.size(); ++i) {
        const auto a=timeline.nodes[i].sample, b=timeline.nodes[i+1].sample;
        if (a >= b || b-a > 3200) return out;
        const double scale=td_psola_pitch_scale_at(pitch,static_cast<double>(a)/(source.samples.size()-1));
        if (!(scale >= 0.125 && scale <= 8.0)) return out;
    }
    auto next_state=state;
    for (std::size_t i=0; i+1<timeline.nodes.size(); ++i) {
        const auto a=timeline.nodes[i].sample, b=timeline.nodes[i+1].sample;
        const int width=static_cast<int>(b-a);
        const std::size_t ni=std::min(i+1,timeline.nodes.size()-2);
        const int next_width=static_cast<int>(timeline.nodes[ni+1].sample-timeline.nodes[ni].sample);
        auto pitch_at=[&](std::size_t index) {
            if (!timeline.nodes[index].voiced) return 2048;
            return q11(td_psola_pitch_scale_at(pitch,
                static_cast<double>(timeline.nodes[index].sample)/(source.samples.size()-1)));
        };
        const int duration_q11=q11(i < timeline.split_node ? ld : rd);
        const int old_carry=next_state.carry;
        const auto step=legacy_tds_step_m33(width,pitch_at(i),
            duration_q11,next_width,pitch_at(ni),old_carry);
        if (!step.valid) return {};
        int target=width*duration_q11/2048;
        if (!target) target=width*2048/pitch_at(i);
        next_state.target_samples+=target;
        next_state.budget_consumed_samples+=old_carry+target-step.carry;
        if (step.delta_q11==32767 || step.delta_q11==-32768) ++next_state.clamped_delta_records;
        next_state.carry=step.carry; ++next_state.intervals;
        if (!step.count) ++next_state.dropped;
        for (int j=0; j<step.count; ++j) {
            const int period=step.first_period+rounded_delta(step.delta_q11,j);
            if (period < 1 || period > 3200) return {};
            const int support=std::min(width,period);
            std::vector<std::int16_t> left(source.samples.begin()+a,source.samples.begin()+a+support);
            std::vector<std::int16_t> right(source.samples.begin()+b-support,source.samples.begin()+b);
            const auto w=window(support);
            const auto cursor=out.samples.size();
            if (!legacy_tds_write_m34(out.samples,cursor,period,left,right,w,w)) return {};
            // M32 phone-local energy remains live; no whole-diphone averaging.
            // A 5 ms blend is evaluated in SOURCE coordinates at the split.
            const double split=static_cast<double>(timeline.nodes[timeline.split_node].sample);
            for (int k=0; k<period; ++k) {
                const double x=a+static_cast<double>(width)*k/period;
                const double t=std::clamp((x-split)/(source.sample_rate*0.005)+0.5,0.0,1.0);
                const double gain=le+(re-le)*t;
                out.samples[cursor+k]=static_cast<std::int16_t>(std::lround(std::clamp(
                    out.samples[cursor+k]*gain,-32768.0,32767.0)));
            }
            ++next_state.grains;
            next_state.emitted_samples+=period;
        }
    }
    // The PC final source node is the last sample, not one-past-end. This
    // adapter emits intervals only; no invented duplicate terminal sample.
    state=next_state;
    return out;
}
}
