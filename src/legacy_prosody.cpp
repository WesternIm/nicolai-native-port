#include "nicolai/legacy_prosody.hpp"
#include "nicolai/stateful_tds.hpp"
#include "nicolai/address_space.hpp"
#include "nicolai/g711.hpp"
#include "nicolai/static_files.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <unordered_set>

namespace nicolai {
namespace {
std::pair<int,std::size_t> max_pcm_step(const Pcm16Mono& pcm){
    int best=0; std::size_t at=0;
    for(std::size_t i=1;i<pcm.samples.size();++i){
        const int step=std::abs(static_cast<int>(pcm.samples[i])-pcm.samples[i-1]);
        if(step>best){best=step;at=i;}
    }
    return {best,at};
}
std::uint32_t le32(const std::vector<std::uint8_t>&b,std::size_t o){if(o+4>b.size())return 0;return std::uint32_t(b[o])|(std::uint32_t(b[o+1])<<8)|(std::uint32_t(b[o+2])<<16)|(std::uint32_t(b[o+3])<<24);}
std::string phone4(std::uint32_t x){char c[5]={char(x&255),char((x>>8)&255),char((x>>16)&255),char((x>>24)&255),0};return std::string(c);}
int first_period(const SegRunSpan&s){return s.voiced&&!s.periods.empty()?s.periods.front():0;}
int last_period(const SegRunSpan&s){return s.voiced&&!s.periods.empty()?s.periods.back():0;}
double robust_period_m23(const SegSpanLayout& layout){
    std::vector<int> periods;
    for(const auto& run:layout.runs) if(run.voiced)
        for(int p:run.periods) if(p>=20 && p<=800) periods.push_back(p);
    if(periods.empty()) return 0.0;
    const auto mid=periods.begin()+periods.size()/2;
    std::nth_element(periods.begin(),mid,periods.end());
    double med=*mid;
    if((periods.size()&1u)==0){
        const auto lo=std::max_element(periods.begin(),mid);
        if(lo!=mid) med=(med+*lo)*0.5;
    }
    return med;
}
bool legacy_phone_is_vowel_m23(const std::string& p){
    if(p.empty()||p=="#") return false;
    const char c=p[0];
    return c=='a'||c=='A'||c=='e'||c=='E'||c=='i'||c=='I'||
           c=='o'||c=='O'||c=='u'||c=='U'||c=='y'||c=='Y';
}

bool m24_phone_is_stressed_vowel(const std::string& p){
    return legacy_phone_is_vowel_m23(p) && !p.empty() && p.back()=='0';
}

double m24_declination_multiplier(const LegacyTimingPolicy& policy, double x){
    x=std::clamp(x,0.0,1.0);
    const double raw = x<=0.5
        ? policy.pitch_declination_start + (policy.pitch_declination_mid-policy.pitch_declination_start)*(x*2.0)
        : policy.pitch_declination_mid + (policy.pitch_declination_end-policy.pitch_declination_mid)*((x-0.5)*2.0);
    const double strength=std::clamp(policy.pitch_declination_strength,0.0,2.0);
    return std::pow(std::max(0.25,raw),strength);
}

// M25 sparse-anchor reconstruction. The interpolation mechanics mirror the
// recovered PC forward-anchor walk; only the still-unknown upstream anchor
// authoring values are reconstructed from general phonetic cues.
std::vector<double> m25_phone_pitch_percent_lattice(
    const std::vector<std::string>& phones, std::size_t word_count,
    const LegacyTimingPolicy& policy) {
    std::vector<double> out(phones.size(), 0.0);
    std::vector<std::size_t> active;
    for (std::size_t i=0;i<phones.size();++i) if (phones[i]!="#") active.push_back(i);
    if (active.empty()) return out;
    const std::size_t n=active.size();
    const double start=policy.pitch_anchor_start_percent +
        (word_count>1 ? policy.pitch_anchor_long_phrase_start_boost_percent : 0.0);
    const double end=policy.pitch_anchor_end_percent;
    std::vector<double> a(n,0.0);
    std::vector<bool> is_anchor(n,false);
    auto baseline=[&](std::size_t r){
        if(n<=1) return start;
        const double x=double(r)/double(n-1);
        return start+(end-start)*x;
    };
    a[0]=start; is_anchor[0]=true;
    if(n>1){a[n-1]=end; is_anchor[n-1]=true;}
    for(std::size_t r=1;r+1<n;++r){
        if(m24_phone_is_stressed_vowel(phones[active[r]])){
            a[r]=baseline(r)+policy.pitch_anchor_stress_lift_percent;
            is_anchor[r]=true;
        }
    }
    // Piecewise linear bridge between sparse anchors: equivalent to repeatedly
    // walking toward the next explicit anchor with a constant per-step slope.
    std::size_t left=0;
    while(left+1<n){
        std::size_t right=left+1;
        while(right<n && !is_anchor[right]) ++right;
        if(right>=n) right=n-1;
        const double av=a[left], bv=a[right];
        const std::size_t span=std::max<std::size_t>(1,right-left);
        for(std::size_t r=left;r<=right;++r){
            const double t=double(r-left)/double(span);
            a[r]=av+(bv-av)*t;
        }
        left=right;
    }
    for(std::size_t r=0;r<n;++r) out[active[r]]=a[r];
    // Boundary records do not carry voiced F0; copy the nearest spoken anchor
    // so diphones touching # do not invent an extra pitch discontinuity.
    for(std::size_t i=0;i<phones.size();++i){
        if(phones[i]!="#") continue;
        std::size_t l=i, r=i; bool hl=false,hr=false;
        while(l>0){--l; if(phones[l]!="#"){hl=true;break;}}
        while(r+1<phones.size()){++r; if(phones[r]!="#"){hr=true;break;}}
        if(hl&&hr) out[i]=0.5*(out[l]+out[r]);
        else if(hl) out[i]=out[l];
        else if(hr) out[i]=out[r];
    }
    return out;
}

double m25_anchor_f0(double percent, const LegacyWordProsodyProfile* wordstr,
                      const LegacyTimingPolicy& policy){
    const double base=policy.legacy_pitch_base_hz;
    double k=0.95;
    if(wordstr && wordstr->valid && wordstr->values.size()>28 &&
       std::isfinite(wordstr->values[28])) k=wordstr->values[28];
    const double authored=base*(1.0+0.01*percent);
    return base+(authored-base)*k; // exact downstream PC centering stage
}

bool m27_is_recovered_wh_word(const std::string& w){
    // Exact lexical family recovered from the three CP866 lookup strings at
    // 0x104e8808/0x104e88f0/0x104e89e0. The old table spells "что" as
    // phonetic "што"; accept the modern orthography preserved by M17 too.
    static const std::unordered_set<std::string> k = {
        "кто","кого","кому","кем","ком",
        "што","что","чего","чему","чем","чём",
        "сколько","сколь",
        "какой","какого","какому","каким","каком","какая","какую","какою","какое",
        "какие","каких","какими",
        "который","которого","которому","которым","котором","которая","которой",
        "которую","которое","которые","которых","которыми",
        "каков","какова","каково","каковы","как","зачем","почему","где","когда","неужели"
    };
    return k.find(w)!=k.end();
}

bool m27_has_recovered_wh_word(const RussianFrontendResult* frontend){
    if(!frontend) return false;
    for(const auto& w:frontend->words) if(m27_is_recovered_wh_word(w.source_utf8)) return true;
    return false;
}

int m27_terminal_physical_class(
    FrontendBoundaryKind kind, const LegacyTimingPolicy& policy,
    const RussianFrontendResult* frontend){
    if(policy.physical_terminal_class_override>=0 &&
       policy.physical_terminal_class_override<14)
        return policy.physical_terminal_class_override;

    const bool wh=m27_has_recovered_wh_word(frontend);
    // 0x10215de0 exact terminal classifier:
    //   "." and ";" -> declarative 1/2 (2 only when the proprietary <<<
    //   marker is present; M17 does not manufacture that marker),
    //   '?' + recovered WH lexicon -> 3, otherwise morphology class 7/8,
    //   "..." -> 6,
    //   remaining explicit terminal signs -> WH 5 / non-WH 4.
    switch(kind){
        case FrontendBoundaryKind::Question:
            if(wh) return 3;
            return std::clamp(policy.physical_nonwh_question_class,7,8);
        case FrontendBoundaryKind::Exclamation:
            return wh?5:4;
        case FrontendBoundaryKind::Sentence:
        case FrontendBoundaryKind::Semicolon:
            return 1;
        case FrontendBoundaryKind::Colon:
            return wh?5:4;
        // No explicit punctuation in the portable frontend is treated as the
        // legacy default declarative closure, matching prior PC corpus timing.
        case FrontendBoundaryKind::Word:
        case FrontendBoundaryKind::Hyphen:
        case FrontendBoundaryKind::Dash:
        case FrontendBoundaryKind::Other:
        default:
            return 1;
    }
}

int m27_internal_physical_class(FrontendBoundaryKind kind,
                                const LegacyTimingPolicy& policy){
    if(!policy.physical_use_punctuation_classes)
        return std::clamp(policy.physical_internal_class,0,13);
    // Direct string branches recovered from 0x10215de0:
    // ':' and ')' -> 10; ',' and '(' -> 12; plain word gaps are neutral.
    // Parentheses are not separately represented by M17 yet, but comma/colon
    // are, so those two rules can be ported without inventing metadata.
    switch(kind){
        case FrontendBoundaryKind::Comma: return 12;
        case FrontendBoundaryKind::Colon: return 10;
        default: return std::clamp(policy.physical_internal_class,0,13);
    }
}

struct M27PhysicalPitchLattice {
    std::vector<std::array<double,3>> pitch;
    // M27 broad approximation: k4 was projected onto the last active phone.
    std::vector<bool> terminal_proxy;
    // M28 proven standard orthographic empty stress marker (<...<>).
    std::vector<bool> empty_marker_exact;
    // M29 ordinary single-< stress marker attached to the stressed vowel.
    std::vector<bool> single_marker_exact;
};

M27PhysicalPitchLattice m27_physical_phone_pitch_lattice(
    const std::vector<std::string>& phones,
    const std::vector<FrontendBoundary>& boundaries,
    const LegacyPhysicalProsodyProfile* physical,
    const LegacyWordProsodyProfile* wordstr,
    const LegacyTimingPolicy& policy,
    const RussianFrontendResult* frontend){
    M27PhysicalPitchLattice result;
    result.pitch.assign(phones.size(), {0.0,0.0,0.0});
    result.terminal_proxy.assign(phones.size(), false);
    result.empty_marker_exact.assign(phones.size(), false);
    result.single_marker_exact.assign(phones.size(), false);
    auto& out=result.pitch;
    if(!physical || !physical->valid || physical->records.size()<14) return result;

    // Word phone spans are observable in the portable frontend.  M30 keeps
    // these separate from the *legacy authoring spans*: 0x10214f40 scans many
    // words until state+0x12c reaches the exact four-byte code "(/)" and only
    // then chooses one physical.int row from state+0x130[span_end].  M27-M29
    // accidentally treated each word as its own authoring span.
    struct WordSpan { std::size_t first=0,last=0,boundary=0; FrontendBoundaryKind kind=FrontendBoundaryKind::Word; };
    std::vector<WordSpan> words;
    std::size_t first=0;
    while(first<phones.size() && phones[first]=="#") ++first;
    for(const auto& b:boundaries){
        if(b.phone_index>=phones.size() || b.phone_index<first) continue;
        std::size_t last=b.phone_index;
        if(last>0 && phones[last]=="#") --last;
        if(last>=first) words.push_back({first,last,b.phone_index,b.kind});
        first=b.phone_index+1;
        while(first<phones.size() && phones[first]=="#") ++first;
    }
    if(words.empty()){
        std::vector<std::size_t> active;
        for(std::size_t i=0;i<phones.size();++i) if(phones[i]!="#") active.push_back(i);
        if(active.empty()) return result;
        words.push_back({active.front(),active.back(),phones.size()-1,FrontendBoundaryKind::Sentence});
    }

    struct AuthoringSpan { std::size_t first_word=0,last_word=0; };
    std::vector<AuthoringSpan> authoring;
    std::size_t span_first=0;
    for(std::size_t wi=0;wi<words.size();++wi){
        // In the PC state machine "(/)" is the hard terminator.  The portable
        // frontend cannot observe the dynamically generated code lattice, but
        // sentence/question/exclamation/semicolon are definite phrase-level
        // boundaries.  Comma/colon remain inside the same span, matching the
        // PC's separate internal physical classes.  The 22-phrase golden corpus
        // contains no case requiring the legacy >6-item dynamic splitter.
        const auto k=words[wi].kind;
        const bool hard=(k==FrontendBoundaryKind::Sentence ||
                         k==FrontendBoundaryKind::Question ||
                         k==FrontendBoundaryKind::Exclamation ||
                         k==FrontendBoundaryKind::Semicolon);
        if(hard){ authoring.push_back({span_first,wi}); span_first=wi+1; }
    }
    if(span_first<words.size()) authoring.push_back({span_first,words.size()-1});
    if(authoring.empty()) authoring.push_back({0,words.size()-1});

    const double wordstr24=(wordstr && wordstr->valid && wordstr->values.size()>24) ? wordstr->values[24] : 2.0;
    const double wordstr33=(wordstr && wordstr->valid && wordstr->values.size()>33) ? wordstr->values[33] : 20.0;
    const auto classes = frontend ? legacy_physical_class_lattice_m27(*frontend,policy) : std::vector<int>{};

    for(const auto& as:authoring){
        if(as.first_word>=words.size() || as.last_word>=words.size() || as.first_word>as.last_word) continue;
        const auto& end_word=words[as.last_word];
        const bool utterance_terminal=(as.last_word+1==words.size());

        // Exact M30 correction: all t/e/l authoring passes in this span select
        // physical.int through state+0x130[span_end], not through each word's
        // own class byte.  This is visible at 0x1021568b..0x102156c2.
        int cls = as.last_word<classes.size() ? classes[as.last_word] :
            (utterance_terminal ? m27_terminal_physical_class(end_word.kind,policy,frontend)
                                : m27_internal_physical_class(end_word.kind,policy));
        cls=std::clamp(cls,0,13);
        const auto& rec=physical->records[static_cast<std::size_t>(cls)];

        // 0x10214fcc..0x10215072 counts words whose annotated orthography has
        // '<' or '>'. Normal stressed Russian words have both, e.g.
        // <ска<зка> and <молоко<> in the supplied exc_rus.txt, so each
        // frontend word with a resolved stress is marker-bearing.
        int marker_word_count=0;
        int total_vowels=0;
        std::vector<int> vowel_prefix(as.last_word-as.first_word+2,0);
        for(std::size_t wi=as.first_word;wi<=as.last_word;++wi){
            int n=0;
            for(std::size_t pi=words[wi].first;pi<=words[wi].last && pi<phones.size();++pi)
                if(legacy_phone_is_vowel_m23(phones[pi])) ++n;
            total_vowels+=n;
            vowel_prefix[wi-as.first_word+1]=total_vowels;
            if(!frontend || wi>=frontend->words.size() || frontend->words[wi].stress_vowel_index>=0)
                ++marker_word_count;
        }
        total_vowels=std::max(1,total_vowels);
        marker_word_count=std::max(1,marker_word_count);

        bool seen_empty_marker=false;
        for(std::size_t wi=as.first_word;wi<=as.last_word;++wi){
            const auto& sp=words[wi];
            std::vector<std::size_t> active;
            for(std::size_t i=sp.first;i<=sp.last && i<phones.size();++i) if(phones[i]!="#") active.push_back(i);
            if(active.empty()) continue;

            // Keep the broad physical table available for diagnostics, but use
            // the authoring-span row for every word. Production broad strength
            // is still zero, so this cannot silently retune M29.
            for(std::size_t r=0;r<active.size();++r){
                const double x=active.size()<=1?0.0:static_cast<double>(r)/static_cast<double>(active.size()-1);
                out[active[r]]=legacy_physical_pitch_triplet(rec,x,18);
            }

            // Retain the empirically validated M27 terminal proxy only on the
            // final word of the whole utterance. It remains independently
            // controllable while the exact legacy terminal branch is audited.
            if(utterance_terminal && wi==as.last_word){
                const auto last=active.back();
                out[last][0]=static_cast<double>(rec.values[20])+static_cast<double>(rec.values[25]);
                out[last][1]=static_cast<double>(rec.values[20])+static_cast<double>(rec.values[32]);
                out[last][2]=static_cast<double>(rec.values[20])+static_cast<double>(rec.values[39]);
                result.terminal_proxy[last]=true;
            }

            std::size_t stress_phone=active.back();
            int stress_ord=-1;
            if(frontend && wi<frontend->words.size()) stress_ord=frontend->words[wi].stress_vowel_index;
            int seen_vowels=0;
            bool found_stress=false;
            for(const auto pi:active){
                if(!legacy_phone_is_vowel_m23(phones[pi])) continue;
                if((stress_ord>=0 && seen_vowels==stress_ord) ||
                   (stress_ord<0 && m24_phone_is_stressed_vowel(phones[pi]))){
                    stress_phone=pi; found_stress=true; break;
                }
                ++seen_vowels;
            }
            if(!found_stress){
                for(const auto pi:active) if(m24_phone_is_stressed_vowel(phones[pi])){
                    stress_phone=pi; found_stress=true; break;
                }
            }
            if(!found_stress) continue;

            const auto rel=wi-as.first_word;
            const int vowel_position=std::clamp(vowel_prefix[rel] + std::max(0,stress_ord),0,total_vowels-1);
            const bool empty_marker=(stress_phone==active.back());
            if(empty_marker){
                const int base_slot=legacy_physical_base_slot_pc(marker_word_count,wordstr24);
                // Late branches of 0x10214f40 feed the same span-wide vowel
                // ordinal/count to 0x10215d30.  M28's synthetic 2*word marker
                // coordinate was therefore wrong for interpolated <> nodes.
                out[stress_phone]=legacy_physical_empty_marker_triplet_pc(
                    rec,!seen_empty_marker,utterance_terminal && wi==as.last_word,
                    base_slot,vowel_position,total_vowels);
                result.empty_marker_exact[stress_phone]=true;
                seen_empty_marker=true;
            } else {
                // As in M29, do not let a disabled exact layer leak into a
                // neighbouring three-point midpoint. Only author a single-<
                // triplet when its independent experimental strength is live.
                if(policy.physical_single_marker_pitch_strength>0.0){
                    out[stress_phone]=legacy_physical_single_marker_triplet_pc(
                        rec,marker_word_count,vowel_position,total_vowels,
                        wordstr24,wordstr33);
                }
                result.single_marker_exact[stress_phone]=true;
            }
        }
    }

    for(std::size_t i=0;i<phones.size();++i){
        if(phones[i]!="#") continue;
        std::size_t l=i,r=i; bool hl=false,hr=false;
        while(l>0){--l;if(phones[l]!="#"){hl=true;break;}}
        while(r+1<phones.size()){++r;if(phones[r]!="#"){hr=true;break;}}
        if(hl&&hr) for(int k=0;k<3;++k) out[i][k]=0.5*(out[l][k]+out[r][k]);
        else if(hl) out[i]=out[l];
        else if(hr) out[i]=out[r];
    }
    return result;
}

struct M31PhysicalLengthEnergyLattice {
    std::vector<double> length_percent;
    std::vector<double> energy_percent;
    std::vector<bool> authored;
};

M31PhysicalLengthEnergyLattice m31_physical_length_energy_lattice(
    const std::vector<std::string>& phones,
    const LegacyPhysicalProsodyProfile* physical,
    const LegacyWordProsodyProfile* wordstr,
    const LegacyTimingPolicy& policy,
    const RussianFrontendResult* frontend){
    M31PhysicalLengthEnergyLattice out;
    out.length_percent.assign(phones.size(),0.0);
    out.energy_percent.assign(phones.size(),100.0);
    out.authored.assign(phones.size(),false);
    if(!frontend || !physical || !physical->valid || physical->records.size()<14) return out;

    // Resolve each frontend word back to its phone span in the flattened chain.
    struct WordSpan { std::size_t first=0,last=0; };
    std::vector<WordSpan> words;
    words.reserve(frontend->words.size());
    std::size_t cursor=0;
    while(cursor<phones.size() && phones[cursor]=="#") ++cursor;
    for(std::size_t wi=0;wi<frontend->words.size() && cursor<phones.size();++wi){
        while(cursor<phones.size() && phones[cursor]=="#") ++cursor;
        const std::size_t first=cursor;
        while(cursor<phones.size() && phones[cursor]!="#") ++cursor;
        if(cursor>first) words.push_back({first,cursor-1});
        else words.push_back({first,first});
        while(cursor<phones.size() && phones[cursor]=="#") ++cursor;
    }
    if(words.size()!=frontend->words.size()) return out;

    const double wordstr24=(wordstr && wordstr->valid && wordstr->values.size()>24)
        ? static_cast<double>(wordstr->values[24]) : 2.0;
    const auto spans=legacy_authoring_spans_m30(*frontend,policy);
    for(const auto& span:spans){
        if(span.first_word>=words.size() || span.last_word>=words.size() ||
           span.first_word>span.last_word) continue;
        const int cls=std::clamp(span.physical_class,0,13);
        const auto& rec=physical->records[static_cast<std::size_t>(cls)];
        const int marker_count=std::max(1,span.marker_word_count);
        int marker_ordinal=0;
        for(std::size_t wi=span.first_word;wi<=span.last_word;++wi){
            if(wi>=frontend->words.size()) break;
            const int stress_ord=frontend->words[wi].stress_vowel_index;
            if(stress_ord<0) continue;
            ++marker_ordinal;
            int seen=0;
            std::size_t stress_phone=words[wi].last;
            bool found=false;
            for(std::size_t pi=words[wi].first;pi<=words[wi].last && pi<phones.size();++pi){
                if(!legacy_phone_is_vowel_m23(phones[pi])) continue;
                if(seen==stress_ord){stress_phone=pi;found=true;break;}
                ++seen;
            }
            if(!found || stress_phone>=phones.size()) continue;

            // Generic portable frontend produces the ordinary stressed-vowel
            // single-'<' path.  The exact PC branches for '>', '<<' and '<<<'
            // are exposed by the helpers but are not invented when their raw
            // authoring markers are unavailable.
            const int lp=legacy_physical_length_percent_pc(
                rec,marker_count,marker_ordinal,false,false,false,wordstr24);
            const int ep=legacy_physical_energy_percent_pc(
                rec,marker_count,marker_ordinal,false,false,false);
            out.length_percent[stress_phone]=static_cast<double>(lp);
            out.energy_percent[stress_phone]=static_cast<double>(ep);
            out.authored[stress_phone]=true;
        }
    }
    return out;
}
}

std::vector<int> legacy_physical_class_lattice_m27(
    const RussianFrontendResult& frontend, const LegacyTimingPolicy& policy){
    std::vector<int> out(frontend.words.size(),0);
    if(out.empty()) return out;
    for(std::size_t wi=0;wi<out.size();++wi){
        const auto kind = wi<frontend.boundaries.size()
            ? frontend.boundaries[wi].kind : FrontendBoundaryKind::Word;
        const bool terminal = wi+1==out.size();
        out[wi]=terminal
            ? m27_terminal_physical_class(kind,policy,&frontend)
            : m27_internal_physical_class(kind,policy);
        out[wi]=std::clamp(out[wi],0,13);
    }
    return out;
}

LegacyRecoveredSymbol legacy_recovered_symbol(const std::string& phone) {
    if (phone.empty() || phone == "#") return LegacyRecoveredSymbol::Other;
    std::string p = phone;
    if (!p.empty() && p.back() == '\'') p.pop_back();
    while (!p.empty() && p.back() >= '0' && p.back() <= '9') p.pop_back();
    if (p == "i" || p == "I") return LegacyRecoveredSymbol::I;
    if (p == "j" || p == "J") return LegacyRecoveredSymbol::J;
    if (p == "r" || p == "R") return LegacyRecoveredSymbol::R;
    if (p == "u" || p == "U") return LegacyRecoveredSymbol::U;
    return LegacyRecoveredSymbol::Other;
}

LegacyRecoveredRuleHits analyze_legacy_recovered_neighbor_rules(
    const std::vector<std::string>& phones) {
    LegacyRecoveredRuleHits h;
    auto at = [&](std::ptrdiff_t i) {
        if (i < 0 || static_cast<std::size_t>(i) >= phones.size())
            return LegacyRecoveredSymbol::Other;
        return legacy_recovered_symbol(phones[static_cast<std::size_t>(i)]);
    };
    for (std::size_t n = 0; n < phones.size(); ++n) {
        const auto i = static_cast<std::ptrdiff_t>(n);
        const auto cur = at(i);
        if (cur == LegacyRecoveredSymbol::Other) continue;
        const auto prev = at(i - 1);
        const auto next = at(i + 1);
        if (prev == LegacyRecoveredSymbol::R || next == LegacyRecoveredSymbol::R)
            ++h.r_adjacent;
        if (prev == LegacyRecoveredSymbol::J && cur == LegacyRecoveredSymbol::I)
            ++h.j_to_i;
        if (cur == LegacyRecoveredSymbol::U && next == LegacyRecoveredSymbol::J &&
            at(i + 2) == LegacyRecoveredSymbol::U)
            ++h.u_j_u_left_u;
        if (prev == LegacyRecoveredSymbol::U && cur == LegacyRecoveredSymbol::J &&
            next == LegacyRecoveredSymbol::U)
            ++h.u_j_u_center_j;
        if (at(i - 2) == LegacyRecoveredSymbol::U && prev == LegacyRecoveredSymbol::J &&
            cur == LegacyRecoveredSymbol::U)
            ++h.u_j_u_right_u;
    }
    return h;
}


std::vector<LegacyAuthoringSpanState> legacy_authoring_spans_m30(
    const RussianFrontendResult& frontend, const LegacyTimingPolicy& policy){
    std::vector<LegacyAuthoringSpanState> out;
    if(frontend.words.empty()) return out;
    const auto classes=legacy_physical_class_lattice_m27(frontend,policy);
    std::size_t first=0;
    for(std::size_t wi=0;wi<frontend.words.size();++wi){
        const auto kind=wi<frontend.boundaries.size()?frontend.boundaries[wi].kind:FrontendBoundaryKind::Word;
        const bool hard=(kind==FrontendBoundaryKind::Sentence ||
                         kind==FrontendBoundaryKind::Question ||
                         kind==FrontendBoundaryKind::Exclamation ||
                         kind==FrontendBoundaryKind::Semicolon);
        if(!hard && wi+1<frontend.words.size()) continue;
        LegacyAuthoringSpanState st;
        st.first_word=first; st.last_word=wi;
        st.physical_class=wi<classes.size()?classes[wi]:0;
        for(std::size_t j=first;j<=wi;++j){
            if(frontend.words[j].stress_vowel_index>=0) ++st.marker_word_count;
            for(const auto& ph:frontend.words[j].phones) if(legacy_phone_is_vowel_m23(ph)) ++st.vowel_count;
        }
        st.marker_word_count=std::max(1,st.marker_word_count);
        st.vowel_count=std::max(1,st.vowel_count);
        out.push_back(st);
        first=wi+1;
    }
    return out;
}

LegacyPhoneDurationProfile parse_legacy_russian_phone_durations(const std::vector<std::uint8_t>&b,const EdatLayout&l){LegacyPhoneDurationProfile out;if(!l.valid){out.error="invalid_edat";return out;}const auto recs=scan_static_file_records(b,l);const auto*r=find_static_file_record(recs,"rusvox\\data\\duration.par");if(!r||!r->has_field_10c_ref){out.error="duration_record_missing";return out;}auto rr=resolve_static_ref({make_edat_range_descriptor(l)},StaticDuration::Initialization,kEdatTaggedRefMagic,r->field_10c_target);if(!rr.valid){out.error="duration_static_ref_failed";return out;}std::size_t o=static_cast<std::size_t>(rr.file_offset);for(int i=0;i<128&&o+8<=b.size();++i,o+=8){const auto ph=phone4(le32(b,o));const auto ms=int(le32(b,o+4));if(ph.empty()||ph[0]==0)break;bool ascii=true;for(unsigned char c:ph)if(c<0x20||c>0x7e){ascii=false;break;}if(!ascii||ms<=0||ms>1000)break;out.milliseconds[ph]=ms;}out.valid=out.milliseconds.size()>=60;if(!out.valid)out.error="duration_table_too_short";return out;}

LegacyWordProsodyProfile parse_legacy_russian_wordstr(const std::vector<std::uint8_t>&b,const EdatLayout&l){LegacyWordProsodyProfile out;if(!l.valid){out.error="invalid_edat";return out;}const auto recs=scan_static_file_records(b,l);const auto*r=find_static_file_record(recs,"rusvox\\data\\wordstr.par");if(!r||!r->has_field_10c_ref){out.error="wordstr_record_missing";return out;}auto rr=resolve_static_ref({make_edat_range_descriptor(l)},StaticDuration::Initialization,kEdatTaggedRefMagic,r->field_10c_target);if(!rr.valid||rr.file_offset+35*4>b.size()){out.error="wordstr_static_ref_failed";return out;}out.values.reserve(35);for(int i=0;i<35;++i){float f=0;const auto u=le32(b,static_cast<std::size_t>(rr.file_offset)+i*4);std::memcpy(&f,&u,sizeof(f));out.values.push_back(f);}out.valid=true;return out;}

LegacyPhysicalProsodyProfile parse_legacy_russian_physical(
    const std::vector<std::uint8_t>& b, const EdatLayout& l){
    LegacyPhysicalProsodyProfile out;
    if(!l.valid){out.error="invalid_edat";return out;}
    const auto recs=scan_static_file_records(b,l);
    const auto* r=find_static_file_record(recs,"rusvox\\data\\physical.int");
    if(!r||!r->has_field_10c_ref){out.error="physical_record_missing";return out;}
    const auto rr=resolve_static_ref({make_edat_range_descriptor(l)},StaticDuration::Initialization,
                                     kEdatTaggedRefMagic,r->field_10c_target);
    constexpr std::size_t kRecords=14, kStride=42;
    if(!rr.valid || rr.file_offset+kRecords*kStride>b.size()){out.error="physical_static_ref_failed";return out;}
    out.records.resize(kRecords);
    const auto base=static_cast<std::size_t>(rr.file_offset);
    for(std::size_t ri=0;ri<kRecords;++ri)
        for(std::size_t j=0;j<kStride;++j)
            out.records[ri].values[j]=static_cast<std::int8_t>(b[base+ri*kStride+j]);
    // Canonical Nicolai image sanity checks recovered from the MSI/database.
    bool zero=true; for(auto v:out.records[0].values) zero &= (v==0);
    if(!zero || out.records[1].values[0]!=-20 || out.records[1].values[1]!=49 ||
       out.records[1].values[18]!=10 || out.records[1].values[21]!=15 ||
       out.records[13].values[41]!=50){out.error="physical_table_signature_mismatch";out.records.clear();return out;}
    out.valid=true;
    return out;
}

std::array<double,3> legacy_physical_pitch_triplet(
    const LegacyPhysicalProsodyRecord& record, double position, int base_slot){
    base_slot=std::clamp(base_slot,18,20);
    const double x=std::clamp(position,0.0,1.0)*6.0;
    const int i0=std::clamp(static_cast<int>(std::floor(x)),0,6);
    const int i1=std::min(6,i0+1);
    const double t=x-static_cast<double>(i0);
    const double base=static_cast<double>(record.values[static_cast<std::size_t>(base_slot)]);
    std::array<double,3> out{};
    for(int k=0;k<3;++k){
        const int group=21+k*7;
        const double a=static_cast<double>(record.values[static_cast<std::size_t>(group+i0)]);
        const double c=static_cast<double>(record.values[static_cast<std::size_t>(group+i1)]);
        out[static_cast<std::size_t>(k)]=base+a+(c-a)*t;
    }
    return out;
}

int legacy_physical_interp_add_pc(int additive, int start, int end, int position, int count){
    if(count<=1) return additive+start;
    const double v=static_cast<double>(additive+start) +
        static_cast<double>(end-start)*static_cast<double>(position)/static_cast<double>(count-1);
    return static_cast<int>(v); // MSVCRT _ftol: truncate toward zero
}

int legacy_physical_interp_pc(int start, int end, int position, int count){
    if(count<=1) return start;
    const double v=static_cast<double>(start) +
        static_cast<double>(end-start)*static_cast<double>(position)/static_cast<double>(count-1);
    return static_cast<int>(v);
}

int legacy_physical_base_slot_pc(int marker_word_count, double threshold){
    // fild marker_word_count; fcomp wordstr[24]; test AH,0x41. For Nicolai
    // wordstr[24] is exactly 2.0: <= threshold selects rec[18], > selects 19.
    return static_cast<double>(marker_word_count)<=threshold ? 18 : 19;
}

std::array<double,3> legacy_physical_single_marker_triplet_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int vowel_position,
    int vowel_count,
    double wordstr_24,
    double wordstr_33){
    const int base_slot=legacy_physical_base_slot_pc(marker_word_count,wordstr_24);
    vowel_count=std::max(1,vowel_count);
    vowel_position=std::clamp(vowel_position,0,vowel_count-1);
    // 0x10214f65..0x10214f98 computes (wordstr[33]*0.5)-wordstr[33]
    // and converts through the legacy _ftol helper.  For Nicolai 20 -> -10.
    const int bias=static_cast<int>(-0.5*wordstr_33);
    const int start=bias+static_cast<int>(record.values[static_cast<std::size_t>(base_slot)]);
    const int end=static_cast<int>(record.values[20]);
    std::array<double,3> out{};
    for(int lane=0;lane<3;++lane){
        // Static stack-dataflow of 0x1021516f..0x10215b13 proves the selector
        // guard local at baseline ESP+0x3c is initialized to zero and has no
        // reachable writes or aliases before the branch.  Therefore k3 is
        // dead in this Nicolai build and the live ordinary '<' path is k1.
        const int additive=static_cast<int>(record.values[22+lane*7]);
        out[static_cast<std::size_t>(lane)]=static_cast<double>(
            legacy_physical_interp_add_pc(additive,start,end,vowel_position,vowel_count));
    }
    return out;
}

