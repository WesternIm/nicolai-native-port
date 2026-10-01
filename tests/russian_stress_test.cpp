#include "nicolai/russian_stress.hpp"
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static std::vector<std::uint8_t> cp1251_fixture() {
    // CP1251 bytes for:
    // абажур : <абажу<р> /i
    // абрикос : <абрико<с> /i
    // абрис : <а<брис> /i
    const unsigned char raw[] = {
        0xE0,0xE1,0xE0,0xE6,0xF3,0xF0,' ',':',' ','<',0xE0,0xE1,0xE0,0xE6,0xF3,'<',0xF0,'>',' ','/','i','\n',
        0xE0,0xE1,0xF0,0xE8,0xEA,0xEE,0xF1,' ',':',' ','<',0xE0,0xE1,0xF0,0xE8,0xEA,0xEE,'<',0xF1,'>',' ','/','i','\n',
        0xE0,0xE1,0xF0,0xE8,0xF1,' ',':',' ','<',0xE0,'<',0xE1,0xF0,0xE8,0xF1,'>',' ','/','i','\n'
    };
    return {raw, raw + sizeof(raw)};
}

static void require(bool ok) {
    if (!ok) { std::cerr << "Russian stress contract failed\n"; std::exit(EXIT_FAILURE); }
}

int main() {
    auto d = nicolai::parse_exc_rus_cp1251(cp1251_fixture());
    require(d.valid);
    require(nicolai::lookup_stress_vowel(d, "абажур") == 2); // а-а-У
    require(nicolai::lookup_stress_vowel(d, "абрикос") == 2); // а-и-О
    require(nicolai::lookup_stress_vowel(d, "абрис") == 0);   // А-и
    d.stress_vowel_by_word = {{"акустика",1},{"физика",0},{"логика",0},{"рука",1},{"река",1}};
    for (const auto* word : {"акустику","акустике","акустики","акустикой","акустикою"})
        require(nicolai::lookup_fixed_ika_stress_m43(d, word) == 1);
    require(nicolai::lookup_fixed_ika_stress_m43(d, "физику") == 0);
    require(nicolai::lookup_fixed_ika_stress_m43(d, "логику") == 0);
    for (const auto* word : {"руку","реку","неизвестику","техника","акустический","мини-акустику"})
        require(!nicolai::lookup_fixed_ika_stress_m43(d, word));
    d.stress_vowel_by_word["акустика"] = 3; // End-stressed lemma is not the fixed-stem class.
    require(!nicolai::lookup_fixed_ika_stress_m43(d, "акустику"));
    d.stress_vowel_by_word["акустика"] = 1;
    d.stress_vowel_by_word["акустике"] = 2; // Conflicting family refuses unknown forms.
    require(!nicolai::lookup_fixed_ika_stress_m43(d, "акустику"));
    require(nicolai::lookup_fixed_ika_stress_m43(d, "акустике") == 2); // Exact wins.
    d.stress_vowel_by_word.erase("акустике");
    d.stress_vowel_by_word["акустик"] = 2; // Potential masculine homonym disagrees.
    require(!nicolai::lookup_fixed_ika_stress_m43(d, "акустику"));
    d.valid = false;
    require(!nicolai::lookup_fixed_ika_stress_m43(d, "физику"));
    std::cout << "russian_stress_test: PASSED\n";
}
