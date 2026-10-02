#include "nicolai/russian_frontend.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok){if(!ok) throw std::runtime_error("M50 noun contract failed");}
void checksum(std::vector<std::uint8_t>& b) {
    std::uint32_t crc=0xffffffffu;
    for(std::size_t i=48;i<b.size();++i) {
        crc^=b[i];for(unsigned k=0;k<8;++k) crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));
    }
    crc=~crc;for(unsigned i=0;i<4;++i) b[44+i]=(crc>>(8*i))&255;
}
std::vector<std::uint8_t> fixture() {
    std::vector<std::uint8_t> b(1848);const std::string magic("N50NOUN\0",8);
    std::copy(magic.begin(),magic.end(),b.begin());
    const std::string sha="f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7";
    for(unsigned i=0;i<32;++i) b[8+i]=std::stoul(sha.substr(2*i,2),nullptr,16);
    b[40]=8;b[41]=7;
    // Invented members, not original table rows.
    b[48+9*4]=1;b[49+9*4]=1;checksum(b);return b;
}
}
int main() try {
    using namespace nicolai;
    auto data=fixture();auto p=parse_russian_noun_yo_policy_m50(data);
    require(p.valid && noun_yo_form_member_m50(p,1,7,1)==true);
    require(noun_yo_form_member_m50(p,1,7,2)==false);
    require(noun_yo_form_member_m50(p,1,3,1)==false);
    require(noun_yo_form_member_m50(p,2,202,6)==false);
    for(auto args:{std::array<unsigned,3>{1,1,1},{1,2,1},{1,100,1},{2,99,1},
                  {2,203,1},{1,7,0},{1,7,9},{2,100,7},{3,7,1}})
        require(!noun_yo_form_member_m50(p,args[0],args[1],args[2]));
    require(!parse_russian_noun_yo_policy_m50({}).valid);
    require(!load_russian_noun_yo_policy_m50("nicolai-no-such-noun-fixture.bin").valid);
    for(unsigned fault=0;fault<10;++fault) {
        auto bad=fixture();
        switch(fault) {
        case 0:bad.pop_back();break;case 1:bad.push_back(0);break;
        case 2:bad[0]^=1;break;case 3:bad[8]^=1;break;case 4:bad[42]=1;break;
        case 5:bad[44]^=1;break;case 6:bad[48]=9;checksum(bad);break;
        case 7:bad[48]=2;bad[49]=bad[50]=3;checksum(bad);break;
        case 8:bad[48+9*97]=1;bad[49+9*97]=7;checksum(bad);break;
        case 9:bad[49]=1;checksum(bad);break;
        }
        require(!parse_russian_noun_yo_policy_m50(bad).valid);
    }
    RussianYoPolicyM49 yo;yo.valid=true;yo.paradigms[1]={7};yo.types[1]={2};
    RussianLexiconM44 lex;lex.valid=true;
    const std::string stem{char(0xa1),char(0xa0),char(0xa1),char(0xa0)};
    lex.stems[stem].push_back({stem,{7,0,1,0,2,252}});
    lex.nonverb_forms[7].resize(8,"**");lex.nonverb_forms[7][0]="<"+std::string(1,char(0xa5));
    const auto row=23*(20+2);lex.types[row]=8;lex.types[row+1]='-';
    require(!lookup_russian_lexicon_stress_m49(lex,yo,"бабае").stress_vowel);
    const auto query=lookup_russian_lexicon_stress_m50(lex,yo,p,"бабае");
    require(query.stress_vowel==2 && query.yo_letter_index==4);
    RussianFrontendOptions options;options.lexicon_m44=&lex;options.yo_policy_m49=&yo;
    options.noun_yo_policy_m50=&p;options.enable_lexicon_stress_m50=true;
    const auto front=russian_text_to_nicolai_phones("бабае",options);
    require(front.valid && front.words[0].stress_source=="legacy-lexicon-m50");
    require(front.words[0].source_utf8=="бабае" && front.words[0].pronunciation_utf8=="бабаё");
    require(front.phones==russian_text_to_nicolai_phones("бабаё").phones);
    require(russian_text_to_nicolai_phones("баба́е",options).words[0].pronunciation_utf8=="бабае");
    RussianStressDictionary dictionary;dictionary.valid=true;dictionary.stress_vowel_by_word["бабае"]=0;
    options.stress_dictionary=&dictionary;
    require(russian_text_to_nicolai_phones("бабае",options).words[0].stress_source=="dictionary");
    require(russian_text_to_nicolai_phones("бабае",options).words[0].pronunciation_utf8=="бабае");
    p.forms[4].clear();const auto negative=lookup_russian_lexicon_stress_m50(lex,yo,p,"бабае");
    require(negative.stress_vowel==2 && !negative.yo_letter_index);
    p=parse_russian_noun_yo_policy_m50(data);
    lex.stems[stem+char(0xa5)].push_back({stem+char(0xa5),{0,3,0,0}});
    require(lookup_russian_lexicon_stress_m50(lex,yo,p,"бабае").status=="ambiguous");
    p.valid=false;require(lookup_russian_lexicon_stress_m50(lex,yo,p,"бабае").status=="invalid-yo-policy");
    std::cout<<"M50 noun ownership, policy guards, spelling consensus and route passed\n";
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