std::array<double,3> legacy_physical_empty_marker_triplet_pc(
    const LegacyPhysicalProsodyRecord& record,
    bool first_empty_marker, bool last_global_marker, int base_slot,
    int marker_position, int marker_count){
    base_slot=std::clamp(base_slot,18,19);
    marker_count=std::max(1,marker_count);
    marker_position=std::clamp(marker_position,0,marker_count-1);
    std::array<double,3> out{};
    if(last_global_marker){
        // 0x10215722..0x10215784: rec[20] + k4 in each seven-wide lane.
        for(int lane=0;lane<3;++lane)
            out[static_cast<std::size_t>(lane)] =
                static_cast<double>(record.values[20]) +
                static_cast<double>(record.values[21+lane*7+4]);
        return out;
    }
    const int selector = first_empty_marker ? 0 : 2;
    const int start=record.values[static_cast<std::size_t>(base_slot)];
    const int end=record.values[20];
    for(int lane=0;lane<3;++lane){
        const int additive=record.values[static_cast<std::size_t>(21+lane*7+selector)];
        if(first_empty_marker)
            out[static_cast<std::size_t>(lane)] = static_cast<double>(start+additive);
        else
            out[static_cast<std::size_t>(lane)] = static_cast<double>(
                legacy_physical_interp_add_pc(additive,start,end,marker_position,marker_count));
    }
    return out;
}


