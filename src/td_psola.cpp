#include "nicolai/td_psola.hpp"
#include "nicolai/g711.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>

namespace nicolai {
namespace {
constexpr double kPi = 3.141592653589793238462643383279502884;

std::int16_t clip16(double x) {
    x = std::max(-32768.0, std::min(32767.0, x));
    return static_cast<std::int16_t>(std::lrint(x));
}

double mean_period(const std::vector<int>& p) {
    if (p.empty()) return 0.0;
    return static_cast<double>(std::accumulate(p.begin(), p.end(), std::int64_t{0})) /
           static_cast<double>(p.size());
}

std::size_t nearest_mark_index(const std::vector<std::size_t>& marks, double x) {
    if (marks.empty()) return 0;
    const auto it = std::lower_bound(marks.begin(), marks.end(), static_cast<std::size_t>(std::max(0.0, x)));
    if (it == marks.begin()) return 0;
    if (it == marks.end()) return marks.size() - 1;
    const auto hi = static_cast<std::size_t>(it - marks.begin());
    const auto lo = hi - 1;
    return (x - static_cast<double>(marks[lo]) <= static_cast<double>(marks[hi]) - x) ? lo : hi;
}

int local_period(const SegPitchSchedule& s, std::size_t mark_index) {
    if (s.periods.empty()) return 0;
    if (mark_index == 0) return s.periods.front();
    if (mark_index >= s.periods.size()) return s.periods.back();
    return (s.periods[mark_index - 1] + s.periods[mark_index]) / 2;
}

// Symmetric Hann centered at zero. Radius P produces a 2P+1 grain.
double grain_hann(long rel, int radius) {
    if (radius <= 0 || std::abs(rel) > radius) return 0.0;
    const double x = static_cast<double>(rel + radius) / static_cast<double>(2 * radius);
    return 0.5 - 0.5 * std::cos(2.0 * kPi * x);
}

} // namespace

double td_psola_pitch_scale_at(const TdPsolaConfig& config, double position) {
    if (!config.use_three_point_pitch) return config.pitch_scale;
    const double x = std::clamp(position, 0.0, 1.0);
    if (x <= 0.5)
        return config.pitch_scale_start +
            (config.pitch_scale_mid - config.pitch_scale_start) * (x * 2.0);
    return config.pitch_scale_mid +
        (config.pitch_scale_end - config.pitch_scale_mid) * ((x - 0.5) * 2.0);
}

SegPitchSchedule parse_clean_voiced_seg_schedule(
    const DiphoneUnit& unit,
    std::size_t pcm_sample_count,
    int min_period,
    int max_period) {

    SegPitchSchedule out;
    if (unit.metadata.size() < 5) { out.error = "seg_too_short"; return out; }
    out.mode = unit.metadata[0];
    out.declared_periods = unit.metadata[1];
    out.split_index = unit.metadata[2];
    out.repeated_period_count = unit.metadata[3];

    // This is the exact clean voiced shape repeatedly observed in Nicolai.
    if (out.mode != 2) { out.error = "mixed_or_unvoiced_seg"; return out; }
    if (out.declared_periods <= 0 || out.repeated_period_count != out.declared_periods) {
        out.error = "period_count_header_mismatch"; return out;
    }
    if (out.split_index < 0 || out.split_index > out.declared_periods) {
        out.error = "invalid_split_index"; return out;
    }
    if (unit.metadata.size() != static_cast<std::size_t>(4 + out.declared_periods)) {
        out.error = "period_count_size_mismatch"; return out;
    }

    out.periods.reserve(static_cast<std::size_t>(out.declared_periods));
    std::uint64_t sum = 0;
    for (std::size_t i = 4; i < unit.metadata.size(); ++i) {
        const int p = unit.metadata[i];
        if (p < min_period || p > max_period) {
            out.error = "period_out_of_range"; return out;
        }
        out.periods.push_back(p);
        sum += static_cast<std::uint64_t>(p);
    }
    if (sum >= pcm_sample_count) { out.error = "periods_exceed_pcm"; return out; }

    // The legacy records leave approximately half-period margins at their
    // edges. Fit the measured period chain into the decoded PCM symmetrically;
    // this yields explicit pitch marks while preserving every SEG period.
    const auto slack = pcm_sample_count - static_cast<std::size_t>(sum);
    out.left_margin = slack / 2;
    out.right_margin = slack - out.left_margin;
    out.marks.reserve(out.periods.size() + 1);
    std::size_t pos = out.left_margin;
    out.marks.push_back(pos);
    for (const int p : out.periods) {
        pos += static_cast<std::size_t>(p);
        out.marks.push_back(pos);
    }
    if (out.marks.back() >= pcm_sample_count) {
        out.error = "last_mark_out_of_bounds"; return out;
    }
    out.valid = true;
    return out;
}

std::vector<std::int16_t> legacy_half_hann_q15(std::size_t period) {
    std::vector<std::int16_t> out;
    if (period == 0) return out;
    out.reserve(period + 1);
    for (std::size_t i = 0; i <= period; ++i) {
        const double w = 0.5 * (1.0 + std::cos(kPi * static_cast<double>(i) /
                                               static_cast<double>(period)));
        out.push_back(clip16(w * 32767.0));
    }
    return out;
}

Pcm16Mono td_psola_resynthesize(
    const Pcm16Mono& source,
    const SegPitchSchedule& schedule,
    const TdPsolaConfig& config,
    TdPsolaDiagnostics* diagnostics) {

    TdPsolaDiagnostics d;
    d.pitch_scale = config.pitch_scale;
    d.duration_scale = config.duration_scale;
    d.source_marks = schedule.marks.size();
    d.mean_source_period = mean_period(schedule.periods);

    Pcm16Mono out;
    out.sample_rate = source.sample_rate;
    const double ps0 = td_psola_pitch_scale_at(config, 0.0);
    const double psm = td_psola_pitch_scale_at(config, 0.5);
    const double ps1 = td_psola_pitch_scale_at(config, 1.0);
    if (!schedule.valid || source.samples.empty() || source.sample_rate <= 0 ||
        !(ps0 > 0.05 && ps0 < 8.0) || !(psm > 0.05 && psm < 8.0) ||
        !(ps1 > 0.05 && ps1 < 8.0) ||
        !(config.duration_scale > 0.05 && config.duration_scale < 8.0)) {
        if (diagnostics) *diagnostics = d;
        return out;
    }

    const std::size_t target_len = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::llround(source.samples.size() * config.duration_scale)));
    // A little guard space allows the final Hann grain to land cleanly; it is
    // cropped back to target_len after normalized OLA.
    const int maxp = *std::max_element(schedule.periods.begin(), schedule.periods.end());
    std::vector<double> acc(target_len + static_cast<std::size_t>(2 * maxp + 8), 0.0);
    std::vector<double> wsum(acc.size(), 0.0);

    double synth = static_cast<double>(schedule.marks.front()) * config.duration_scale;
    const double synth_end = static_cast<double>(schedule.marks.back()) * config.duration_scale;
    std::vector<double> synth_marks;

    while (synth <= synth_end + 0.5) {
        const double source_time = synth / config.duration_scale;
        const std::size_t mi = nearest_mark_index(schedule.marks, source_time);
        const int p = std::max(8, local_period(schedule, mi));
        const std::size_t src_center = schedule.marks[mi];
        const long dst_center = static_cast<long>(std::llround(synth));

        // Classic TD-PSOLA grain: approximately two pitch periods wide,
        // Hann-windowed and centered at the source pitch mark.
        for (long rel = -p; rel <= p; ++rel) {
            const long si = static_cast<long>(src_center) + rel;
            const long di = dst_center + rel;
            if (si < 0 || di < 0 || si >= static_cast<long>(source.samples.size()) ||
                di >= static_cast<long>(acc.size())) continue;
            const double w = grain_hann(rel, p);
            acc[static_cast<std::size_t>(di)] += w * source.samples[static_cast<std::size_t>(si)];
            wsum[static_cast<std::size_t>(di)] += w;
        }
        ++d.grains_added;
        synth_marks.push_back(synth);

        // Pitch controls synthesis-mark spacing; duration controls how far
        // source time advances along the unit. Keeping them independent is the
        // defining TD-PSOLA behavior we need from Tempo.
        const double denom = std::max(1.0,
            static_cast<double>(schedule.marks.back() - schedule.marks.front()));
        const double pos = std::clamp(
            (source_time - static_cast<double>(schedule.marks.front())) / denom, 0.0, 1.0);
        const double local_pitch_scale = td_psola_pitch_scale_at(config, pos);
        double hop = static_cast<double>(p) / local_pitch_scale;
        hop = std::max(4.0, hop);
        synth += hop;
    }

    out.samples.resize(target_len, 0);
    for (std::size_t i = 0; i < target_len; ++i) {
        if (wsum[i] > 1e-8) {
            out.samples[i] = clip16(acc[i] / wsum[i]);
        } else {
            // Preserve uncovered edge samples by simple time mapping rather
            // than introducing digital silence.
            const double src = static_cast<double>(i) / config.duration_scale;
            const auto j = std::min<std::size_t>(source.samples.size() - 1,
                static_cast<std::size_t>(std::max(0.0, std::floor(src))));
            out.samples[i] = source.samples[j];
        }
    }

    d.synthesis_marks = synth_marks.size();
    d.output_samples = out.samples.size();
    if (synth_marks.size() > 1) {
        d.mean_target_period = (synth_marks.back() - synth_marks.front()) /
                               static_cast<double>(synth_marks.size() - 1);
    }
    d.valid = !out.samples.empty() && d.grains_added > 0;
    if (diagnostics) *diagnostics = d;
    return out;
}

