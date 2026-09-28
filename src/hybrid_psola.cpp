#include "nicolai/hybrid_psola.hpp"
#include "nicolai/g711.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>

namespace nicolai {
namespace {

std::int16_t clip16(double x) {
    x = std::max(-32768.0, std::min(32767.0, x));
    return static_cast<std::int16_t>(std::lrint(x));
}

Pcm16Mono slice_pcm(const Pcm16Mono& source, std::size_t begin, std::size_t end) {
    Pcm16Mono out;
    out.sample_rate = source.sample_rate;
    begin = std::min(begin, source.samples.size());
    end = std::min(end, source.samples.size());
    if (end <= begin) return out;
    out.samples.insert(out.samples.end(),
                       source.samples.begin() + static_cast<std::ptrdiff_t>(begin),
                       source.samples.begin() + static_cast<std::ptrdiff_t>(end));
    return out;
}

SegPitchSchedule make_run_pitch_schedule(const SegRunSpan& span, std::size_t source_samples) {
    SegPitchSchedule s;
    s.mode = 2;
    s.declared_periods = static_cast<int>(span.periods.size());
    s.repeated_period_count = s.declared_periods;
    s.split_index = 0;
    s.periods = span.periods;
    if (!span.voiced || span.periods.empty() || source_samples == 0) {
        s.error = "not_voiced";
        return s;
    }
    if (span.use_explicit_marks) {
        s.marks = span.source_marks;
        if (s.marks.empty() || s.marks.back() >= source_samples ||
            !std::is_sorted(s.marks.begin(), s.marks.end())) {
            s.error = "invalid_explicit_marks"; return s;
        }
        s.valid = true;
        return s;
    }

    std::size_t period_sum = 0;
    for (const int p : span.periods) period_sum += static_cast<std::size_t>(p);
    if (period_sum > source_samples) {
        s.error = "run_periods_exceed_span";
        return s;
    }
    const std::size_t slack = source_samples - period_sum;
    s.left_margin = slack / 2;
    s.right_margin = slack - s.left_margin;

    // M15's complete SEG grammar shows one period value per voiced slot.
    // Therefore use one pitch mark per recorded period rather than M14's
    // provisional N+1 boundary-mark interpretation.
    s.marks.reserve(span.periods.size());
    std::size_t pos = s.left_margin;
    for (std::size_t i = 0; i < span.periods.size(); ++i) {
        if (pos >= source_samples) {
            s.error = "run_mark_out_of_bounds";
            s.marks.clear();
            return s;
        }
        s.marks.push_back(pos);
        if (i + 1 < span.periods.size())
            pos += static_cast<std::size_t>(span.periods[i]);
    }
    s.valid = !s.marks.empty();
    return s;
}

int first_period(const SegRunSpan& s) {
    return s.voiced && !s.periods.empty() ? s.periods.front() : 0;
}
int last_period(const SegRunSpan& s) {
    return s.voiced && !s.periods.empty() ? s.periods.back() : 0;
}

} // namespace

Pcm16Mono stretch_unvoiced_ola(const Pcm16Mono& source, double duration_scale) {
    Pcm16Mono out;
    out.sample_rate = source.sample_rate;
    if (source.samples.empty() || source.sample_rate <= 0 ||
        !(duration_scale > 0.05 && duration_scale < 8.0)) return out;
    if (std::abs(duration_scale - 1.0) < 1e-12) {
        out.samples = source.samples;
        return out;
    }

    const std::size_t target_len = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::llround(source.samples.size() * duration_scale)));
    // 10 ms analysis frame with 50% analysis overlap. For the practical Tempo
    // range this keeps noisy/fricative spectra intact much better than sample
    // resampling: duration is changed by synthesis-hop spacing, not by changing
    // the samples inside each frame.
    std::size_t frame = static_cast<std::size_t>(std::max(64, source.sample_rate / 100));
    frame = std::min(frame, source.samples.size());
    if (frame < 16) {
        out.samples.resize(target_len);
        for (std::size_t i=0;i<target_len;++i) {
            const auto j = std::min<std::size_t>(source.samples.size()-1,
                static_cast<std::size_t>(static_cast<double>(i) / duration_scale));
            out.samples[i] = source.samples[j];
        }
        return out;
    }
    const std::size_t analysis_hop = std::max<std::size_t>(1, frame / 2);
    const std::size_t synthesis_hop = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::llround(analysis_hop * duration_scale)));

    std::vector<double> acc(target_len + frame + 8, 0.0);
    std::vector<double> wsum(acc.size(), 0.0);
    constexpr double kPi = 3.141592653589793238462643383279502884;

    std::size_t src = 0, dst = 0;
    while (src < source.samples.size() && dst < acc.size()) {
        const std::size_t available = std::min(frame, source.samples.size() - src);
        for (std::size_t i=0;i<available && dst+i<acc.size();++i) {
            const double t = frame > 1 ? static_cast<double>(i) / static_cast<double>(frame - 1) : 0.0;
            const double w = 0.5 - 0.5 * std::cos(2.0 * kPi * t);
            acc[dst+i] += w * source.samples[src+i];
            wsum[dst+i] += w;
        }
        if (source.samples.size() - src <= analysis_hop) break;
        src += analysis_hop;
        dst += synthesis_hop;
    }

    out.samples.resize(target_len, 0);
    for (std::size_t i=0;i<target_len;++i) {
        if (wsum[i] > 1e-8) {
            out.samples[i] = clip16(acc[i] / wsum[i]);
        } else {
            // Edge/gap fallback only; map time without interpolation to avoid
            // introducing a second spectral resampling path.
            const auto j = std::min<std::size_t>(source.samples.size()-1,
                static_cast<std::size_t>(static_cast<double>(i) / duration_scale));
            out.samples[i] = source.samples[j];
        }
    }
    return out;
}


