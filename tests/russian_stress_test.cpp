#include "nicolai/russian_stress.hpp"
#include <cassert>
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

int main() {
    auto d = nicolai::parse_exc_rus_cp1251(cp1251_fixture());
    assert(d.valid);
    assert(nicolai::lookup_stress_vowel(d, "абажур").value() == 2); // а-а-У
    assert(nicolai::lookup_stress_vowel(d, "абрикос").value() == 2); // а-и-О
    assert(nicolai::lookup_stress_vowel(d, "абрис").value() == 0);   // А-и
    std::cout << "russian_stress_test: PASSED\n";
}
