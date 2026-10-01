#include "nicolai/psola_join.hpp"
#include "nicolai/g711.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>

namespace nicolai {
namespace {

int median(std::vector<int> v) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    const auto n = v.size();
    if (n & 1u) return v[n / 2];
    return (v[n / 2 - 1] + v[n / 2]) / 2;
}

int edge_median(const std::vector<int>& p, bool right) {
    if (p.empty()) return 0;
    const std::size_t n = std::min<std::size_t>(3, p.size());
    std::vector<int> v;
    v.reserve(n);
    if (right) {
        v.insert(v.end(), p.end() - static_cast<std::ptrdiff_t>(n), p.end());
    } else {
        v.insert(v.end(), p.begin(), p.begin() + static_cast<std::ptrdiff_t>(n));
    }
    return median(std::move(v));
}

// Pearson-style correlation after DC removal. Used only to align the shared
// voiced phone within a bounded fraction of one pitch period.
double correlation(const std::vector<std::int16_t>& a,
                   std::size_t ao,
                   const std::vector<std::int16_t>& b,
                   std::size_t bo,
                   std::size_t n) {
    if (n < 2 || ao + n > a.size() || bo + n > b.size()) return -2.0;
    double ma = 0.0, mb = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        ma += a[ao + i];
        mb += b[bo + i];
    }
    ma /= static_cast<double>(n);
    mb /= static_cast<double>(n);
    double num = 0.0, da = 0.0, db = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double x = static_cast<double>(a[ao + i]) - ma;
        const double y = static_cast<double>(b[bo + i]) - mb;
        num += x * y;
        da += x * x;
        db += y * y;
    }
    if (da <= 1e-12 || db <= 1e-12) return 0.0;
    return num / std::sqrt(da * db);
}

std::int16_t clip16(double v) {
    v = std::max(-32768.0, std::min(32767.0, v));
    return static_cast<std::int16_t>(std::lrint(v));
}

bool threatened_unvoiced_burst(const std::vector<std::int16_t>& pcm,
                              std::size_t begin, std::size_t count,
                              std::size_t trim, std::size_t overlap,
                              bool right_side, int sample_rate) {
    if (count == 0 || begin+count > pcm.size()) return false;
    const auto window = std::min(count, static_cast<std::size_t>(std::max(8, sample_rate/1000)));
    double total=0.0, energy=0.0, best=0.0;
    int peak=0;
    std::size_t best_begin=0;
    for (std::size_t i=0;i<count;++i) {
        const double value=pcm[begin+i];
        total+=value*value;
        peak=std::max(peak,std::abs(static_cast<int>(pcm[begin+i])));
        energy+=value*value;
        if (i>=window) {
            const double old=pcm[begin+i-window];
            energy-=old*old;
        }
        if (i+1>=window && energy>best) { best=energy; best_begin=i+1-window; }
    }
    // A one-millisecond pulse dominates this edge; stationary fricative noise
    // does not satisfy the concentration test. Only protect it if the old
    // trim/window would attenuate the center by at least 25 percent.
    if (peak<1500 || total<=0.0 || best<0.55*total) return false;
    const double center=static_cast<double>(best_begin)+0.5*static_cast<double>(window-1);
    double retained=0.0;
    constexpr double kPi=3.14159265358979323846;
    if (right_side && center>=static_cast<double>(trim)) {
        const double t=std::clamp((center-static_cast<double>(trim))/std::max<std::size_t>(1,overlap-1),0.0,1.0);
        retained=0.5-0.5*std::cos(kPi*t);
    } else if (!right_side && center<static_cast<double>(overlap)) {
        const double t=center/std::max<std::size_t>(1,overlap-1);
        retained=0.5+0.5*std::cos(kPi*t);
    }
    return retained<0.75;
}

} // namespace

PitchPeriodHint infer_pitch_period_hint(const DiphoneUnit& unit,
                                        int min_period,
                                        int max_period) {
    PitchPeriodHint out;
    if (min_period <= 0 || max_period < min_period) return out;

    // SEG records begin with four control words.  In fully voiced records,
    // every remaining word is a pitch-period-like duration. Mixed records also
    // contain negative/control words; keep only physically plausible positive
    // periods for the M13 boundary estimator.
    for (std::size_t i = std::min<std::size_t>(4, unit.metadata.size());
         i < unit.metadata.size(); ++i) {
        const int v = unit.metadata[i];
        if (v >= min_period && v <= max_period) out.periods.push_back(v);
    }
    if (out.periods.empty()) return out;
    out.left_period = edge_median(out.periods, false);
    out.right_period = edge_median(out.periods, true);
    out.valid = out.left_period > 0 && out.right_period > 0;
    return out;
}