namespace {

struct PiecewiseDurationMapM23 {
    double split = 0.0;
    double left = 1.0;
    double right = 1.0;
    double source_len = 0.0;

    double map(double x) const {
        x = std::clamp(x, 0.0, source_len);
        if (x <= split) return x * left;
        return split * left + (x - split) * right;
    }
    double inverse(double y) const {
        const double ys = split * left;
        if (y <= ys || right <= 1e-12)
            return std::clamp(y / std::max(left, 1e-12), 0.0, source_len);
        return std::clamp(split + (y - ys) / right, 0.0, source_len);
    }
    double target_len() const { return map(source_len); }
};

std::size_t nearest_mark_index_m23(const std::vector<std::size_t>& marks, double x) {
    if (marks.empty()) return 0;
    auto it = std::lower_bound(marks.begin(), marks.end(), static_cast<std::size_t>(std::max(0.0, x)));
    if (it == marks.begin()) return 0;
    if (it == marks.end()) return marks.size() - 1;
    const std::size_t hi = static_cast<std::size_t>(it - marks.begin());
    const std::size_t lo = hi - 1;
    return (x - static_cast<double>(marks[lo]) <= static_cast<double>(marks[hi]) - x) ? lo : hi;
}

int local_period_m23(const SegPitchSchedule& s, std::size_t mark_index) {
    if (s.periods.empty()) return 0;
    if (mark_index == 0) return s.periods.front();
    if (mark_index >= s.periods.size()) return s.periods.back();
    return (s.periods[mark_index - 1] + s.periods[mark_index]) / 2;
}

double grain_hann_m23(long rel, int radius) {
    constexpr double kPi = 3.141592653589793238462643383279502884;
    if (radius <= 0 || std::abs(rel) > radius) return 0.0;
    const double x = static_cast<double>(rel + radius) / static_cast<double>(2 * radius);
    return 0.5 - 0.5 * std::cos(2.0 * kPi * x);
}

Pcm16Mono td_psola_piecewise_m23(
    const Pcm16Mono& source,
    const SegPitchSchedule& schedule,
    const TdPsolaConfig& pitch_config,
    const PiecewiseDurationMapM23& tm,
    TdPsolaDiagnostics* diagnostics) {

    TdPsolaDiagnostics d;
    d.pitch_scale = pitch_config.pitch_scale;
    d.duration_scale = source.samples.empty() ? 1.0 : tm.target_len() / source.samples.size();
    d.source_marks = schedule.marks.size();
    if (!schedule.periods.empty()) {
        d.mean_source_period = std::accumulate(schedule.periods.begin(), schedule.periods.end(), 0.0) /
                               static_cast<double>(schedule.periods.size());
    }
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    const double ps0 = td_psola_pitch_scale_at(pitch_config, 0.0);
    const double psm = td_psola_pitch_scale_at(pitch_config, 0.5);
    const double ps1 = td_psola_pitch_scale_at(pitch_config, 1.0);
    if (!schedule.valid || source.samples.empty() || source.sample_rate <= 0 ||
        !(ps0 > 0.05 && ps0 < 8.0) || !(psm > 0.05 && psm < 8.0) ||
        !(ps1 > 0.05 && ps1 < 8.0) ||
        !(tm.left > 0.05 && tm.left < 8.0 && tm.right > 0.05 && tm.right < 8.0)) {
        if (diagnostics) *diagnostics = d;
        return out;
    }
    if (std::abs(tm.left - tm.right) < 1e-12) {
        TdPsolaConfig c = pitch_config;
        c.duration_scale = tm.left;
        return td_psola_resynthesize(source, schedule, c, diagnostics);
    }

    const std::size_t target_len = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::llround(tm.target_len())));
    const int maxp = *std::max_element(schedule.periods.begin(), schedule.periods.end());
    std::vector<double> acc(target_len + static_cast<std::size_t>(2 * maxp + 8), 0.0);
    std::vector<double> wsum(acc.size(), 0.0);

    double synth = tm.map(static_cast<double>(schedule.marks.front()));
    const double synth_end = tm.map(static_cast<double>(schedule.marks.back()));
    std::vector<double> synth_marks;
    while (synth <= synth_end + 0.5) {
        const double source_time = tm.inverse(synth);
        const std::size_t mi = nearest_mark_index_m23(schedule.marks, source_time);
        const int p = std::max(8, local_period_m23(schedule, mi));
        const std::size_t src_center = schedule.marks[mi];
        const long dst_center = static_cast<long>(std::llround(synth));
        for (long rel = -p; rel <= p; ++rel) {
            const long si = static_cast<long>(src_center) + rel;
            const long di = dst_center + rel;
            if (si < 0 || di < 0 || si >= static_cast<long>(source.samples.size()) ||
                di >= static_cast<long>(acc.size())) continue;
            const double w = grain_hann_m23(rel, p);
            acc[static_cast<std::size_t>(di)] += w * source.samples[static_cast<std::size_t>(si)];
            wsum[static_cast<std::size_t>(di)] += w;
        }
        ++d.grains_added;
        synth_marks.push_back(synth);
        const double denom = std::max(1.0,
            static_cast<double>(schedule.marks.back() - schedule.marks.front()));
        const double pos = std::clamp(
            (source_time - static_cast<double>(schedule.marks.front())) / denom, 0.0, 1.0);
        const double local_pitch_scale = td_psola_pitch_scale_at(pitch_config, pos);
        synth += std::max(4.0, static_cast<double>(p) / local_pitch_scale);
    }

    out.samples.resize(target_len, 0);
    for (std::size_t i = 0; i < target_len; ++i) {
        if (wsum[i] > 1e-8) out.samples[i] = clip16(acc[i] / wsum[i]);
        else {
            ++d.uncovered_samples;
            const auto j = std::min<std::size_t>(source.samples.size() - 1,
                static_cast<std::size_t>(std::floor(tm.inverse(static_cast<double>(i)))));
            out.samples[i] = source.samples[j];
        }
    }
    if (pitch_config.blend_uncovered_edges_m40) {
        std::vector<std::int16_t> mapped(target_len);
        for (std::size_t i = 0; i < target_len; ++i) {
            const auto j = std::min<std::size_t>(source.samples.size() - 1,
                static_cast<std::size_t>(std::floor(tm.inverse(static_cast<double>(i)))));
            mapped[i] = source.samples[j];
        }
        blend_uncovered_edges_m40(out, mapped, wsum,
            std::min<std::size_t>(32, static_cast<std::size_t>(maxp)));
    }
    for (std::size_t i = 1; pitch_config.audit_transients_m40 && i < out.samples.size(); ++i) {
        const int step = std::abs(static_cast<int>(out.samples[i]) - out.samples[i - 1]);
        if (step > d.max_output_step) {
            d.max_output_step = step;
            d.max_output_step_at = i;
            d.max_step_weight_before = wsum[i - 1];
            d.max_step_weight_after = wsum[i];
        }
    }
    d.synthesis_marks = synth_marks.size();
    d.output_samples = out.samples.size();
    if (synth_marks.size() > 1)
        d.mean_target_period = (synth_marks.back() - synth_marks.front()) /
                               static_cast<double>(synth_marks.size() - 1);
    d.valid = !out.samples.empty() && d.grains_added > 0;
    if (diagnostics) *diagnostics = d;
    return out;
}

