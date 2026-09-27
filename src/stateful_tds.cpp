#include "nicolai/stateful_tds.hpp"
#include "nicolai/legacy_runtime_executor_m36.hpp"
#include "nicolai/legacy_runtime_grain_m36.hpp"
#include "nicolai/legacy_runtime_state.hpp"
#include "nicolai/legacy_tds.hpp"
#include "nicolai/legacy_window_m36.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace nicolai {
namespace {
int q11(double x) { return static_cast<int>(std::lround(x * 2048.0)); }
int rounded_delta(int delta, int ordinal) {
    const int v = delta * ordinal + 1024;
    return v >= 0 ? v / 2048 : -((-v + 2047) / 2048);
}
std::vector<std::int16_t> window_m34(int length) {
    if (!length) return {};
    auto w = legacy_half_hann_q15(length);
    w.resize(length); // PC writer consumes length WORDs, no endpoint.
    return w;
}
bool m36_env_enabled() {
    const char* value = std::getenv("NICOLAI_M36_TRANSITION_EXECUTOR");
    return value != nullptr && std::atoi(value) != 0;
}

bool timeline_positions(const SegSourceTimelineM33& timeline,
    std::vector<std::int32_t>& out) {
    out.clear();
    out.reserve(timeline.nodes.size());
    for (const auto& node : timeline.nodes) {
        if (node.sample > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
            return false;
        out.push_back(static_cast<std::int32_t>(node.sample));
    }
    return true;
}

bool slice_pcm(const std::vector<std::int16_t>& source, int offset, int length,
    std::vector<std::int16_t>& out) {
    if (offset < 0 || length <= 0) return false;
    const auto begin = static_cast<std::size_t>(offset);
    const auto count = static_cast<std::size_t>(length);
    if (begin > source.size() || count > source.size() - begin) return false;
    out.assign(source.begin() + begin, source.begin() + begin + length);
    return true;
}

bool m36_write(std::vector<std::int16_t>& output, LegacyRuntimeStateM36& runtime,
    int period, const std::vector<std::int16_t>& source,
    int left_offset, int left_length, int right_offset, int right_length) {
    const auto lw = legacy_window_m36_lookup(left_length);
    const auto rw = legacy_window_m36_lookup(right_length);
    if (!lw.valid || !rw.valid) return false;
    std::vector<std::int16_t> left, right;
    if (!slice_pcm(source, left_offset, left_length, left) ||
        !slice_pcm(source, right_offset, right_length, right)) return false;
    if (!legacy_tds_write_m34(output, static_cast<std::size_t>(runtime.cursor),
            period, left, right, lw.q15, rw.q15)) return false;
    return legacy_runtime_post_write_m36(runtime, period).valid;
}

bool m36_execute_initial(std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& runtime,
    const std::vector<std::int16_t>& source,
    const std::vector<std::int32_t>& positions,
    int interval_index, const LegacyTdsStepM33& step) {
    if (!step.valid || step.count <= 0) return false;
    auto staged_output = output;
    auto staged_runtime = runtime;
    staged_runtime.word2e = static_cast<std::int16_t>(interval_index);
    const auto first = legacy_runtime_initial_source_selection_m36(
        positions, staged_runtime, interval_index, step.first_period);
    if (!first.valid || !m36_write(staged_output, staged_runtime, step.first_period, source,
            first.left_source_position, first.left_window_length,
            first.right_source_position, first.right_window_length)) return false;

    for (int ordinal = 1; ordinal < step.count; ++ordinal) {
        const auto grain = legacy_runtime_initial_repeated_grain_m36(
            positions, positions, interval_index, false,
            step.first_period, step.delta_q11, ordinal);
        if (!grain.valid || !m36_write(staged_output, staged_runtime, grain.period, source,
                grain.left_source_position, grain.left_window_length,
                grain.right_source_position, grain.right_window_length)) return false;
    }
    output = std::move(staged_output);
    runtime = staged_runtime;
    return true;
}

bool m36_execute_compat_interval(std::vector<std::int16_t>& output,
    LegacyRuntimeStateM36& runtime,
    const std::vector<std::int16_t>& source,
    const std::vector<std::int32_t>& positions,
    int interval_index, const LegacyTdsStepM33& step) {
    if (!step.valid || step.count <= 0 || interval_index < 0 ||
        interval_index + 1 >= static_cast<int>(positions.size())) return false;
    const int a = positions[static_cast<std::size_t>(interval_index)];
    const int b = positions[static_cast<std::size_t>(interval_index + 1)];
    const int width = b - a;
    if (width <= 0) return false;
    for (int ordinal = 0; ordinal < step.count; ++ordinal) {
        const int period = step.first_period + rounded_delta(step.delta_q11, ordinal);
        if (period < 1 || period > 3200) return false;
        const int support = std::min(width, period);
        std::vector<std::int16_t> left, right;
        if (!slice_pcm(source, a, support, left) ||
            !slice_pcm(source, b - support, support, right)) return false;
        const auto w = window_m34(support);
        if (!legacy_tds_write_m34(output, static_cast<std::size_t>(runtime.cursor),
                period, left, right, w, w) ||
            !legacy_runtime_post_write_m36(runtime, period).valid) return false;
    }
    return true;
}

Pcm16Mono resynthesize_stateful_m34_legacy(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double ld, double rd, double le, double re, StatefulTdsM34& state) {
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    if (source.sample_rate <= 0 || !timeline.valid || timeline.nodes.size() < 2 ||
        timeline.split_node >= timeline.nodes.size() || source.samples.size() < 2 ||
        timeline.nodes.back().sample != source.samples.size()-1 ||
        !(ld >= 0.125 && ld <= 4.0 && rd >= 0.125 && rd <= 4.0) ||
        !(le > 0.0 && le < 8.0 && re > 0.0 && re < 8.0)) return out;
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
            const auto w=window_m34(support);
            const auto cursor=out.samples.size();
            if (!legacy_tds_write_m34(out.samples,cursor,period,left,right,w,w)) return {};
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
    state=next_state;
    return out;
}
} // namespace

Pcm16Mono resynthesize_stateful_m36_experimental(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double ld, double rd, double le, double re, StatefulTdsM34& state) {
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    if (source.sample_rate <= 0 || !timeline.valid || timeline.nodes.size() < 2 ||
        timeline.split_node >= timeline.nodes.size() || source.samples.size() < 2 ||
        timeline.nodes.back().sample != source.samples.size()-1 ||
        !(ld >= 0.125 && ld <= 4.0 && rd >= 0.125 && rd <= 4.0) ||
        std::abs(le-1.0) > 1e-12 || std::abs(re-1.0) > 1e-12) return out;

    std::vector<std::int32_t> positions;
    if (!timeline_positions(timeline, positions)) return out;
    for (std::size_t i=0; i+1<timeline.nodes.size(); ++i) {
        const auto a=timeline.nodes[i].sample, b=timeline.nodes[i+1].sample;
        if (a >= b || b-a > 3200) return out;
        const double scale=td_psola_pitch_scale_at(
            pitch,static_cast<double>(a)/(source.samples.size()-1));
        if (!(scale >= 0.125 && scale <= 8.0)) return out;
    }

    auto next_state = state;
    LegacyRuntimeStateM36 runtime;
    LegacyRuntimeStepBuffersM36 buffers;
    bool local_started = false;

    // Cross-descriptor PCM ownership is deliberately not guessed here. The
    // recovered 0x10108cf0 executor needs source context beyond one portable
    // diphone slice plus caller step-buffer ownership that is not yet proven.
    // The runnable A/B profile therefore isolates exact local initial,
    // ordinary and deferred-terminal behavior first.
    next_state.m36_has_pending_terminal = false;
    next_state.m36_pending_interval = -1;
    next_state.m36_pending_pcm.clear();
    next_state.m36_pending_positions.clear();

    auto pitch_at=[&](std::size_t index) {
        if (!timeline.nodes[index].voiced) return 2048;
        return q11(td_psola_pitch_scale_at(pitch,
            static_cast<double>(timeline.nodes[index].sample)/(source.samples.size()-1)));
    };

    for (std::size_t i=0; i+1<timeline.nodes.size(); ++i) {
        const int width=static_cast<int>(timeline.nodes[i+1].sample-timeline.nodes[i].sample);
        const std::size_t ni=std::min(i+1,timeline.nodes.size()-2);
        const int next_width=static_cast<int>(timeline.nodes[ni+1].sample-timeline.nodes[ni].sample);
        const int duration_q11=q11(i < timeline.split_node ? ld : rd);
        const int old_carry=next_state.carry;
        const auto step=legacy_tds_step_m33(width,pitch_at(i),
            duration_q11,next_width,pitch_at(ni),old_carry);
        if (!step.valid) return {};

        int target=width*duration_q11/2048;
        if (!target) target=width*2048/pitch_at(i);
        next_state.target_samples+=target;
        next_state.budget_consumed_samples+=old_carry+target-step.carry;
        if (step.delta_q11==32767 || step.delta_q11==-32768)
            ++next_state.clamped_delta_records;
        next_state.carry=step.carry;
        ++next_state.intervals;

        if (step.count==0) {
            ++next_state.dropped;
            legacy_runtime_bookkeep_m36(runtime,buffers,static_cast<int>(i),
                static_cast<int>(positions.size()),step,false,local_started);
            continue;
        }

        const bool terminal = i+2==timeline.nodes.size();
        if (!local_started && !terminal) {
            if (m36_execute_initial(out.samples,runtime,source.samples,positions,
                    static_cast<int>(i),step)) {
                ++next_state.m36_initial_paths;
                next_state.grains+=static_cast<std::size_t>(step.count);
                for(int ordinal=0;ordinal<step.count;++ordinal)
                    next_state.emitted_samples+=step.first_period+
                        rounded_delta(step.delta_q11,ordinal);
                legacy_runtime_bookkeep_m36(runtime,buffers,static_cast<int>(i),
                    static_cast<int>(positions.size()),step,false,local_started);
                local_started=true;
                continue;
            }
            ++next_state.m36_fallbacks;
        }

        if (terminal) {
            const auto terminal_result=legacy_runtime_execute_terminal_m36(
                out.samples,runtime,source.samples,positions,static_cast<int>(i),step);
            if (terminal_result.valid) {
                ++next_state.grains;
                next_state.emitted_samples+=terminal_result.total_samples_written;
                ++next_state.m36_terminal_flushes;
                legacy_runtime_bookkeep_m36(runtime,buffers,static_cast<int>(i),
                    static_cast<int>(positions.size()),step,false,local_started);
                local_started=true;
                continue;
            }
            ++next_state.m36_fallbacks;
        } else {
            legacy_runtime_checkpoint_m36(runtime);
            const auto ordinary=legacy_runtime_execute_ordinary_m36(
                out.samples,runtime,source.samples,positions,static_cast<int>(i),step);
            if (ordinary.valid) {
                next_state.grains+=static_cast<std::size_t>(ordinary.grains_written);
                next_state.emitted_samples+=ordinary.total_samples_written;
                legacy_runtime_bookkeep_m36(runtime,buffers,static_cast<int>(i),
                    static_cast<int>(positions.size()),step,false,local_started);
                local_started=true;
                continue;
            }
            ++next_state.m36_fallbacks;
        }

        // Guarded compatibility fallback keeps the A/B run complete when a
        // recovered M36 window/source request leaves the currently proven
        // domain. It deliberately uses the old analytic source-front/tail
        // selection and is counted so a candidate cannot hide behind fallback.
        if (!m36_execute_compat_interval(out.samples,runtime,source.samples,
                positions,static_cast<int>(i),step)) return {};
        next_state.grains+=static_cast<std::size_t>(step.count);
        for(int ordinal=0;ordinal<step.count;++ordinal)
            next_state.emitted_samples+=step.first_period+
                rounded_delta(step.delta_q11,ordinal);
        legacy_runtime_bookkeep_m36(runtime,buffers,static_cast<int>(i),
            static_cast<int>(positions.size()),step,false,local_started);
        local_started=true;
    }

    state=std::move(next_state);
    return out;
}

Pcm16Mono resynthesize_stateful_m36_chain_experimental(
    const std::vector<StatefulTdsUnitM36>& units, StatefulTdsM34& state) {
    if (units.empty()) return {};
    Pcm16Mono out;
    out.sample_rate = units.front().source.sample_rate;
    auto next = state;
    LegacyRuntimeStateM36 runtime;
    LegacyRuntimeStepBuffersM36 buffers;
    std::vector<std::int32_t> previous_positions;
    const StatefulTdsUnitM36* previous = nullptr;
    bool pending = false;

    auto account = [&](int grains, int samples) {
        next.grains += static_cast<std::size_t>(grains);
        next.emitted_samples += samples;
    };
    auto compat = [&](const StatefulTdsUnitM36& unit,
        const std::vector<std::int32_t>& positions,
        int index, const LegacyTdsStepM33& step) {
        const int start = runtime.cursor;
        if (!m36_execute_compat_interval(out.samples,runtime,
                unit.source.samples,positions,index,step)) return false;
        account(step.count,runtime.cursor-start);
        return true;
    };
    auto flush = [&] {
        if (!pending || !previous || !buffers.valid) return true;
        const auto record = buffers.at_74;
        const auto r = legacy_runtime_execute_terminal_m36(out.samples,runtime,
            previous->source.samples,previous_positions,
            record.interval_index,record.step);
        if (r.valid) account(1,r.total_samples_written);
        else {
            ++next.m36_fallbacks;
            if (!compat(*previous,previous_positions,
                    record.interval_index,record.step)) return false;
        }
        ++next.m36_terminal_flushes;
        pending = false;
        runtime.word24=runtime.word26=runtime.word28=runtime.word2a=0;
        runtime.word2c=runtime.word2e=0;
        runtime.field30=0;
        return true;
    };

    for (const auto& unit : units) {
        const auto& source = unit.source;
        const auto& timeline = unit.timeline;
        if (out.sample_rate <= 0 || source.sample_rate != out.sample_rate ||
            !timeline.valid || timeline.nodes.size() < 2 ||
            timeline.nodes.size() > 32768 ||
            timeline.split_node >= timeline.nodes.size() ||
            source.samples.size() < 2 || timeline.nodes.front().sample != 0 ||
            timeline.nodes.back().sample != source.samples.size()-1 ||
            !(unit.left_duration >= .125 && unit.left_duration <= 4.0 &&
              unit.right_duration >= .125 && unit.right_duration <= 4.0) ||
            !(std::abs(unit.left_energy-1.0) <= 1e-12) ||
            !(std::abs(unit.right_energy-1.0) <= 1e-12)) return {};
        std::vector<std::int32_t> positions;
        if (!timeline_positions(timeline,positions)) return {};
        for (std::size_t i=0;i+1<positions.size();++i) {
            if (positions[i+1] <= positions[i] || positions[i+1]-positions[i] > 3200)
                return {};
            const auto scale = td_psola_pitch_scale_at(unit.pitch,
                static_cast<double>(positions[i])/(source.samples.size()-1));
            if (!(scale >= .125 && scale <= 8.0)) return {};
        }
        if (!unit.cross_from_previous && !flush()) return {};
        bool cross_pending = pending;
        bool started = false;
        runtime.field30 = 1;
        auto pitch_at = [&](std::size_t i) {
            return timeline.nodes[i].voiced ? q11(td_psola_pitch_scale_at(unit.pitch,
                static_cast<double>(positions[i])/(source.samples.size()-1))) : 2048;
        };
        for (std::size_t i=0;i+1<positions.size();++i) {
            const std::size_t ni=std::min(i+1,positions.size()-2);
            const int width=positions[i+1]-positions[i];
            const int next_width=positions[ni+1]-positions[ni];
            const int duration=q11(i<timeline.split_node ?
                unit.left_duration : unit.right_duration);
            const int old_carry=next.carry;
            const auto step=legacy_tds_step_m33(width,pitch_at(i),duration,
                next_width,pitch_at(ni),old_carry);
            if (!step.valid) return {};
            int target=width*duration/2048;
            if (!target) target=width*2048/pitch_at(i);
            next.target_samples+=target;
            next.budget_consumed_samples+=old_carry+target-step.carry;
            next.carry=step.carry;
            ++next.intervals;
            if (step.delta_q11==32767 || step.delta_q11==-32768)
                ++next.clamped_delta_records;
            const int index=static_cast<int>(i);
            const int count=static_cast<int>(positions.size());
            if (!step.count) {
                ++next.dropped;
                legacy_runtime_bookkeep_m36(runtime,buffers,index,count,
                    step,cross_pending,started);
                continue;
            }
            if (cross_pending) {
                const auto old=buffers.at_74;
                const auto r=legacy_runtime_execute_cross_route_m36(
                    out.samples,runtime,buffers,previous->source.samples,
                    previous_positions,source.samples,positions,
                    old.interval_index,index,count,started,old.step,step);
                if (r.valid) {
                    account(r.pcm.total_grains_written,r.pcm.total_samples_written);
                    ++next.m36_cross_paths;
                } else {
                    // The exact executor is transactional. If a window/source
                    // request is unsupported, emit each record exactly once.
                    ++next.m36_fallbacks;
                    if (!compat(*previous,previous_positions,old.interval_index,old.step) ||
                        !compat(unit,positions,index,step)) return {};
                    legacy_runtime_bookkeep_m36(runtime,buffers,index,count,step,true,started);
                }
                pending=false;
                cross_pending=false;
            } else if (i+2==positions.size()) {
                // Defer this positive terminal record to the next descriptor.
                // Do NOT attenuate every diphone tail in a voiced chain.
                legacy_runtime_bookkeep_m36(runtime,buffers,index,count,step,false,started);
                pending=true;
            } else {
                if (started) legacy_runtime_checkpoint_m36(runtime);
                const auto r=started ? legacy_runtime_execute_ordinary_m36(
                    out.samples,runtime,source.samples,positions,index,step) :
                    legacy_runtime_execute_zero_cross_m36(
                    out.samples,runtime,source.samples,positions,index,step);
                if (r.valid) account(r.grains_written,r.total_samples_written);
                else {
                    ++next.m36_fallbacks;
                    if (!compat(unit,positions,index,step)) return {};
                }
                if (!started) ++next.m36_initial_paths;
                // This branch starts with ordinary/zero-cross, not the
                // separate two-record 0x101086c0 initial executor.
                legacy_runtime_bookkeep_m36(runtime,buffers,index,count,step,!started,true);
            }
            started=true;
        }
        if (cross_pending) {
            // An entirely dropped descriptor cannot steal old PCM ownership.
            if (!flush()) return {};
        }
        previous=&unit;
        previous_positions=std::move(positions);
    }
    if (!flush()) return {};
    next.m36_has_pending_terminal=false;
    next.m36_pending_interval=-1;
    next.m36_pending_pcm.clear();
    next.m36_pending_positions.clear();
    state=std::move(next);
    return out;
}

Pcm16Mono resynthesize_stateful_m34(const Pcm16Mono& source,
    const SegSourceTimelineM33& timeline, const TdPsolaConfig& pitch,
    double ld, double rd, double le, double re, StatefulTdsM34& state) {
    if (m36_env_enabled())
        return resynthesize_stateful_m36_experimental(
            source,timeline,pitch,ld,rd,le,re,state);
    return resynthesize_stateful_m34_legacy(
        source,timeline,pitch,ld,rd,le,re,state);
}
}