Pcm16Mono hann_ola_join(const Pcm16Mono& left,
                        const Pcm16Mono& right,
                        int left_period_hint,
                        int right_period_hint,
                        OlaJoinDiagnostics* diagnostics, bool search_phase,
                        bool preserve_unvoiced_m41) {
    Pcm16Mono out;
    out.sample_rate = left.sample_rate;
    OlaJoinDiagnostics d;
    d.left_period_hint = left_period_hint;
    d.right_period_hint = right_period_hint;

    if (left.sample_rate <= 0 || left.sample_rate != right.sample_rate ||
        left.samples.empty() || right.samples.empty()) {
        if (diagnostics) *diagnostics = d;
        return out;
    }

    int lp = left_period_hint;
    int rp = right_period_hint;
    // For unvoiced/unknown edges, use a short 5 ms transition rather than
    // inventing a pitch period.
    const int fallback = std::max(16, left.sample_rate / 200);
    if (lp <= 0) lp = fallback;
    if (rp <= 0) rp = fallback;

    std::size_t overlap = static_cast<std::size_t>(std::max(16, (lp + rp) / 2));
    overlap = std::min(overlap, left.samples.size() / 3);
    overlap = std::min(overlap, right.samples.size() / 3);
    if (overlap < 8) {
        out.samples = left.samples;
        out.samples.insert(out.samples.end(), right.samples.begin(), right.samples.end());
        d.valid = true;
        if (diagnostics) *diagnostics = d;
        return out;
    }

    // Diphones meet at the midpoint of their shared phone. Search only within
    // half a pitch period on either side so we can phase-align voiced joins
    // without throwing away arbitrary amounts of the unit.
    const std::size_t max_lt = search_phase ?
        std::min<std::size_t>(std::max(0, lp / 2), left.samples.size() - overlap) : 0;
    const std::size_t max_rt = search_phase ?
        std::min<std::size_t>(std::max(0, rp / 2), right.samples.size() - overlap) : 0;

    double best = -3.0;
    std::size_t best_lt = 0, best_rt = 0;
    for (std::size_t lt = 0; lt <= max_lt; ++lt) {
        const std::size_t ao = left.samples.size() - lt - overlap;
        for (std::size_t rt = 0; rt <= max_rt; ++rt) {
            const double c = correlation(left.samples, ao, right.samples, rt, overlap);
            if (c > best) {
                best = c;
                best_lt = lt;
                best_rt = rt;
            }
        }
    }

    if (preserve_unvoiced_m41) {
        const bool left_burst=left_period_hint<=0 && threatened_unvoiced_burst(
            left.samples,left.samples.size()-best_lt-overlap,best_lt+overlap,
            best_lt,overlap,false,left.sample_rate);
        const bool right_burst=right_period_hint<=0 && threatened_unvoiced_burst(
            right.samples,0,best_rt+overlap,best_rt,overlap,true,right.sample_rate);
        if (left_burst || right_burst) {
            d.protected_transient_m41=true;
            best_lt=best_rt=0;
            overlap=std::min(overlap,static_cast<std::size_t>(std::max(8,left.sample_rate/1000)));
            best=correlation(left.samples,left.samples.size()-overlap,right.samples,0,overlap);
        }
    }

    const std::size_t left_end = left.samples.size() - best_lt;
    const std::size_t left_overlap_begin = left_end - overlap;
    out.samples.reserve(left.samples.size() + right.samples.size() - overlap - best_lt - best_rt);
    out.samples.insert(out.samples.end(), left.samples.begin(), left.samples.begin() + static_cast<std::ptrdiff_t>(left_overlap_begin));

    // Complementary half-Hann (raised-cosine) windows. w_in goes 0->1 and
    // w_out = 1-w_in, so DC gain stays exactly one through the overlap.
    constexpr double kPi = 3.14159265358979323846;
    for (std::size_t i = 0; i < overlap; ++i) {
        const double t = overlap > 1 ? static_cast<double>(i) / static_cast<double>(overlap - 1) : 1.0;
        const double w_in = 0.5 - 0.5 * std::cos(kPi * t);
        const double w_out = 1.0 - w_in;
        const double a = left.samples[left_overlap_begin + i];
        const double b = right.samples[best_rt + i];
        out.samples.push_back(clip16(w_out * a + w_in * b));
    }
    out.samples.insert(out.samples.end(),
                       right.samples.begin() + static_cast<std::ptrdiff_t>(best_rt + overlap),
                       right.samples.end());

    d.valid = true;
    d.overlap_samples = overlap;
    d.left_trim = best_lt;
    d.right_trim = best_rt;
    d.normalized_correlation = best;
    if (diagnostics) *diagnostics = d;
    return out;
}

DiphoneChainResult synthesize_diphone_chain_m13(
    const std::vector<std::uint8_t>& database_bytes,
    const DiphoneCatalog& catalog,
    const std::vector<std::string>& phones,
    int sample_rate) {

    DiphoneChainResult out;
    if (!catalog.valid) { out.error = "invalid_catalog"; return out; }
    if (phones.size() < 2) { out.error = "need_at_least_two_phones"; return out; }
    if (sample_rate <= 0) { out.error = "invalid_sample_rate"; return out; }

    std::vector<Pcm16Mono> decoded;
    std::vector<PitchPeriodHint> hints;
    decoded.reserve(phones.size() - 1);
    hints.reserve(phones.size() - 1);

    for (std::size_t i = 0; i + 1 < phones.size(); ++i) {
        const auto* u = find_diphone(catalog, phones[i], phones[i + 1]);
        if (!u) {
            out.error = "missing_diphone_" + phones[i] + "_" + phones[i + 1];
            return out;
        }
        const auto encoded = extract_diphone_compressed_bytes(database_bytes, catalog, *u);
        if (encoded.empty()) {
            out.error = "empty_diphone_" + phones[i] + "_" + phones[i + 1];
            return out;
        }
        decoded.push_back(decode_g711_alaw_pcm(encoded, sample_rate));
        hints.push_back(infer_pitch_period_hint(*u));
        out.diphone_labels.push_back(phones[i] + "->" + phones[i + 1]);
    }

    out.pcm = decoded.front();
    for (std::size_t i = 1; i < decoded.size(); ++i) {
        OlaJoinDiagnostics d;
        const int lp = hints[i - 1].valid ? hints[i - 1].right_period : 0;
        const int rp = hints[i].valid ? hints[i].left_period : 0;
        out.pcm = hann_ola_join(out.pcm, decoded[i], lp, rp, &d);
        if (!d.valid) { out.error = "ola_join_failed"; return out; }
        out.joins.push_back(d);
    }
    out.valid = true;
    return out;
}

} // namespace nicolai