void apply_phone_side_energy_m32(
    Pcm16Mono& pcm, double split_fraction, double left_gain, double right_gain) {
    if (pcm.samples.empty() ||
        (std::abs(left_gain - 1.0) < 1e-12 && std::abs(right_gain - 1.0) < 1e-12)) return;
    split_fraction = std::clamp(split_fraction, 0.0, 1.0);
    // A boundary unit can have no support on one side. In that case do not
    // manufacture a transition from the absent phone's gain.
    if (split_fraction <= 0.0) left_gain = right_gain;
    if (split_fraction >= 1.0) right_gain = left_gain;
    const std::size_t split = static_cast<std::size_t>(std::llround(
        split_fraction * static_cast<double>(pcm.samples.size())));
    const std::size_t fade = std::min<std::size_t>(
        static_cast<std::size_t>(std::max(1, pcm.sample_rate / 200)),
        pcm.samples.size() / 8); // 5 ms at 16 kHz, bounded on short units
    const std::size_t lo = split > fade ? split - fade : 0;
    const std::size_t hi = std::min(pcm.samples.size(), split + fade);
    for (std::size_t i = 0; i < pcm.samples.size(); ++i) {
        double gain = left_gain;
        if (i >= hi) gain = right_gain;
        else if (i > lo && hi > lo) {
            const double t = static_cast<double>(i - lo) / static_cast<double>(hi - lo);
            gain = left_gain + (right_gain - left_gain) * t;
        }
        pcm.samples[i] = clip16(static_cast<double>(pcm.samples[i]) * gain);
    }
}

