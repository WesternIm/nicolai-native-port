#include "nicolai/russian_frontend.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace nicolai {
namespace {

struct InputWord {
    std::vector<std::uint32_t> cps;
    std::optional<std::size_t> explicit_stress_vowel;
    FrontendBoundaryKind boundary_after = FrontendBoundaryKind::Word;
};

bool decode_utf8(const std::string& s, std::vector<std::uint32_t>& out, std::string& err) {
    out.clear();
    for (std::size_t i = 0; i < s.size();) {
        const auto c = static_cast<unsigned char>(s[i]);
        std::uint32_t cp = 0;
        std::size_t n = 0;
        if (c < 0x80) { cp = c; n = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; n = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; n = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; n = 4; }
        else { err = "invalid UTF-8 leading byte"; return false; }
        if (i + n > s.size()) { err = "truncated UTF-8 sequence"; return false; }
        for (std::size_t j = 1; j < n; ++j) {
            const auto cc = static_cast<unsigned char>(s[i+j]);
            if ((cc & 0xC0) != 0x80) { err = "invalid UTF-8 continuation byte"; return false; }
            cp = (cp << 6) | (cc & 0x3F);
        }
        out.push_back(cp);
        i += n;
    }
    return true;
}

void append_utf8(std::string& s, std::uint32_t cp) {
    if (cp <= 0x7F) s.push_back(static_cast<char>(cp));
    else if (cp <= 0x7FF) {
        s.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        s.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        s.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::uint32_t lower_ru(std::uint32_t cp) {
    if (cp >= 0x0410 && cp <= 0x042F) return cp + 0x20;
    if (cp == 0x0401) return 0x0451;
    return cp;
}

bool is_ru_letter(std::uint32_t cp) {
    cp = lower_ru(cp);
    return (cp >= 0x0430 && cp <= 0x044F) || cp == 0x0451;
}

bool is_boundary(std::uint32_t cp) {
    if (cp <= 0x7F) {
        const char c = static_cast<char>(cp);
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
               c == '.' || c == ',' || c == '!' || c == '?' || c == ':' || c == ';' ||
               c == '-' || c == '(' || c == ')' || c == '"' || c == '\'' || c == '/';
    }
    return cp == 0x2013 || cp == 0x2014 || cp == 0x00AB || cp == 0x00BB || cp == 0x2026;
}

FrontendBoundaryKind boundary_kind(std::uint32_t cp) {
    switch (cp) {
        case U',': return FrontendBoundaryKind::Comma;
        case U';': return FrontendBoundaryKind::Semicolon;
        case U':': return FrontendBoundaryKind::Colon;
        case U'.': case 0x2026: return FrontendBoundaryKind::Sentence;
        case U'?': return FrontendBoundaryKind::Question;
        case U'!': return FrontendBoundaryKind::Exclamation;
        case U'-': return FrontendBoundaryKind::Hyphen;
        case 0x2013: case 0x2014: return FrontendBoundaryKind::Dash;
        case U' ': case U'\t': case U'\r': case U'\n': return FrontendBoundaryKind::Word;
        default: return FrontendBoundaryKind::Other;
    }
}

std::string hard_phone(std::uint32_t cp) {
    static const std::unordered_map<std::uint32_t,std::string> m = {
        {U'п',"p"},{U'б',"b"},{U'м',"m"},{U'ф',"f"},{U'в',"v"},
        {U'т',"t"},{U'д',"d"},{U'н',"n"},{U'с',"s"},{U'з',"z"},
        {U'ш',"sh"},{U'ж',"zh"},{U'ц',"c"},{U'щ',"sc"},{U'ч',"ch"},
        {U'л',"l"},{U'р',"r"},{U'й',"j"},{U'к',"k"},{U'г',"g"},{U'х',"x"}
    };
    const auto it = m.find(cp);
    return it == m.end() ? std::string{} : it->second;
}

bool is_vowel_cp(std::uint32_t cp) {
    switch (cp) {
        case U'а': case U'о': case U'у': case U'ы': case U'э': case U'и':
        case U'я': case U'ё': case U'ю': case U'е': return true;
        default: return false;
    }
}

bool is_iotated(std::uint32_t cp) {
    return cp == U'я' || cp == U'ё' || cp == U'ю' || cp == U'е';
}

void soften_previous(std::vector<std::string>& phones) {
    if (phones.empty()) return;
    auto& p = phones.back();
    if (p == "p" || p == "b" || p == "m" || p == "f" || p == "v" ||
        p == "t" || p == "d" || p == "n" || p == "s" || p == "z" ||
        p == "l" || p == "r" || p == "k" || p == "g" || p == "x") p += "'";
}

bool phone_is_soft_context(const std::vector<std::string>& phones) {
    if (phones.empty()) return false;
    const auto& p = phones.back();
    return p == "j" || p == "ch" || p == "sc" || (!p.empty() && p.back() == '\'');
}

std::size_t count_vowels(const std::vector<std::uint32_t>& w) {
    return static_cast<std::size_t>(std::count_if(w.begin(), w.end(), [](auto cp) {
        return is_vowel_cp(lower_ru(cp));
    }));
}

std::string word_utf8(const std::vector<std::uint32_t>& w) {
    std::string s;
    for (auto cp : w) append_utf8(s, lower_ru(cp));
    return s;
}

const std::unordered_map<std::string,std::size_t>& builtin_stress() {
    // Tiny deterministic fallback only. The legacy exc_rus.txt loader is the
    // preferred source when available. Indices count vowels from zero.
    static const std::unordered_map<std::string,std::size_t> m = {
        {"мама",0},{"папа",0},{"яма",0},{"мир",0},{"тётя",0},{"конь",0},
        {"привет",1},{"люблю",1},{"щёлк",0},{"молоко",2},{"объём",1},
        {"вокзал",1},{"сказка",0},{"подписка",1},{"работа",1},{"машина",1},{"это",0}
    };
    return m;
}

std::pair<std::size_t,std::string> resolve_stress(
    const InputWord& w, const std::string& source, const RussianFrontendOptions& options,
    std::optional<std::size_t>& recovered_yo) {
    const auto nv = count_vowels(w.cps);
    if (nv == 0) return {0,"none"};
    if (w.explicit_stress_vowel && *w.explicit_stress_vowel < nv)
        return {*w.explicit_stress_vowel,"explicit"};

    std::size_t vo = 0;
    for (auto cp : w.cps) {
        cp = lower_ru(cp);
        if (!is_vowel_cp(cp)) continue;
        if (cp == U'ё') return {vo,"yo"};
        ++vo;
    }
    if (options.stress_dictionary && options.stress_dictionary->valid) {
        if (auto s = lookup_stress_vowel(*options.stress_dictionary, source); s && *s < nv)
            return {*s,"dictionary"};
    }
    if(options.lexicon_m44) {
        const auto query=options.enable_lexicon_stress_m50 ?
            (options.yo_policy_m49 && options.noun_yo_policy_m50 ? lookup_russian_lexicon_stress_m50(*options.lexicon_m44,*options.yo_policy_m49,*options.noun_yo_policy_m50,source) : RussianLexiconStressM44{}) :
            options.enable_lexicon_stress_m49 ?
            (options.yo_policy_m49 ? lookup_russian_lexicon_stress_m49(*options.lexicon_m44,*options.yo_policy_m49,source) : RussianLexiconStressM44{}) :
            options.enable_lexicon_stress_m48 ?
            lookup_russian_lexicon_stress_m48(*options.lexicon_m44,source) :
            options.enable_lexicon_stress_m47 ?
            lookup_russian_lexicon_stress_m47(*options.lexicon_m44,source) :
            lookup_russian_lexicon_stress_m44(*options.lexicon_m44,source);
        if(query.stress_vowel && *query.stress_vowel<nv &&
           (!query.yo_letter_index || (*query.yo_letter_index<w.cps.size() && w.cps[*query.yo_letter_index]==U'е'))) {
            recovered_yo=query.yo_letter_index;
            return {*query.stress_vowel,options.enable_lexicon_stress_m50?"legacy-lexicon-m50":
                options.enable_lexicon_stress_m49?"legacy-lexicon-m49":
                options.enable_lexicon_stress_m48?"legacy-lexicon-m48":
                options.enable_lexicon_stress_m47?"legacy-lexicon-m47":"legacy-lexicon-m44"};
        }
    }
    if (options.stress_dictionary && options.stress_dictionary->valid) {
        if (options.enable_fixed_ika_stress_m43) {
            if (auto s = lookup_fixed_ika_stress_m43(*options.stress_dictionary, source); s && *s < nv)
                return {*s,"dictionary-ika-m43"};
        }
    }
    if (options.use_builtin_stress_fallback) {
        const auto it = builtin_stress().find(source);
        if (it != builtin_stress().end() && it->second < nv) return {it->second,"builtin"};
    }
    if (nv == 1) return {0,"single-vowel"};
    // Russian lexical stress is not predictable; this fallback exists only to
    // keep the frontend deterministic when no dictionary is supplied.
    return {nv - 1,"heuristic"};
}

enum class VowelPosition {
    Stressed,
    Pretonic1,
    PretonicRemote,
    Posttonic,
    WordFinalUnstressed
};

std::string select_vowel_phone(std::uint32_t cp, bool soft_context,
                               VowelPosition pos, bool reduction, bool word_initial) {
    cp = lower_ru(cp);
    if (!reduction || pos == VowelPosition::Stressed) {
        switch (cp) {
            case U'а': return soft_context ? "A0" : "a0";
            case U'я': return soft_context ? "A0" : "a0";
            case U'о': return soft_context ? "O0" : "o0";
            case U'ё': return soft_context ? "O0" : "o0";
            case U'у': return soft_context ? "U0" : "u0";
            case U'ю': return soft_context ? "U0" : "u0";
            case U'ы': return "y0";
            case U'и': return "i0";
            case U'э': return soft_context ? "E0" : "e0";
            case U'е': return soft_context ? "E0" : "e0";
        }
    }

    const bool pre = pos == VowelPosition::Pretonic1;
    const bool remote_pre = pos == VowelPosition::PretonicRemote;
    const bool post = pos == VowelPosition::Posttonic;
    const bool final = pos == VowelPosition::WordFinalUnstressed;
    switch (cp) {
        // Akanye: unstressed о shares Nicolai's reduced а family.
        case U'а': case U'о':
            if (soft_context) return pre ? "A1" : "i4";
            // Initial unstressed а/о uses the a1 entry form even when stress
            // is remote. a3 remains an interior reduction: # -> a3 is absent
            // from Nicolai's graph. This is frontend selection, not a missing
            // diphone substitution in the renderer.
            if (word_initial) return "a1";
            if (pre) return "a1";
            if (remote_pre) return "a3";
            if (post) return "a5";
            if (final) return "a4";
            return "a4";
        case U'я':
            return pre ? "A1" : "i4";
        case U'ё':
            return soft_context ? "O0" : "o0"; // ё is normally stressed; defensive fallback
        case U'у':
            if (soft_context) return "U4";
            // u4 is an interior reduced form: the recorded voice has # -> u1
            // but no # -> u4. Keep stressed/unreduced у on the full u0 path.
            if (word_initial) return "u1";
            return pre ? "u1" : "u4";
        case U'ю': return "U4";
        case U'ы': return pre ? "y1" : "y4";
        case U'и': return soft_context ? (pre ? "i1" : "i4") : (pre ? "y1" : "y4");
        case U'э': return "e4";
        case U'е': return soft_context ? "E4" : "e4";
    }
    return {};
}

std::vector<std::string> word_to_phones(const std::vector<std::uint32_t>& w,
                                        std::size_t stress_vowel,
                                        bool reduction) {
    std::vector<std::string> out;
    bool force_iotation = true;
    std::size_t vowel_ord = 0;
    for (std::size_t i = 0; i < w.size(); ++i) {
        const auto cp = lower_ru(w[i]);
        if (cp == U'ь') { soften_previous(out); force_iotation = true; continue; }
        if (cp == U'ъ') { force_iotation = true; continue; }
        if (is_vowel_cp(cp)) {
            const bool iot = is_iotated(cp);
            const bool after_vowel_or_sign = force_iotation || (i > 0 && is_vowel_cp(lower_ru(w[i-1])));
            if (cp == U'и') soften_previous(out);
            if (iot) {
                if (after_vowel_or_sign) out.push_back("j");
                else soften_previous(out);
            }
            const bool soft = phone_is_soft_context(out);
            VowelPosition vp = VowelPosition::Posttonic;
            if (vowel_ord == stress_vowel) {
                vp = VowelPosition::Stressed;
            } else if (vowel_ord + 1 == stress_vowel) {
                vp = VowelPosition::Pretonic1;
            } else if (vowel_ord < stress_vowel) {
                vp = VowelPosition::PretonicRemote;
            } else {
                bool only_signs_after = true;
                for (std::size_t j = i + 1; j < w.size(); ++j) {
                    const auto q = lower_ru(w[j]);
                    if (q != U'ь' && q != U'ъ') { only_signs_after = false; break; }
                }
                vp = only_signs_after ? VowelPosition::WordFinalUnstressed
                                      : VowelPosition::Posttonic;
            }
            auto v = select_vowel_phone(cp, soft, vp, reduction, i == 0);
            if (!v.empty()) out.push_back(std::move(v));
            ++vowel_ord;
            force_iotation = false;
            continue;
        }
        const auto hp = hard_phone(cp);
        if (!hp.empty()) {
            out.push_back(hp);
            force_iotation = false;
        }
    }
    return out;
}

const std::unordered_map<std::string,std::string>& devoicing() {
    static const std::unordered_map<std::string,std::string> m = {
        {"b","p"},{"b'","p'"},{"d","t"},{"d'","t'"},{"g","k"},{"g'","k'"},
        {"z","s"},{"z'","s'"},{"zh","sh"},{"v","f"},{"v'","f'"}
    };
    return m;
}
const std::unordered_map<std::string,std::string>& voicing() {
    static const std::unordered_map<std::string,std::string> m = {
        {"p","b"},{"p'","b'"},{"t","d"},{"t'","d'"},{"k","g"},{"k'","g'"},
        {"s","z"},{"s'","z'"},{"sh","zh"},{"f","v"},{"f'","v'"}
    };
    return m;
}

bool is_voiced_obstruent(const std::string& p) { return devoicing().count(p) != 0; }
bool is_voiceless_obstruent(const std::string& p) {
    return voicing().count(p) != 0 || p == "c" || p == "ch" || p == "sc" ||
           p == "C" || p == "CH" || p == "SC" || p == "X";
}
bool is_obstruent(const std::string& p) { return is_voiced_obstruent(p) || is_voiceless_obstruent(p); }

void assimilate_obstruents(std::vector<std::string>& phones) {
    if (phones.empty()) return;
    // Final devoicing.
    if (auto it = devoicing().find(phones.back()); it != devoicing().end()) phones.back() = it->second;

    // Regressive voicing assimilation inside an adjacent obstruent cluster.
    for (std::size_t i = phones.size(); i-- > 1;) {
        auto& left = phones[i - 1];
        const auto& right = phones[i];
        if (!is_obstruent(left) || !is_obstruent(right)) continue;
        if (is_voiceless_obstruent(right)) {
            if (auto it = devoicing().find(left); it != devoicing().end()) left = it->second;
        } else if (right != "v" && right != "v'") {
            if (auto it = voicing().find(left); it != voicing().end()) left = it->second;
        }
    }
}

} // namespace

RussianFrontendResult russian_text_to_nicolai_phones(
    const std::string& utf8_text, const RussianFrontendOptions& options) {
    RussianFrontendResult r;
    if (utf8_text.empty()) { r.error = "text is empty"; return r; }

    std::vector<std::uint32_t> cps;
    if (!decode_utf8(utf8_text, cps, r.error)) return r;

    std::vector<InputWord> words;
    InputWord cur;
    auto flush = [&](FrontendBoundaryKind kind = FrontendBoundaryKind::Word) {
        if (!cur.cps.empty()) {
            cur.boundary_after = kind;
            words.push_back(cur);
            cur = {};
        }
    };

    for (auto cp : cps) {
        cp = lower_ru(cp);
        if (is_ru_letter(cp)) cur.cps.push_back(cp);
        else if (cp == 0x0301) {
            // Combining acute immediately after a vowel explicitly sets stress.
            if (!cur.cps.empty() && is_vowel_cp(lower_ru(cur.cps.back()))) {
                const auto before = static_cast<std::size_t>(std::count_if(
                    cur.cps.begin(), cur.cps.end(), [](auto x){ return is_vowel_cp(lower_ru(x)); }));
                if (before) cur.explicit_stress_vowel = before - 1;
            }
        }
        else if (is_boundary(cp)) flush(boundary_kind(cp));
        else if (cp >= U'0' && cp <= U'9') {
            r.error = "digits are not normalized in M17 yet";
            return r;
        } else flush(FrontendBoundaryKind::Other);
    }
    flush();
    if (words.empty()) { r.error = "no Russian words found"; return r; }

    r.phones.push_back("#");
    for (const auto& w : words) {
        FrontendWord fw;
        fw.source_utf8 = word_utf8(w.cps);
        std::optional<std::size_t> recovered_yo;
        const auto stress = resolve_stress(w, fw.source_utf8, options, recovered_yo);
        fw.stress_vowel_index = static_cast<int>(stress.first);
        fw.stress_source = stress.second;
        auto pronounced=w.cps;
        if(recovered_yo) pronounced[*recovered_yo]=U'ё';
        fw.pronunciation_utf8=word_utf8(pronounced);
        fw.phones = word_to_phones(pronounced, stress.first, options.enable_vowel_reduction);
        if (fw.phones.empty()) { r.error = "word produced no phones: " + fw.source_utf8; return r; }
        if (options.enable_consonant_assimilation) assimilate_obstruents(fw.phones);

        if (!r.normalized_utf8.empty()) r.normalized_utf8.push_back(' ');
        r.normalized_utf8 += fw.source_utf8;
        r.phones.insert(r.phones.end(), fw.phones.begin(), fw.phones.end());
        r.phones.push_back("#");
        r.boundaries.push_back({r.phones.size() - 1, w.boundary_after});
        r.words.push_back(std::move(fw));
    }
    r.valid = true;
    return r;
}

} // namespace nicolai