DiphoneChainM14Result synthesize_diphone_chain_m14(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    const TdPsolaConfig& config,
    int sample_rate) {

    DiphoneChainM14Result out;
    if (!catalog.valid) { out.error = "invalid_catalog"; return out; }
    if (phones.size() < 2) { out.error = "need_at_least_two_phones"; return out; }
    if (sample_rate <= 0) { out.error = "invalid_sample_rate"; return out; }

    std::vector<Pcm16Mono> units_pcm;
    std::vector<PitchPeriodHint> edge_hints;
    units_pcm.reserve(phones.size() - 1);
    edge_hints.reserve(phones.size() - 1);

    for (std::size_t i = 0; i + 1 < phones.size(); ++i) {
        const auto* unit = find_diphone(catalog, phones[i], phones[i + 1]);
        if (!unit) { out.error = "missing_diphone_" + phones[i] + "_" + phones[i + 1]; return out; }
        const auto encoded = extract_diphone_compressed_bytes(database_bytes, catalog, *unit);
        if (encoded.empty()) { out.error = "empty_diphone"; return out; }
        auto pcm = decode_g711_alaw_pcm(encoded, sample_rate);
        if (pcm.samples.empty()) { out.error = "decode_failed"; return out; }

        M14UnitDiagnostics ud;
        ud.label = phones[i] + "->" + phones[i + 1];
        ud.schedule = parse_clean_voiced_seg_schedule(*unit, pcm.samples.size());
        if (ud.schedule.valid) {
            auto r = td_psola_resynthesize(pcm, ud.schedule, config, &ud.psola);
            if (ud.psola.valid && !r.samples.empty()) {
                pcm = std::move(r);
                ud.used_td_psola = true;
            }
        }
        units_pcm.push_back(std::move(pcm));

        auto h = infer_pitch_period_hint(*unit);
        if (h.valid && config.pitch_scale > 0.0) {
            h.left_period = static_cast<int>(std::lround(h.left_period / config.pitch_scale));
            h.right_period = static_cast<int>(std::lround(h.right_period / config.pitch_scale));
        }
        edge_hints.push_back(std::move(h));
        out.diphone_labels.push_back(ud.label);
        out.units.push_back(std::move(ud));
    }

    out.pcm = units_pcm.front();
    for (std::size_t i = 1; i < units_pcm.size(); ++i) {
        OlaJoinDiagnostics jd;
        const int lp = edge_hints[i - 1].valid ? edge_hints[i - 1].right_period : 0;
        const int rp = edge_hints[i].valid ? edge_hints[i].left_period : 0;
        out.pcm = hann_ola_join(out.pcm, units_pcm[i], lp, rp, &jd);
        if (!jd.valid) { out.error = "boundary_join_failed"; return out; }
        out.joins.push_back(jd);
    }
    out.valid = true;
    return out;
}

} // namespace nicolai