Pcm16Mono stretch_unvoiced_piecewise_m23(
    const Pcm16Mono& source,
    const PiecewiseDurationMapM23& tm) {
    if (std::abs(tm.left - tm.right) < 1e-12)
        return stretch_unvoiced_ola(source, tm.left);
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    if (source.samples.empty() || source.sample_rate <= 0 ||
        !(tm.left > 0.05 && tm.left < 8.0 && tm.right > 0.05 && tm.right < 8.0)) return out;
    const std::size_t target_len = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::llround(tm.target_len())));
    std::size_t frame = static_cast<std::size_t>(std::max(64, source.sample_rate / 100));
    frame = std::min(frame, source.samples.size());
    if (frame < 16) {
        out.samples.resize(target_len);
        for (std::size_t i = 0; i < target_len; ++i) {
            const auto j = std::min<std::size_t>(source.samples.size() - 1,
                static_cast<std::size_t>(std::floor(tm.inverse(static_cast<double>(i)))));
            out.samples[i] = source.samples[j];
        }
        return out;
    }
    const std::size_t analysis_hop = std::max<std::size_t>(1, frame / 2);
    std::vector<double> acc(target_len + frame + 8, 0.0), wsum(acc.size(), 0.0);
    constexpr double kPi = 3.141592653589793238462643383279502884;
    for (std::size_t src = 0; src < source.samples.size(); src += analysis_hop) {
        const std::size_t dst = static_cast<std::size_t>(std::llround(tm.map(static_cast<double>(src))));
        if (dst >= acc.size()) break;
        const std::size_t available = std::min(frame, source.samples.size() - src);
        for (std::size_t i = 0; i < available && dst + i < acc.size(); ++i) {
            const double t = frame > 1 ? static_cast<double>(i) / static_cast<double>(frame - 1) : 0.0;
            const double w = 0.5 - 0.5 * std::cos(2.0 * kPi * t);
            acc[dst + i] += w * source.samples[src + i];
            wsum[dst + i] += w;
        }
        if (source.samples.size() - src <= analysis_hop) break;
    }
    out.samples.resize(target_len, 0);
    for (std::size_t i = 0; i < target_len; ++i) {
        if (wsum[i] > 1e-8) out.samples[i] = clip16(acc[i] / wsum[i]);
        else {
            const auto j = std::min<std::size_t>(source.samples.size() - 1,
                static_cast<std::size_t>(std::floor(tm.inverse(static_cast<double>(i)))));
            out.samples[i] = source.samples[j];
        }
    }
    return out;
}

} // namespace

