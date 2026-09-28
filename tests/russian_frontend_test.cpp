#include "nicolai/russian_frontend.hpp"
#include "nicolai/russian_stress.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

static nicolai::RussianFrontendResult front(const std::string& text) {
    return nicolai::russian_text_to_nicolai_phones(text);
}
static void require(bool ok) {
    if (!ok) {
        std::cerr << "Russian frontend contract failed\n";
        std::exit(EXIT_FAILURE);
    }
}
static void expect(const std::string& text, const std::vector<std::string>& phones) {
    auto r = front(text);
    require(r.valid);
    require(r.phones == phones);
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

    // Word-initial remote-pretonic а/о must not request absent # -> a3.
    // Explicit stress keeps these regressions independent of external data.
    expect("акусти\xCC\x81ку", {"#","a1","k","u1","s","t'","i0","k","u4","#"});
    expect("аппара\xCC\x81т", {"#","a1","p","p","a1","r","a0","t","#"});
    expect("оборо\xCC\x81на", {"#","a1","b","a1","r","o0","n","a4","#"});
    expect("огоро\xCC\x81д", {"#","a1","g","a1","r","o0","t","#"});
    expect("а\xCC\x81том", {"#","a0","t","a5","m","#"});
    nicolai::RussianFrontendOptions unreduced;
    unreduced.enable_vowel_reduction = false;
    auto full = nicolai::russian_text_to_nicolai_phones("акусти\xCC\x81ку", unreduced);
    require(full.valid && full.phones[1] == "a0");

    // Initial reduced у has a recorded # -> u1 entry; # -> u4 is absent.
    // Keep the non-initial and stressed paths independent of this rule.
    auto street = front("улице");
    require(street.valid && street.words[0].stress_source == "heuristic");
    require(street.phones.size() > 2 && street.phones[1] == "u1");
    auto remote_u = front("уговори\xCC\x81л");
    require(remote_u.valid && remote_u.phones[1] == "u1");
    auto stressed_u = front("у\xCC\x81тро");
    require(stressed_u.valid && stressed_u.phones[1] == "u0");
    auto full_u = nicolai::russian_text_to_nicolai_phones("уговори\xCC\x81л", unreduced);
    require(full_u.valid && full_u.phones[1] == "u0");

    // Voicing/devoicing assimilation.
    expect("сказка", {"#","s","k","a0","s","k","a4","#"});
    expect("вокзал", {"#","v","a1","g","z","a0","l","#"});
    expect("подписка", {"#","p","a1","t","p'","i0","s","k","a4","#"});

    // Explicit combining acute overrides heuristics: молоКО vs МОлоко.
    auto explicit_stress = front("мо\xCC\x81локо");
    require(explicit_stress.valid);
    require(explicit_stress.words[0].stress_vowel_index == 0);
    require(explicit_stress.words[0].stress_source == "explicit");
    require(explicit_stress.phones == std::vector<std::string>({"#","m","o0","l","a5","k","a4","#"}));

    // ё is self-stressing without a dictionary.
    auto yo = front("ёлка");
    require(yo.valid && yo.words[0].stress_vowel_index == 0 && yo.words[0].stress_source == "yo");

    auto punct = front("привет, Николай!");
    require(punct.valid);
    require(punct.boundaries.size() == 2);
    require(punct.boundaries[0].kind == nicolai::FrontendBoundaryKind::Comma);
    require(punct.boundaries[1].kind == nicolai::FrontendBoundaryKind::Exclamation);

    auto bad = front("123");
    require(!bad.valid);
    std::cout << "russian_frontend_test: PASSED\n";
}