int legacy_physical_length_percent_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int marker_ordinal,
    bool marker_is_greater,
    bool has_double_left_from_marker,
    bool has_triple_left_in_word,
    double wordstr_24){
    marker_word_count=std::max(1,marker_word_count);
    marker_ordinal=std::clamp(marker_ordinal,1,marker_word_count);
    const bool long_profile=static_cast<double>(marker_word_count)>wordstr_24;
    const int base=long_profile?8:3;
    int selector=3;
    if(marker_is_greater) selector=0;
    else if(has_double_left_from_marker) selector=has_triple_left_in_word?1:2;
    int value=static_cast<int>(record.values[static_cast<std::size_t>(base+selector)]);
    if(marker_ordinal==marker_word_count)
        value+=static_cast<int>(record.values[static_cast<std::size_t>(base+4)]);
    return value;
}

int legacy_physical_energy_percent_pc(
    const LegacyPhysicalProsodyRecord& record,
    int marker_word_count,
    int marker_ordinal,
    bool marker_is_greater,
    bool has_double_left_from_marker,
    bool has_triple_left_in_word){
    marker_word_count=std::max(1,marker_word_count);
    marker_ordinal=std::clamp(marker_ordinal,1,marker_word_count);
    int selector=3;
    if(marker_is_greater) selector=0;
    else if(has_triple_left_in_word) selector=has_double_left_from_marker?1:2;
    const int start=static_cast<int>(record.values[static_cast<std::size_t>(13+selector)]);
    if(marker_word_count<=1) return start;
    const int terminal=static_cast<int>(record.values[17]);
    const double factor=100.0-
        static_cast<double>(100-terminal)*static_cast<double>(marker_ordinal-1)/
        static_cast<double>(marker_word_count-1);
    // The PC branch adds 0.5 before _ftol for these positive energy values.
    return static_cast<int>(static_cast<double>(start)*factor*0.01+0.5);
}