Pcm16Mono resynthesize_seg_m15(
    const Pcm16Mono& source,
    const SegScheduleM15& schedule,
    const SegSpanLayout& layout,
    const TdPsolaConfig& config,
    M15UnitDiagnostics* diagnostics) {

    Pcm16Mono out;
    out.sample_rate = source.sample_rate;
    if (!schedule.valid || !layout.valid || schedule.runs.size() != layout.runs.size() ||
        source.samples.empty()) return out;

    std::vector<Pcm16Mono> rendered;
    rendered.reserve(layout.runs.size());
    std::vector<int> left_hints, right_hints;

    for (std::size_t i = 0; i < layout.runs.size(); ++i) {
        const auto& span = layout.runs[i];
        auto raw = slice_pcm(source, span.source_begin, span.source_end);
        if (raw.samples.empty()) return {};

        M15RunDiagnostics rd;
        rd.voiced = span.voiced;
        rd.slots = span.slots;
        rd.source_begin = span.source_begin;
        rd.source_end = span.source_end;
        rd.periods = span.periods;
        rd.signed_period_reset = span.signed_period_reset;

        Pcm16Mono r;
        if (span.voiced) {
            auto ps = make_run_pitch_schedule(span, raw.samples.size());
            TdPsolaConfig run_config = config;
            if (config.use_three_point_pitch && !source.samples.empty()) {
                const double den = static_cast<double>(source.samples.size());
                const double a = static_cast<double>(span.source_begin) / den;
                const double b = static_cast<double>(span.source_end) / den;
                const double m = 0.5 * (a + b);
                run_config.use_three_point_pitch = true;
                run_config.pitch_scale_start = td_psola_pitch_scale_at(config, a);
                run_config.pitch_scale_mid = td_psola_pitch_scale_at(config, m);
                run_config.pitch_scale_end = td_psola_pitch_scale_at(config, b);
                run_config.pitch_scale = run_config.pitch_scale_mid;
            }
            r = td_psola_resynthesize(raw, ps, run_config, &rd.psola);
            if (rd.psola.valid && !r.samples.empty()) {
                rd.used_td_psola = true;
            } else {
                // Structural parser succeeded, but if a pathological tiny run
                // cannot support a grain, preserve audio rather than dropping it.
                r = stretch_unvoiced_ola(raw, config.duration_scale);
            }
        } else {
            r = stretch_unvoiced_ola(raw, config.duration_scale);
            rd.used_unvoiced_stretch = !r.samples.empty();
        }
        if (r.samples.empty()) return {};
        rd.output_samples = r.samples.size();
        rendered.push_back(std::move(r));
        left_hints.push_back(first_period(span));
        right_hints.push_back(last_period(span));
        if (diagnostics) diagnostics->runs.push_back(std::move(rd));
    }

    out = rendered.front();
    for (std::size_t i = 1; i < rendered.size(); ++i) {
        OlaJoinDiagnostics jd;
        const std::size_t before = out.samples.size();
        const double lps = td_psola_pitch_scale_at(config, 1.0);
        const double rps = td_psola_pitch_scale_at(config, 0.0);
        const int lp = right_hints[i - 1] > 0
            ? static_cast<int>(std::lround(right_hints[i - 1] / lps)) : 0;
        const int rp = left_hints[i] > 0
            ? static_cast<int>(std::lround(left_hints[i] / rps)) : 0;
        out = hann_ola_join(out, rendered[i], lp, rp, &jd, config.search_join_phase);
        if (!jd.valid) return {};
        if (diagnostics) {
            diagnostics->internal_joins.push_back(jd);
            diagnostics->internal_join_centers.push_back(
                before - std::min(before, jd.left_trim + jd.overlap_samples / 2));
        }
    }
    return out;
}


