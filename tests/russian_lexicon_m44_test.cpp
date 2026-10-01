#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/russian_frontend.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("lexicon M44 contract failed");}
void put16(std::vector<std::uint8_t>& b,std::size_t o,unsigned v) {b[o]=v&255;b[o+1]=v>>8;}
void put32(std::vector<std::uint8_t>& b,std::size_t o,unsigned v) {put16(b,o,v&65535);put16(b,o+2,v>>16);}
void flx(std::vector<std::uint8_t>& b,std::vector<std::uint8_t>& index,unsigned header,
         const std::vector<std::string>& forms) {
    b.assign(header,0);index.assign(1024,0);
    for(unsigned i=1;i<256;++i) {put16(b,2*(i-1),header);put32(index,4*i,header);}
    for(const auto& form:forms) {b.push_back(static_cast<std::uint8_t>(form.size()));b.insert(b.end(),form.begin(),form.end());}
    b.resize(b.size()+10,0);
}
nicolai::RussianLexiconPayloadsM44 fixture() {
    nicolai::RussianLexiconPayloadsM44 p;
    p.dictionary.assign(2000000,0);p.index.assign(3520,0);p.types.assign(3680,0);
    // Invented two-vowel stems, not copied proprietary records.
    std::vector<std::vector<std::uint8_t>> records={
        {8,4,0xa0,0xa0,0xa0,0xa0,1,0,1}, // Invariable/unsupported tail.
        {11,4,0xa0,0xa0,0xa0,0xa0,1,0,1,0,1,252},
        {12,4,0xa1,0xa0,0xa1,0xa0,1,0,0,2,1,0,2}};
    std::size_t at=2*(records.size()+1);
    for(std::size_t i=0;i<records.size();++i) {
        put16(p.dictionary,2*i,static_cast<unsigned>(at));
        std::copy(records[i].begin(),records[i].end(),p.dictionary.begin()+at);at+=records[i].size();
    }
    put16(p.dictionary,2*records.size(),static_cast<unsigned>(at));
    p.index[0]='x';put32(p.index,40,static_cast<unsigned>(records.size()));p.index[44]=255;
    flx(p.nonverb,p.nonverb_index,510,{"#",std::string(1,char(0xe3)),"#","#","#","#","#","**"});
    flx(p.verb,p.verb_index,512,{"x","y","z","q",std::string{char(0xa5),char(0xac)},"w","v","b","c","d","f","g","h","j"});
    for(auto pair:{std::pair<unsigned,unsigned>{1,1},{6,2}}) {
        const auto row=23*(20*pair.first+pair.second);
        const unsigned n=pair.first==1?8:14;p.types[row]=n;
        std::fill(p.types.begin()+row+1,p.types.begin()+row+n+1,'+');
    }
    return p;
}
}
int main() try {
    using namespace nicolai;
    auto p=fixture();auto l=parse_russian_lexicon_payloads_m44(p);
    require(l.valid && l.blocks==1 && l.records==3);
    require(lookup_russian_lexicon_stress_m44(l,"бабаем").stress_vowel==0);
    RussianFrontendOptions options;options.lexicon_m44=&l;
    auto front=russian_text_to_nicolai_phones("бабаем",options);
    require(front.valid && front.words[0].stress_vowel_index==0 && front.words[0].stress_source=="legacy-lexicon-m44");
    require(russian_text_to_nicolai_phones("бабаем").words[0].stress_vowel_index==2);
    require(russian_text_to_nicolai_phones("баба́ем",options).words[0].stress_vowel_index==1);
    RussianStressDictionary exact;exact.valid=true;exact.stress_vowel_by_word["бабаем"]=1;
    options.stress_dictionary=&exact;
    require(russian_text_to_nicolai_phones("бабаем",options).words[0].stress_source=="dictionary");
    require(russian_text_to_nicolai_phones("бабаем",options).words[0].stress_vowel_index==1);
    require(lookup_russian_lexicon_stress_m44(l,"аааау").stress_vowel==0);
    require(!lookup_russian_lexicon_stress_m44(l,"аааа").stress_vowel); // competing unsupported exact record
    for(const auto* word:{"","Бабаем","x","мини-бабаем","бабаемся","бабаем́","бабаем!"})
        require(!lookup_russian_lexicon_stress_m44(l,word).stress_vowel);
    auto& e=l.stems[std::string{char(0xa1),char(0xa0),char(0xa1),char(0xa0)}];
    auto conflicting=e.front();conflicting.metadata[4]=2;e.push_back(conflicting);
    require(lookup_russian_lexicon_stress_m44(l,"бабаем").status=="ambiguous");
    e.pop_back();e.front().metadata[3]=1; // reflexive-only record cannot accept non-reflexive form
    require(!lookup_russian_lexicon_stress_m44(l,"бабаем").stress_vowel);
    l=parse_russian_lexicon_payloads_m44(p);
    l.types[23*(20*6+2)+5]='-';
    require(lookup_russian_lexicon_stress_m44(l,"бабаем").status=="unsupported-candidate");
    l=parse_russian_lexicon_payloads_m44(p);l.verb_forms[1][4]=std::string{'<',char(0xa5),char(0xac)};
    require(!lookup_russian_lexicon_stress_m44(l,"бабаем").stress_vowel);
    for(unsigned fault=0;fault<8;++fault) {
        auto bad=p;
        switch(fault) {
        case 0:bad.dictionary.pop_back();break;
        case 1:put16(bad.dictionary,0,7);break;
        case 2:bad.dictionary[8]=255;break;
        case 3:put32(bad.index,40,8192);break;
        case 4:put32(bad.verb_index,4,511);break;
        case 5:bad.types[23*(20*6+2)+5]='?';break;
        case 6:bad.index[44]=0;break;
        case 7:bad.nonverb.back()=1;break;
        }
        const auto r=parse_russian_lexicon_payloads_m44(bad);
        require(!r.valid && r.records==0 && r.stems.empty() && !r.error.empty());
    }
    require(!parse_russian_lexicon_m44({},{}).valid);
    auto inactive=p;put16(inactive.verb,2*148,0xffff);put32(inactive.verb_index,4*149,0xffff);
    require(parse_russian_lexicon_payloads_m44(inactive).valid); // unused legacy slot is not a pointer
    std::cout<<"Russian lexicon M44 bounded parser and conservative stress contracts passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