LegacyAuthoringSplitDecisionM31 legacy_authoring_split_decision_m31(
    const std::vector<LegacyAuthoringSplitItemM31>& items,
    std::size_t first, std::size_t last,
    char candidate_separator, int threshold, bool strict_greater){
    LegacyAuthoringSplitDecisionM31 out;
    if(items.empty() || first>=items.size() || first>last) return out;
    last=std::min(last,items.size()-1);
    int effective=0;
    for(std::size_t i=first;i<=last;++i)
        if(items[i].annotated_has_left || !items[i].legacy_skip_count) ++effective;
    out.effective_count=effective;
    const bool over=strict_greater ? effective>threshold : effective>=threshold;
    if(!over) return out;
    const double midpoint=0.5*(static_cast<double>(first)+static_cast<double>(last));
    bool found=false;
    double best=0.0;
    for(std::size_t i=first;i<=last;++i){
        if(!items[i].code_empty || items[i].raw_separator!=candidate_separator) continue;
        const double d=std::abs(static_cast<double>(i)-midpoint);
        // Static reverse confirms midpoint-nearest selection.  Lower index on
        // an exact tie is the conservative deterministic projection until the
        // raw separator producer itself is fully named.
        if(!found || d<best-1e-12 || (std::abs(d-best)<=1e-12 && i<out.index)){
            found=true; best=d; out.index=i;
        }
    }
    out.split=found;
    return out;
}