Pcm16Mono resynthesize_seg_m23_phone_sides(
    const Pcm16Mono& source,
    const SegScheduleM15& schedule,
    const SegSpanLayout& layout,
    double pitch_scale,
    double left_duration_scale,
    double right_duration_scale,
    M15UnitDiagnostics* diagnostics) {

    TdPsolaConfig c;
    c.pitch_scale = pitch_scale;
    return resynthesize_seg_m32_phone_sides(
        source, schedule, layout, c,
        left_duration_scale, right_duration_scale, 1.0, 1.0, diagnostics);
}

Pcm16Mono resynthesize_seg_m32_phone_sides(
    const Pcm16Mono& source,
    const SegScheduleM15& schedule,
    const SegSpanLayout& layout,
    const TdPsolaConfig& pitch_config,
    double left_duration_scale,
    double right_duration_scale,
    double left_energy_gain,
    double right_energy_gain,
    M15UnitDiagnostics* diagnostics) {

    const bool equal_duration = std::abs(left_duration_scale - right_duration_scale) < 1e-12;
    const bool unity_energy = std::abs(left_energy_gain - 1.0) < 1e-12 &&
                              std::abs(right_energy_gain - 1.0) < 1e-12;
    if (equal_duration) {
        TdPsolaConfig c = pitch_config;
        c.duration_scale = left_duration_scale;
        auto pcm = resynthesize_seg_m15(source, schedule, layout, c, diagnostics);
        if (!pcm.samples.empty() && !unity_energy) {
            const double split_fraction = source.samples.empty() ? 0.5 :
                static_cast<double>(std::min(layout.split_sample_estimate, source.samples.size())) /
                static_cast<double>(source.samples.size());
            apply_phone_side_energy_m32(
                pcm, split_fraction, left_energy_gain, right_energy_gain);
        }
        return pcm;
    }
    Pcm16Mono out; out.sample_rate = source.sample_rate;
    if (!schedule.valid || !layout.valid || schedule.runs.size() != layout.runs.size() ||
        source.samples.empty() || !(left_energy_gain > 0.0 && left_energy_gain < 8.0) ||
        !(right_energy_gain > 0.0 && right_energy_gain < 8.0)) return out;
    const double ps0 = td_psola_pitch_scale_at(pitch_config, 0.0);
    const double psm = td_psola_pitch_scale_at(pitch_config, 0.5);
    const double ps1 = td_psola_pitch_scale_at(pitch_config, 1.0);
    if (!(ps0 > 0.05 && ps0 < 8.0) || !(psm > 0.05 && psm < 8.0) ||
        !(ps1 > 0.05 && ps1 < 8.0)) return out;

    std::vector<Pcm16Mono> rendered;
    std::vector<int> left_hints, right_hints;
    rendered.reserve(layout.runs.size());
    for (std::size_t i = 0; i < layout.runs.size(); ++i) {
        const auto& span = layout.runs[i];
        auto raw = slice_pcm(source, span.source_begin, span.source_end);
        if (raw.samples.empty()) return {};

        const std::size_t run_len = span.source_end - span.source_begin;
        double local_split = 0.0;
        double ls = left_duration_scale, rs = right_duration_scale;
        if (layout.split_sample_estimate <= span.source_begin) {
            local_split = 0.0; ls = rs = right_duration_scale;
        } else if (layout.split_sample_estimate >= span.source_end) {
            local_split = static_cast<double>(run_len); ls = rs = left_duration_scale;
        } else {
            local_split = static_cast<double>(layout.split_sample_estimate - span.source_begin);
        }
        PiecewiseDurationMapM23 tm{local_split, ls, rs, static_cast<double>(run_len)};

        M15RunDiagnostics rd;
        rd.voiced = span.voiced; rd.slots = span.slots;
        rd.source_begin = span.source_begin; rd.source_end = span.source_end;
        rd.periods = span.periods; rd.signed_period_reset = span.signed_period_reset;
        Pcm16Mono r;
        if (span.voiced) {
            auto ps = make_run_pitch_schedule(span, raw.samples.size());
            TdPsolaConfig run_config = pitch_config;
            if (pitch_config.use_three_point_pitch && !source.samples.empty()) {
                const double den = static_cast<double>(source.samples.size());
                const double a = static_cast<double>(span.source_begin) / den;
                const double b = static_cast<double>(span.source_end) / den;
                const double m = 0.5 * (a + b);
                run_config.pitch_scale_start = td_psola_pitch_scale_at(pitch_config, a);
                run_config.pitch_scale_mid = td_psola_pitch_scale_at(pitch_config, m);
                run_config.pitch_scale_end = td_psola_pitch_scale_at(pitch_config, b);
                run_config.pitch_scale = run_config.pitch_scale_mid;
            }
            r = td_psola_piecewise_m23(raw, ps, run_config, tm, &rd.psola);
            if (rd.psola.valid && !r.samples.empty()) rd.used_td_psola = true;
            else r = stretch_unvoiced_piecewise_m23(raw, tm);
        } else {
            r = stretch_unvoiced_piecewise_m23(raw, tm);
            rd.used_unvoiced_stretch = !r.samples.empty();
        }
        if (r.samples.empty()) return {};
        rd.output_samples = r.samples.size();
        rendered.push_back(std::move(r));
        left_hints.push_back(first_period(span));
        right_hints.push_back(last_period(span));
        if (diagnostics) diagnostics->runs.push_back(std::move(rd));
    }
    out = rendered.front();
    for (std::size_t i = 1; i < rendered.size(); ++i) {
        OlaJoinDiagnostics jd;
        const std::size_t before = out.samples.size();
        const double den = std::max(1.0, static_cast<double>(source.samples.size()));
        const double left_pos = static_cast<double>(layout.runs[i - 1].source_end) / den;
        const double right_pos = static_cast<double>(layout.runs[i].source_begin) / den;
        const double left_pitch = td_psola_pitch_scale_at(pitch_config, left_pos);
        const double right_pitch = td_psola_pitch_scale_at(pitch_config, right_pos);
        const int lp = right_hints[i - 1] > 0 ?
            static_cast<int>(std::lround(right_hints[i - 1] / left_pitch)) : 0;
        const int rp = left_hints[i] > 0 ?
            static_cast<int>(std::lround(left_hints[i] / right_pitch)) : 0;
        out = hann_ola_join(out, rendered[i], lp, rp, &jd, pitch_config.search_join_phase);
        if (!jd.valid) return {};
        if (diagnostics) {
            diagnostics->internal_joins.push_back(jd);
            diagnostics->internal_join_centers.push_back(
                before - std::min(before, jd.left_trim + jd.overlap_samples / 2));
        }
    }
    const PiecewiseDurationMapM23 unit_tm{
        static_cast<double>(std::min(layout.split_sample_estimate, source.samples.size())),
        left_duration_scale, right_duration_scale, static_cast<double>(source.samples.size())};
    const double split_fraction = unit_tm.target_len() > 1e-12
        ? unit_tm.map(unit_tm.split) / unit_tm.target_len() : 0.5;
    apply_phone_side_energy_m32(out, split_fraction, left_energy_gain, right_energy_gain);
    return out;
}

