#include "nicolai/russian_legacy.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main(){
    // Small CP1251 abbreviation fixture: МБ -> мегабайт, IP -> АйПи<
    const unsigned char abb_raw[] = {
        0xCC,0xC1,'\t',0xEC,0xE5,0xE3,0xE0,0xE1,0xE0,0xE9,0xF2,'\n',
        'I','P','\t',0xC0,0xE9,0xCF,0xE8,'<','\n'
    };
    auto abb=nicolai::parse_abb_rus_cp1251({abb_raw,abb_raw+sizeof(abb_raw)});
    assert(abb.valid && abb.entries==2);

    // CP1251 exception fixture: все будет -> всё будет; абажур with stress.
    const unsigned char exc_raw[] = {
        0xE2,0xF1,0xE5,' ',0xE1,0xF3,0xE4,0xE5,0xF2,' ',':',' ','<',0xE2,0xF1,0xB8,'#','#',0xE1,0xF3,0xE4,0xE5,0xF2,'>',' ','/','i','\n',
        0xE0,0xE1,0xE0,0xE6,0xF3,0xF0,' ',':',' ','<',0xE0,0xE1,0xE0,0xE6,0xF3,'<',0xF0,'>',' ','/','i','\n'
    };
    auto exc=nicolai::parse_exc_rus_replacements_cp1251({exc_raw,exc_raw+sizeof(exc_raw)});
    assert(exc.valid && exc.entries==2 && exc.multiword_entries==1);

    auto r=nicolai::normalize_russian_legacy_text("МБ 123 все будет абажур",&abb,&exc);
    assert(r.valid);
    assert(r.abbreviation_replacements==1);
    assert(r.number_replacements==1);
    assert(r.exception_replacements>=2);
    assert(r.normalized_utf8.find("мегабайт")!=std::string::npos);
    assert(r.normalized_utf8.find("сто двадцать три")!=std::string::npos);
    assert(r.normalized_utf8.find("всё будет")!=std::string::npos);
    // U+0301 combining acute from the legacy '<' stress mark.
    assert(r.normalized_utf8.find("\xCC\x81")!=std::string::npos);
    std::cout<<"russian_legacy_test: PASSED\n";
}