double legacy_pitch_target_f0_m23(
    double source_f0_hz,
    const LegacyWordProsodyProfile* wordstr,
    const LegacyTimingPolicy& policy) {
    if (!(source_f0_hz > 1.0) || !(policy.legacy_pitch_base_hz > 20.0))
        return source_f0_hz;
    const double strength = std::clamp(policy.legacy_pitch_strength, 0.0, 1.0);
    if (strength <= 0.0) return source_f0_hz;
    const double recovered = (wordstr && wordstr->valid && wordstr->values.size() > 28)
        ? static_cast<double>(wordstr->values[28]) : 1.0;
    const double residual = std::clamp(policy.legacy_pitch_source_residual, 0.0, 2.0);
    const double base = policy.legacy_pitch_base_hz;
    const double contour_input = base + (source_f0_hz - base) * residual;
    const double pc_adjusted = base + (contour_input - base) * recovered;
    return source_f0_hz + (pc_adjusted - source_f0_hz) * strength;
}

DiphoneChainLegacyResult synthesize_diphone_chain_legacy_duration(
    const std::vector<std::uint8_t>&db,
    const DiphoneCatalog&catalog,
    const std::vector<std::string>&phones,
    const LegacyPhoneDurationProfile&dur,
    const LegacyWordProsodyProfile*wordstr,
    double pitch_scale,
    int sample_rate,
    const std::vector<FrontendBoundary>&boundaries,
    const LegacyTimingPolicy&policy,
    const LegacyPhysicalProsodyProfile*physical,
    const RussianFrontendResult*frontend) {
    DiphoneChainLegacyResult out;
    if(!catalog.valid||!dur.valid){out.error="invalid_inputs";return out;}
    if(phones.size()<2||sample_rate<=0){out.error="invalid_phone_chain";return out;}

    auto kind_at = [&](std::size_t phone_index) {
        for (const auto& b : boundaries)
            if (b.phone_index == phone_index) return b.kind;
        return FrontendBoundaryKind::Word;
    };
    auto boundary_scale = [&](std::size_t phone_index) {
        switch (kind_at(phone_index)) {
            case FrontendBoundaryKind::Comma: return policy.comma_boundary_scale;
            case FrontendBoundaryKind::Semicolon:
            case FrontendBoundaryKind::Colon:
            case FrontendBoundaryKind::Dash: return policy.weak_punctuation_boundary_scale;
            case FrontendBoundaryKind::Hyphen: return policy.word_boundary_scale;
            case FrontendBoundaryKind::Sentence:
            case FrontendBoundaryKind::Question:
            case FrontendBoundaryKind::Exclamation: return policy.strong_punctuation_boundary_scale;
            case FrontendBoundaryKind::Other:
            case FrontendBoundaryKind::Word:
            default: return policy.word_boundary_scale;
        }
    };

    std::vector<std::size_t> word_of_phone(phones.size(), static_cast<std::size_t>(-1));
    std::size_t word_count = 0;
    bool inside_word = false;
    for (std::size_t pi = 0; pi < phones.size(); ++pi) {
        if (phones[pi] == "#") { inside_word = false; continue; }
        if (!inside_word) { inside_word = true; ++word_count; }
        word_of_phone[pi] = word_count - 1;
    }

    std::vector<double> word_contour(word_count, 1.0);
    if (policy.enable_wordstr_position_contour && wordstr && wordstr->valid &&
        wordstr->values.size() > 17 && word_count > 1) {
        const double c0 = wordstr->values[7];
        const double cm = wordstr->values[12];
        const double c1 = wordstr->values[17];
        double mean = 0.0;
        for (std::size_t wi = 0; wi < word_count; ++wi) {
            const double x = static_cast<double>(wi) / static_cast<double>(word_count - 1);
            double c = x <= 0.5
                ? c0 + (cm - c0) * (x * 2.0)
                : cm + (c1 - cm) * ((x - 0.5) * 2.0);
            word_contour[wi] = c;
            mean += c;
        }
        mean /= static_cast<double>(word_count);
        if (mean > 1e-9) {
            for (auto& c : word_contour) {
                const double normalized = std::clamp(c / mean, 0.70, 1.35);
                c = std::pow(normalized, std::clamp(policy.wordstr_contour_strength, 0.0, 2.0));
            }
        }
    }

    const auto physical_le_lattice=m31_physical_length_energy_lattice(
        phones,physical,wordstr,policy,frontend);
    const double physical_length_strength=std::clamp(policy.physical_length_strength,0.0,1.0);
    const double physical_energy_strength=std::clamp(policy.physical_energy_strength,0.0,1.0);

    struct UnitCtx {
        Pcm16Mono raw;
        SegScheduleM15 sched;
        SegSpanLayout layout;
        SegSourceTimelineM33 timeline;
        std::string label;
        double source_ms = 0.0;
        double target_ms = 0.0;
        double base_scale = 1.0;
    };
    std::vector<UnitCtx> units;
    units.reserve(phones.size()-1);

    // First pass is deliberately the exact M22 target-duration calculation.
    // M23 only redistributes that already-calibrated target across the recovered
    // phone transition; it does not get to silently move the corpus-wide clock.
    for(std::size_t i=0;i+1<phones.size();++i){
        const auto*unit=find_diphone(catalog,phones[i],phones[i+1]);
        if(!unit){out.error="missing_diphone_"+phones[i]+"_"+phones[i+1];return out;}
        const auto enc=extract_diphone_compressed_bytes(db,catalog,*unit);
        UnitCtx u;
        u.raw=decode_g711_alaw_pcm(enc,sample_rate);
        if(u.raw.samples.empty()){out.error="decode_failed";return out;}
        u.sched=parse_seg_schedule_m15(*unit);
        if(policy.use_stateful_tds_m34)
            u.timeline=source_timeline_seg_m33(u.sched,u.raw.samples.size(),unit->signed_end>=0,sample_rate);
        u.layout=(policy.use_pc_seg_timeline || policy.use_stateful_tds_m34)
            ? layout_seg_runs_m33(u.sched,u.raw.samples.size(),unit->signed_end>=0,sample_rate)
            : layout_seg_runs_m15(u.sched,u.raw.samples.size());
        if(!u.sched.valid||!u.layout.valid){out.error="seg_failed";return out;}
        u.label=phones[i]+"->"+phones[i+1];
        u.source_ms=1000.0*u.raw.samples.size()/sample_rate;
        u.target_ms=u.source_ms;
        u.base_scale=1.0;
        if(phones[i]!="#"&&phones[i+1]!="#"){
            auto a=dur.milliseconds.find(phones[i]), b=dur.milliseconds.find(phones[i+1]);
            if(a!=dur.milliseconds.end()&&b!=dur.milliseconds.end()){
                u.target_ms=0.5*(a->second+b->second);
                u.base_scale=std::clamp(u.target_ms/u.source_ms,0.35,2.5);
                u.base_scale=std::clamp(u.base_scale*policy.phone_duration_scale,0.35,2.5);
                const auto wi=word_of_phone[i];
                if(wi!=static_cast<std::size_t>(-1)&&wi<word_contour.size())
                    u.base_scale=std::clamp(u.base_scale*word_contour[wi],0.35,2.5);
                const bool lv=legacy_phone_is_vowel_m23(phones[i]);
                const bool rv=legacy_phone_is_vowel_m23(phones[i+1]);
                const double class_scale = lv ? (rv ? policy.vv_duration_scale : policy.vc_duration_scale)
                                              : (rv ? policy.cv_duration_scale : policy.cc_duration_scale);
                u.base_scale=std::clamp(u.base_scale*class_scale,0.35,2.5);
                u.target_ms=u.source_ms*u.base_scale;
            }
        } else {
            std::size_t boundary_index=phones.size();
            if(phones[i]=="#"&&i>0) boundary_index=i;
            else if(phones[i+1]=="#"&&i+1<phones.size()-1) boundary_index=i+1;
            if(boundary_index<phones.size()) u.base_scale=std::clamp(boundary_scale(boundary_index),0.20,2.50);
            else u.base_scale=std::clamp(policy.utterance_boundary_scale,0.20,2.50);
            u.target_ms=u.source_ms*u.base_scale;
        }
        units.push_back(std::move(u));
    }

    // Global phone-support lattice. A phone is represented by the right side
    // of the preceding diphone plus the left side of the following diphone.
    // duration.par therefore targets that combined source support once instead
    // of being duplicated independently into two whole-diphone averages.
    std::vector<double> phone_side_hint(phones.size(), 1.0);
    std::vector<bool> phone_side_valid(phones.size(), false);
    for(std::size_t pi=0;pi<phones.size();++pi){
        if(phones[pi]=="#") continue;
        double support=0.0;
        if(pi>0 && pi-1<units.size()){
            const auto&u=units[pi-1];
            const auto extent=policy.use_stateful_tds_m34 ? u.raw.samples.size()-1 : u.raw.samples.size();
            support += static_cast<double>(extent-std::min(u.layout.split_sample_estimate,extent));
        }
        if(pi<units.size()){
            const auto&u=units[pi];
            support += static_cast<double>(std::min(u.layout.split_sample_estimate,u.raw.samples.size()));
        }
        auto it=dur.milliseconds.find(phones[pi]);
        if(support<=1.0||it==dur.milliseconds.end()) continue;
        double target=static_cast<double>(it->second)*sample_rate/1000.0*policy.phone_duration_scale;
        const auto wi=word_of_phone[pi];
        if(wi!=static_cast<std::size_t>(-1)&&wi<word_contour.size()) target*=word_contour[wi];
        phone_side_hint[pi]=std::clamp(target/support,0.20,3.00);
        phone_side_valid[pi]=true;
    }

    std::vector<Pcm16Mono> rendered;
    std::vector<int> lh,rh;
    std::vector<double> unit_pitch_left, unit_pitch_right;
    rendered.reserve(units.size()); lh.reserve(units.size()); rh.reserve(units.size());
    unit_pitch_left.reserve(units.size()); unit_pitch_right.reserve(units.size());
    const double side_strength=std::clamp(policy.phone_side_strength,0.0,1.0);
    const double ratio_limit=std::max(1.0,policy.phone_side_ratio_limit);
    const double pitch_strength=std::clamp(policy.legacy_pitch_strength,0.0,1.0);
    const double anchor_strength=std::clamp(policy.pitch_anchor_strength,0.0,1.0);
    const double physical_strength=std::clamp(policy.physical_pitch_strength,0.0,1.0);
    const double physical_terminal_strength=std::clamp(policy.physical_terminal_pitch_strength,0.0,1.0);
    const auto phone_pitch_percent=m25_phone_pitch_percent_lattice(phones,word_count,policy);
    const auto physical_lattice=m27_physical_phone_pitch_lattice(phones,boundaries,physical,wordstr,policy,frontend);
    const auto& physical_pitch=physical_lattice.pitch;
    StatefulTdsM34 tds_state; // one carry owner per utterance, not per run/unit
    const char* chain_flag=std::getenv("NICOLAI_M36_CHAIN_EXECUTOR");
    const bool m36_chain=policy.use_stateful_tds_m34 && chain_flag && std::atoi(chain_flag)!=0;
    std::vector<StatefulTdsUnitM36> chain_units;

    for(std::size_t i=0;i<units.size();++i){
        auto&u=units[i];
        double left_scale=u.base_scale, right_scale=u.base_scale;
        const auto split=std::min(u.layout.split_sample_estimate,u.raw.samples.size());
        const double L=static_cast<double>(split);
        const double R=static_cast<double>(u.raw.samples.size()-split);
        if(side_strength>0.0 && L>0.0 && R>0.0){
            auto side = [&](std::size_t pi){
                double h=(pi<phone_side_valid.size()&&phone_side_valid[pi])?phone_side_hint[pi]:u.base_scale;
                h=std::clamp(h,u.base_scale/ratio_limit,u.base_scale*ratio_limit);
                return u.base_scale*std::pow(std::max(1e-9,h/u.base_scale),side_strength);
            };
            left_scale=side(i);
            right_scale=side(i+1);
            // Preserve the exact M22 per-unit target length. This makes the new
            // degree of freedom a transition-placement correction, not another
            // global duration knob.
            const double den=L*left_scale+R*right_scale;
            if(den>1e-9){
                const double c=u.base_scale*(L+R)/den;
                left_scale=std::clamp(left_scale*c,0.20,3.00);
                right_scale=std::clamp(right_scale*c,0.20,3.00);
                const double den2=L*left_scale+R*right_scale;
                if(den2>1e-9){
                    const double c2=u.base_scale*(L+R)/den2;
                    left_scale*=c2; right_scale*=c2;
                }
            }
        }

        const double boundary_share=std::clamp(policy.word_boundary_speech_share_m38,0.0,0.75);
        if(!policy.use_stateful_tds_m34 && boundary_share>0.0 && L>0.0 && R>0.0 &&
           (phones[i]=="#")!=(phones[i+1]=="#")){
            const std::size_t bi=phones[i]=="#" ? i : i+1;
            const bool plain_word=std::any_of(boundaries.begin(),boundaries.end(),
                [&](const FrontendBoundary& b){
                    return b.phone_index==bi && b.kind==FrontendBoundaryKind::Word;
                });
            if(bi>0 && bi+1<phones.size() && plain_word){
                const bool left_spoken=phones[i]!="#";
                const double spoken_source=left_spoken?L:R;
                const double boundary_source=left_spoken?R:L;
                const double boundary_scale=std::max(0.20,u.base_scale*(1.0-boundary_share));
                const double spoken_scale=(u.base_scale*(L+R)-boundary_source*boundary_scale)/spoken_source;
                // Reject extreme source splits rather than change the total
                // clock or silently clamp only one side of the balance.
                if(spoken_scale>=0.20 && spoken_scale<=3.00){
                    left_scale=left_spoken?spoken_scale:boundary_scale;
                    right_scale=left_spoken?boundary_scale:spoken_scale;
                }
            }
        }

        // M32: consume [l%d] where the PC runtime does: on the phone feature
        // record represented by each side of this diphone. Unlike M31's
        // weighted whole-unit projection, this deliberately changes the two
        // half durations independently and is not renormalized away.
        if(policy.use_stateful_tds_m34 && policy.shared_phone_duration_m34){
            // One phone coefficient owns the previous right + next left half.
            // Unlike M23 this is not renormalized to each diphone's target.
            if(phone_side_valid[i]) left_scale=phone_side_hint[i];
            if(phone_side_valid[i+1]) right_scale=phone_side_hint[i+1];
        }
        if(physical_length_strength>0.0){
            const double lp0=(i<physical_le_lattice.length_percent.size())
                ? physical_le_lattice.length_percent[i] : 0.0;
            const double lp1=(i+1<physical_le_lattice.length_percent.size())
                ? physical_le_lattice.length_percent[i+1] : 0.0;
            const double lm=std::clamp(1.0+physical_length_strength*lp0*0.01,0.70,1.35);
            const double rm=std::clamp(1.0+physical_length_strength*lp1*0.01,0.70,1.35);
            left_scale=std::clamp(left_scale*lm,0.20,3.00);
            right_scale=std::clamp(right_scale*rm,0.20,3.00);
        }
        // Preserve M31 bit-for-bit when the physical [l] layer is disabled.
        // Re-averaging two nominally equal side scales can change the last
        // floating-point bit and push llround across a sample boundary.
        const double effective_scale=((physical_length_strength>0.0 || policy.use_stateful_tds_m34) && (L+R)>0.0)
            ? (L*left_scale+R*right_scale)/(L+R) : u.base_scale;
        u.target_ms=u.source_ms*effective_scale;

        double left_energy_gain=1.0, right_energy_gain=1.0;
        if(physical_energy_strength>0.0){
            const double ep0=(i<physical_le_lattice.energy_percent.size())
                ? physical_le_lattice.energy_percent[i] : 100.0;
            const double ep1=(i+1<physical_le_lattice.energy_percent.size())
                ? physical_le_lattice.energy_percent[i+1] : 100.0;
            left_energy_gain=std::clamp(
                1.0+physical_energy_strength*(ep0*0.01-1.0),0.40,1.25);
            right_energy_gain=std::clamp(
                1.0+physical_energy_strength*(ep1*0.01-1.0),0.40,1.25);
        }

        M15UnitDiagnostics ud; ud.label=u.label; ud.schedule=u.sched; ud.layout=u.layout;
        if(!u.layout.runs.empty()){
            ud.left_period_hint=first_period(u.layout.runs.front());
            ud.right_period_hint=last_period(u.layout.runs.back());
        }

        // M23 carrier estimate plus M25 sparse PC-style anchor lattice.
        // The recovered PC pipeline stores signed percent anchors, converts
        // each around pitch.par base=83, then applies wordstr[28] centering.
        // M25 reproduces that math and the sparse linear interpolation; only
        // the upstream lexical rule choosing explicit anchors is reconstructed.
        double source_f0=0.0;
        double unit_pitch_scale=pitch_scale;
        const double period=robust_period_m23(u.layout);
        if(period>0.0) source_f0=static_cast<double>(sample_rate)/period;
        if(pitch_strength>0.0 && source_f0>0.0 && policy.legacy_pitch_base_hz>20.0){
            const double target_f0=legacy_pitch_target_f0_m23(source_f0,wordstr,policy);
            unit_pitch_scale*=std::clamp(target_f0/source_f0,0.70,1.35);
        }

        const double den_units=std::max<std::size_t>(1,units.size());
        const double x0=static_cast<double>(i)/static_cast<double>(den_units);
        const double x1=static_cast<double>(i+1)/static_cast<double>(den_units);
        const double xm=0.5*(x0+x1);
        const double stress_boost=std::clamp(policy.pitch_stressed_vowel_boost,0.75,1.35);
        const double left_stress=m24_phone_is_stressed_vowel(phones[i])?stress_boost:1.0;
        const double right_stress=m24_phone_is_stressed_vowel(phones[i+1])?stress_boost:1.0;

        auto blended_scale=[&](double percent, double physical_percent, double local_physical_strength){
            double s=unit_pitch_scale;
            if(anchor_strength>0.0 && source_f0>1e-9){
                const double af0=m25_anchor_f0(percent,wordstr,policy);
                const double as=pitch_scale*std::clamp(af0/source_f0,0.65,1.45);
                // Geometric blend is stable for multiplicative pitch ratios and
                // gives anchor_strength=0 an exact M23 fallback.
                s=std::exp((1.0-anchor_strength)*std::log(std::max(1e-9,s)) +
                           anchor_strength*std::log(std::max(1e-9,as)));
            }
            if(local_physical_strength>0.0 && source_f0>1e-9 && physical && physical->valid){
                const double pf0=m25_anchor_f0(physical_percent,wordstr,policy);
                const double ps=pitch_scale*std::clamp(pf0/source_f0,0.60,1.55);
                s=std::exp((1.0-local_physical_strength)*std::log(std::max(1e-9,s)) +
                           local_physical_strength*std::log(std::max(1e-9,ps)));
            }
            return s;
        };
        const double pct0=phone_pitch_percent[i];
        const double pct1=phone_pitch_percent[i+1];
        const double pctm=0.5*(pct0+pct1);
        const double phy0=physical_pitch[i][0];
        const double phy1=physical_pitch[i+1][2];
        const double phym=0.5*(physical_pitch[i][1]+physical_pitch[i+1][1]);
        const double empty_strength=std::clamp(policy.physical_empty_marker_pitch_strength,0.0,1.0);
        const double single_strength=std::clamp(policy.physical_single_marker_pitch_strength,0.0,1.0);
        auto physical_local_strength=[&](std::size_t idx){
            const bool exact_empty = idx<physical_lattice.empty_marker_exact.size() && physical_lattice.empty_marker_exact[idx];
            const bool exact_single = idx<physical_lattice.single_marker_exact.size() && physical_lattice.single_marker_exact[idx];
            const bool terminal_proxy = idx<physical_lattice.terminal_proxy.size() && physical_lattice.terminal_proxy[idx];
            // Preserve the already-validated M27 terminal correction even when
            // M28 experiments with non-terminal <> anchors.  The exact terminal
            // empty-marker branch is the same recovered k4 triplet, so weakening
            // it with the experimental strength would confound the comparison.
            if(exact_empty && terminal_proxy) return physical_terminal_strength;
            if(exact_empty && empty_strength>0.0) return empty_strength;
            if(exact_single && single_strength>0.0) return single_strength;
            if(terminal_proxy) return physical_terminal_strength;
            return physical_strength;
        };
        const double phy_strength0=physical_local_strength(i);
        const double phy_strength1=physical_local_strength(i+1);
        const double phy_strengthm=0.5*(phy_strength0+phy_strength1);
        const double ps0=blended_scale(pct0,phy0,phy_strength0)*m24_declination_multiplier(policy,x0)*left_stress;
        const double ps1=blended_scale(pct1,phy1,phy_strength1)*m24_declination_multiplier(policy,x1)*right_stress;
        const double psm=blended_scale(pctm,phym,phy_strengthm)*m24_declination_multiplier(policy,xm)*std::sqrt(left_stress*right_stress);

        Pcm16Mono pcm;
        TdPsolaConfig cfg; cfg.pitch_scale=psm; cfg.duration_scale=effective_scale;
        cfg.use_three_point_pitch=anchor_strength>0.0 || physical_strength>0.0 || physical_terminal_strength>0.0 || empty_strength>0.0 || single_strength>0.0 || policy.pitch_declination_strength>0.0 || std::abs(stress_boost-1.0)>1e-12;
        cfg.pitch_scale_start=ps0; cfg.pitch_scale_mid=psm; cfg.pitch_scale_end=ps1;
        cfg.search_join_phase=policy.search_join_phase;
        cfg.blend_uncovered_edges_m40=policy.blend_uncovered_edges_m40;
        cfg.audit_transients_m40=policy.audit_transients_m40;
        const bool side_duration=std::abs(left_scale-right_scale)>=1e-10;
        const bool side_energy=std::abs(left_energy_gain-1.0)>=1e-12 ||
                               std::abs(right_energy_gain-1.0)>=1e-12;
        if(m36_chain){
            StatefulTdsUnitM36 input;
            input.source=u.raw; input.timeline=u.timeline; input.pitch=cfg;
            input.left_duration=left_scale; input.right_duration=right_scale;
            input.left_energy=left_energy_gain; input.right_energy=right_energy_gain;
            // Experimental voicing-to-route policy, not captured original
            // caller gates. False boundaries flush/fade instead of mixing.
            input.cross_from_previous=i>0 && units[i-1].timeline.nodes.back().voiced;
            chain_units.push_back(std::move(input));
        } else if(policy.use_stateful_tds_m34){
            pcm=resynthesize_stateful_m34(u.raw,u.timeline,cfg,left_scale,right_scale,
                left_energy_gain,right_energy_gain,tds_state);
        } else if(!side_duration && !side_energy){
            pcm=resynthesize_seg_m15(u.raw,u.sched,u.layout,cfg,&ud);
        } else {
            pcm=resynthesize_seg_m32_phone_sides(
                u.raw,u.sched,u.layout,cfg,left_scale,right_scale,
                left_energy_gain,right_energy_gain,&ud);
        }
        if(!m36_chain){
            if(pcm.samples.empty()){out.error="render_failed";return out;}
            if(policy.audit_transients_m40){
                const auto [source_step,source_at]=max_pcm_step(u.raw);
                const auto [rendered_step,rendered_at]=max_pcm_step(pcm);
                LegacyUnitTransient transient{u.label,source_step,source_at,
                                              rendered_step,rendered_at,ud.internal_joins.size()};
                transient.internal_join_centers=ud.internal_join_centers;
                for(const auto&run:ud.runs){
                    transient.run_output_samples.push_back(run.output_samples);
                    transient.run_voiced.push_back(run.voiced);
                    transient.run_uncovered_samples.push_back(run.psola.uncovered_samples);
                    transient.run_psola_max_steps.push_back(run.psola.max_output_step);
                    transient.run_psola_max_step_at.push_back(run.psola.max_output_step_at);
                    transient.run_psola_weight_before.push_back(run.psola.max_step_weight_before);
                    transient.run_psola_weight_after.push_back(run.psola.max_step_weight_after);
                }
                out.unit_transients.push_back(std::move(transient));
            }
            rendered.push_back(std::move(pcm));
        }
        lh.push_back(ud.left_period_hint); rh.push_back(ud.right_period_hint);
        unit_pitch_left.push_back(ps0);
        unit_pitch_right.push_back(ps1);
        out.timings.push_back({ud.label,u.source_ms,u.target_ms,effective_scale,
                               left_scale,right_scale,left_energy_gain,right_energy_gain});
    }

    if(m36_chain){
        out.pcm=resynthesize_stateful_m36_chain_experimental(chain_units,tds_state);
        if(out.pcm.samples.empty()){out.error="m36_chain_render_failed";return out;}
    }
    out.tds_intervals_m34=tds_state.intervals;
    out.tds_grains_m34=tds_state.grains;
    out.tds_dropped_m34=tds_state.dropped;
    out.tds_final_carry_m34=tds_state.carry;
    out.tds_target_samples_m35=tds_state.target_samples;
    out.tds_budget_samples_m35=tds_state.budget_consumed_samples;
    out.tds_emitted_samples_m35=tds_state.emitted_samples;
    out.tds_clamped_records_m35=tds_state.clamped_delta_records;
    out.m36_initial_paths=tds_state.m36_initial_paths;
    out.m36_cross_paths=tds_state.m36_cross_paths;
    out.m36_terminal_flushes=tds_state.m36_terminal_flushes;
    out.m36_fallbacks=tds_state.m36_fallbacks;
    if(!m36_chain) out.pcm=rendered.front();
    for(std::size_t i=1;i<rendered.size();++i){
        OlaJoinDiagnostics jd;
        const double lps=(i-1<unit_pitch_right.size()?unit_pitch_right[i-1]:pitch_scale);
        const double rps=(i<unit_pitch_left.size()?unit_pitch_left[i]:pitch_scale);
        const int lp=rh[i-1]>0?int(std::lround(rh[i-1]/lps)):0;
        const int rp=lh[i]>0?int(std::lround(lh[i]/rps)):0;
        if(policy.use_stateful_tds_m34){
            // The adapter emits a continuous integer grain clock. Never trim
            // it again with the old waveform-correlation diphone join.
            out.pcm.samples.insert(out.pcm.samples.end(),rendered[i].samples.begin(),rendered[i].samples.end());
            continue;
        }
        const auto before=out.pcm.samples.size();
        out.pcm=hann_ola_join(out.pcm,rendered[i],lp,rp,&jd,policy.search_join_phase);
        if(!jd.valid){out.error="join_failed";return out;}
        const auto center_offset=std::min(before,jd.left_trim+jd.overlap_samples/2);
        out.joins.push_back({i,phones[i],before-center_offset,jd.overlap_samples,
                             jd.left_trim,jd.right_trim,jd.normalized_correlation});
    }
    out.valid=true;
    return out;
}

std::size_t legacy_pc_terminal_silence_samples(
    const LegacyWordProsodyProfile& wordstr, int sample_rate) {
    if (!wordstr.valid || wordstr.values.size() <= 34 || sample_rate <= 0) return 0;
    const double quantum_ms = wordstr.values[34];
    if (!(quantum_ms > 0.0 && quantum_ms < 1000.0)) return 0;
    const double ms = 2.0 * quantum_ms;
    return static_cast<std::size_t>(std::llround(ms * sample_rate / 1000.0));
}

void append_legacy_pc_terminal_silence(
    Pcm16Mono& pcm, const LegacyWordProsodyProfile& wordstr, int sample_rate) {
    if (pcm.sample_rate <= 0) pcm.sample_rate = sample_rate;
    const auto n = legacy_pc_terminal_silence_samples(wordstr, pcm.sample_rate);
    pcm.samples.insert(pcm.samples.end(), n, 0);
}

} // namespace nicolai