DiphoneChainM15Result synthesize_diphone_chain_m15(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    const TdPsolaConfig& config,
    int sample_rate) {

    DiphoneChainM15Result out;
    if (!catalog.valid) { out.error = "invalid_catalog"; return out; }
    if (phones.size() < 2) { out.error = "need_at_least_two_phones"; return out; }
    if (sample_rate <= 0) { out.error = "invalid_sample_rate"; return out; }

    std::vector<Pcm16Mono> rendered;
    std::vector<int> left_hints, right_hints;
    rendered.reserve(phones.size() - 1);

    for (std::size_t i = 0; i + 1 < phones.size(); ++i) {
        const auto* unit = find_diphone(catalog, phones[i], phones[i + 1]);
        if (!unit) { out.error = "missing_diphone_" + phones[i] + "_" + phones[i + 1]; return out; }
        const auto encoded = extract_diphone_compressed_bytes(database_bytes, catalog, *unit);
        if (encoded.empty()) { out.error = "empty_diphone"; return out; }
        auto raw = decode_g711_alaw_pcm(encoded, sample_rate);
        if (raw.samples.empty()) { out.error = "decode_failed"; return out; }

        M15UnitDiagnostics ud;
        ud.label = phones[i] + "->" + phones[i + 1];
        ud.schedule = parse_seg_schedule_m15(*unit);
        if (!ud.schedule.valid) { out.error = "seg_parse_" + ud.schedule.error; return out; }
        ud.layout = layout_seg_runs_m15(ud.schedule, raw.samples.size());
        if (!ud.layout.valid) { out.error = "seg_layout_" + ud.layout.error; return out; }
        if (!ud.layout.runs.empty()) {
            ud.left_period_hint = first_period(ud.layout.runs.front());
            ud.right_period_hint = last_period(ud.layout.runs.back());
        }

        auto pcm = resynthesize_seg_m15(raw, ud.schedule, ud.layout, config, &ud);
        if (pcm.samples.empty()) { out.error = "seg_render_failed"; return out; }
        rendered.push_back(std::move(pcm));
        left_hints.push_back(ud.left_period_hint);
        right_hints.push_back(ud.right_period_hint);
        out.diphone_labels.push_back(ud.label);
        out.units.push_back(std::move(ud));
    }

    out.pcm = rendered.front();
    for (std::size_t i = 1; i < rendered.size(); ++i) {
        OlaJoinDiagnostics jd;
        const double lps = td_psola_pitch_scale_at(config, 1.0);
        const double rps = td_psola_pitch_scale_at(config, 0.0);
        const int lp = right_hints[i - 1] > 0
            ? static_cast<int>(std::lround(right_hints[i - 1] / lps)) : 0;
        const int rp = left_hints[i] > 0
            ? static_cast<int>(std::lround(left_hints[i] / rps)) : 0;
        out.pcm = hann_ola_join(out.pcm, rendered[i], lp, rp, &jd, config.search_join_phase);
        if (!jd.valid) { out.error = "boundary_join_failed"; return out; }
        out.joins.push_back(jd);
    }
    out.valid = true;
    return out;
}

} // namespace nicolai
