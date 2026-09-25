#include "nicolai/russian_frontend.hpp"
#include "nicolai/russian_stress.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

static nicolai::RussianFrontendResult front(const std::string& text) {
    return nicolai::russian_text_to_nicolai_phones(text);
}
static void expect(const std::string& text, const std::vector<std::string>& phones) {
    auto r = front(text);
    assert(r.valid);
    assert(r.phones == phones);
}

int main() {
    // Builtin stress fallback + M17 allophones.
    expect("мама", {"#","m","a0","m","a4","#"});
    expect("папа", {"#","p","a0","p","a4","#"});
    expect("мир", {"#","m'","i0","r","#"});
    expect("тётя", {"#","t'","O0","t'","i4","#"});
    expect("яма", {"#","j","A0","m","a4","#"});
    expect("молоко", {"#","m","a3","l","a1","k","o0","#"});
    expect("привет", {"#","p","r'","i1","v'","E0","t","#"});
    expect("объём", {"#","a1","b","j","O0","m","#"});
    expect("это", {"#","e0","t","a4","#"});

    // Voicing/devoicing assimilation.
    expect("сказка", {"#","s","k","a0","s","k","a4","#"});
    expect("вокзал", {"#","v","a1","g","z","a0","l","#"});
    expect("подписка", {"#","p","a1","t","p'","i0","s","k","a4","#"});

    // Explicit combining acute overrides heuristics: молоКО vs МОлоко.
    auto explicit_stress = front("мо\xCC\x81локо");
    assert(explicit_stress.valid);
    assert(explicit_stress.words[0].stress_vowel_index == 0);
    assert(explicit_stress.words[0].stress_source == "explicit");
    assert(explicit_stress.phones == std::vector<std::string>({"#","m","o0","l","a5","k","a4","#"}));

    // ё is self-stressing without a dictionary.
    auto yo = front("ёлка");
    assert(yo.valid && yo.words[0].stress_vowel_index == 0 && yo.words[0].stress_source == "yo");

    auto punct = front("привет, Николай!");
    assert(punct.valid);
    assert(punct.boundaries.size() == 2);
    assert(punct.boundaries[0].kind == nicolai::FrontendBoundaryKind::Comma);
    assert(punct.boundaries[1].kind == nicolai::FrontendBoundaryKind::Exclamation);

    auto bad = front("123");
    assert(!bad.valid);
    std::cout << "russian_frontend_test: PASSED\n";
}
