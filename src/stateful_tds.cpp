#include "nicolai/stateful_tds.hpp"
#include "nicolai/legacy_runtime_cross_m36.hpp"
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
    runtime.word2e = static_cast<std::int16_t>(interval_index);
    const auto first = legacy_runtime_initial_source_selection_m36(
        positions, runtime, interval_index, step.first_period);
    if (!first.valid || !m36_write(output, runtime, step.first_period, source,
            first.left_source_position, first.left_window_length,
            first.right_source_position, first.right_window_length)) return false;

    for (int ordinal = 1; ordinal < step.count; ++ordinal) {
        const auto grain = legacy_runtime_initial_repeated_grain_m36(
            positions, positions, interval_index, false,
            step.first_period, step.delta_q11, ordinal);
        if (!grain.valid || !m36_write(output, runtime, grain.period, source,
                grain.left_source_position, grain.left_window_length,
                grain.right_source_position, grain.right_window_length)) return false;
    }
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
            const auto w=window_m34(support);
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
    runtime.cursor = 0;
    runtime.end_cursor = 0;
    bool local_started = false;
    bool consumed_pending = false;

    auto pitch_at=[&](std::size_t index) {
        if (!timeline.nodes[index].voiced) return 2048;
        return q11(td_psola_pitch_scale_at(pitch,
            static_cast<double>(timeline.nodes[index].sample)/(source.samples.size()-1)));
    };

    for (std::size_t i=0; i+1<timeline.nodes.size(); ++i) {
        const auto a=timeline.nodes[i].sample, b=timeline.nodes[i+1].sample;
        const int width=static_cast<int>(b-a);
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
            runtime.word26=1;
            runtime.word2a=static_cast<std::int16_t>(i);
            continue;
        }

        // The previous descriptor's deferred terminal is consumed by the first
        // positive step of this descriptor. Unknown caller gate/flag ownership
        // is intentionally not invented: use the proven nonzero cross branch,
        // with a transactional terminal+ordinary fallback when its guarded
        // geometry rejects the case.
        if (next_state.m36_has_pending_terminal && !consumed_pending) {
            const auto geometry=legacy_runtime_cross_geometry_m36(
                next_state.m36_pending_positions,positions,runtime,
                next_state.m36_pending_interval,static_cast<int>(i));
            bool ok=false;
            if (geometry.valid) {
                const int previous_boundary=next_state.m36_pending_positions[
                    static_cast<std::size_t>(geometry.previous_interval_index+1)];
                const int current_boundary=positions[i+1];
                const auto cross=legacy_runtime_execute_cross_m36(
                    out.samples,runtime,next_state.m36_pending_pcm,source.samples,
                    geometry,previous_boundary,current_boundary,
                    next_state.m36_pending_step,step);
                if (cross.valid) {
                    next_state.grains+=static_cast<std::size_t>(cross.total_grains_written);
                    next_state.emitted_samples+=cross.total_samples_written;
                    ++next_state.m36_cross_paths;
                    ok=true;
                }
            }
            if (!ok) {
                ++next_state.m36_fallbacks;
                const auto terminal=legacy_runtime_execute_terminal_m36(
                    out.samples,runtime,next_state.m36_pending_pcm,
                    next_state.m36_pending_positions,next_state.m36_pending_interval,
                    next_state.m36_pending_step);
                if (terminal.valid) {
                    ++next_state.grains;
                    next_state.emitted_samples+=terminal.total_samples_written;
                    ++next_state.m36_terminal_flushes;
                } else if (!m36_execute_compat_interval(out.samples,runtime,
                    next_state.m36_pending_pcm,next_state.m36_pending_positions,
                    next_state.m36_pending_interval,next_state.m36_pending_step)) {
                    return {};
                } else {
                    next_state.grains+=static_cast<std::size_t>(
                        next_state.m36_pending_step.count);
                    for(int ordinal=0;ordinal<next_state.m36_pending_step.count;++ordinal)
                        next_state.emitted_samples+=next_state.m36_pending_step.first_period+
                            rounded_delta(next_state.m36_pending_step.delta_q11,ordinal);
                }

                legacy_runtime_checkpoint_m36(runtime);
                const auto ordinary=legacy_runtime_execute_ordinary_m36(
                    out.samples,runtime,source.samples,positions,static_cast<int>(i),step);
                if (ordinary.valid) {
                    next_state.grains+=static_cast<std::size_t>(ordinary.grains_written);
                    next_state.emitted_samples+=ordinary.total_samples_written;
                } else if (!m36_execute_compat_interval(out.samples,runtime,
                    source.samples,positions,static_cast<int>(i),step)) {
                    return {};
                } else {
                    next_state.grains+=static_cast<std::size_t>(step.count);
                    for(int ordinal=0;ordinal<step.count;++ordinal)
                        next_state.emitted_samples+=step.first_period+
                            rounded_delta(step.delta_q11,ordinal);
                }
            }
            next_state.m36_has_pending_terminal=false;
            next_state.m36_pending_interval=-1;
            next_state.m36_pending_pcm.clear();
            next_state.m36_pending_positions.clear();
            consumed_pending=true;
            local_started=true;
            runtime.word26=0;
            continue;
        }

        if (!local_started) {
            if (m36_execute_initial(out.samples,runtime,source.samples,positions,
                    static_cast<int>(i),step)) {
                ++next_state.m36_initial_paths;
                next_state.grains+=static_cast<std::size_t>(step.count);
                for(int ordinal=0;ordinal<step.count;++ordinal)
                    next_state.emitted_samples+=step.first_period+
                        rounded_delta(step.delta_q11,ordinal);
                local_started=true;
                runtime.word26=0;
                continue;
            }
            ++next_state.m36_fallbacks;
            legacy_runtime_checkpoint_m36(runtime);
            const auto ordinary=legacy_runtime_execute_ordinary_m36(
                out.samples,runtime,source.samples,positions,static_cast<int>(i),step);
            if (ordinary.valid) {
                next_state.grains+=static_cast<std::size_t>(ordinary.grains_written);
                next_state.emitted_samples+=ordinary.total_samples_written;
            } else if (!m36_execute_compat_interval(out.samples,runtime,
                source.samples,positions,static_cast<int>(i),step)) {
                return {};
            } else {
                next_state.grains+=static_cast<std::size_t>(step.count);
                for(int ordinal=0;ordinal<step.count;++ordinal)
                    next_state.emitted_samples+=step.first_period+
                        rounded_delta(step.delta_q11,ordinal);
            }
            local_started=true;
            runtime.word26=0;
            continue;
        }

        const bool terminal = i+2==timeline.nodes.size();
        if (terminal) {
            next_state.m36_has_pending_terminal=true;
            next_state.m36_pending_interval=static_cast<int>(i);
            next_state.m36_pending_step=step;
            next_state.m36_pending_pcm=source.samples;
            next_state.m36_pending_positions=positions;
            continue;
        }

        legacy_runtime_checkpoint_m36(runtime);
        const auto ordinary=legacy_runtime_execute_ordinary_m36(
            out.samples,runtime,source.samples,positions,static_cast<int>(i),step);
        if (ordinary.valid) {
            next_state.grains+=static_cast<std::size_t>(ordinary.grains_written);
            next_state.emitted_samples+=ordinary.total_samples_written;
        } else {
            ++next_state.m36_fallbacks;
            if (!m36_execute_compat_interval(out.samples,runtime,
                    source.samples,positions,static_cast<int>(i),step)) return {};
            next_state.grains+=static_cast<std::size_t>(step.count);
            for(int ordinal=0;ordinal<step.count;++ordinal)
                next_state.emitted_samples+=step.first_period+
                    rounded_delta(step.delta_q11,ordinal);
        }
        runtime.word26=0;
    }

    // A descriptor made solely of a deferred positive terminal cannot return an
    // empty unit through the legacy chain API. Flush that edge case locally;
    // normal multi-interval descriptors keep the terminal buffered for the next
    // descriptor cross path.
    if (out.samples.empty() && next_state.m36_has_pending_terminal &&
        next_state.m36_pending_pcm==source.samples) {
        const auto terminal=legacy_runtime_execute_terminal_m36(
            out.samples,runtime,next_state.m36_pending_pcm,
            next_state.m36_pending_positions,next_state.m36_pending_interval,
            next_state.m36_pending_step);
        if (!terminal.valid) return {};
        ++next_state.grains;
        next_state.emitted_samples+=terminal.total_samples_written;
        ++next_state.m36_terminal_flushes;
        next_state.m36_has_pending_terminal=false;
        next_state.m36_pending_interval=-1;
        next_state.m36_pending_pcm.clear();
        next_state.m36_pending_positions.clear();
    }

    state=std::move(next_state);
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