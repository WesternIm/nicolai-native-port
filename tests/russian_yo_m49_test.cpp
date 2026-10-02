#include "nicolai/russian_yo_m49.hpp"
#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/russian_frontend.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("M49 contract failed");}
void checksum(std::vector<std::uint8_t>& b) {
    std::uint32_t crc=0xffffffffu;
    for(std::size_t i=48;i<b.size();++i) {
        crc^=b[i];
        for(unsigned bit=0;bit<8;++bit) crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));
    }
    crc=~crc;for(unsigned i=0;i<4;++i) b[44+i]=(crc>>(8*i))&255;
}
std::vector<std::uint8_t> fixture() {
    std::vector<std::uint8_t> b(536);
    const std::string magic("N49YOv1\0",8);
    std::copy(magic.begin(),magic.end(),b.begin());
    const std::string sha="f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7";
    for(unsigned i=0;i<32;++i) b[8+i]=std::stoul(sha.substr(2*i,2),nullptr,16);
    b[40]=0xe8;b[41]=1;
    // Invented selector members, not original data.
    const auto at=48+61*6;b[at]=1;b[at+1]=47;b[at+30]=1;b[at+31]=2;
    checksum(b);return b;
}
}
int main() try {
    using namespace nicolai;
    require(!parse_russian_yo_policy_m49({}).valid);
    require(!parse_russian_yo_policy_m49(std::vector<std::uint8_t>(536)).valid);
    require(!load_russian_yo_policy_m49("nicolai-no-such-policy-fixture.bin").valid);
    auto data=fixture();
    auto parsed=parse_russian_yo_policy_m49(data);
    require(parsed.valid && parsed.paradigms[6]==std::vector<std::uint8_t>{47});
    data[8]^=1;require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data[44]^=1;require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data.push_back(0);require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data[48]=30;checksum(data);require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data[48]=2;data[49]=data[50]=7;checksum(data);require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data[48+30]=1;data[48+31]=20;checksum(data);require(!parse_russian_yo_policy_m49(data).valid);data=fixture();
    data[49]=7;checksum(data);require(!parse_russian_yo_policy_m49(data).valid);
    RussianYoPolicyM49 p;p.valid=true;
    p.paradigms[6]={27,30,32,33,47};p.types[6]={2};
    p.paradigms[7]={93};p.types[7]={2};
    p.paradigms[1]={7};p.types[1]={2};
    require(select_russian_yo_m49(p,6,47,2,4)==true);
    require(select_russian_yo_m49(p,6,47,3,4)==false);
    require(select_russian_yo_m49(p,6,48,2,4)==false);
    for(unsigned q:{30u,32u,33u}) for(unsigned f:{1u,8u,11u})
        require(select_russian_yo_m49(p,6,q,2,f)==false);
    for(unsigned f:{12u,13u}) require(select_russian_yo_m49(p,6,27,2,f)==false);
    require(select_russian_yo_m49(p,6,27,2,14)==true);
    for(unsigned f:{1u,78u,105u,108u,113u}) require(select_russian_yo_m49(p,7,93,2,f)==false);
    for(unsigned f:{79u,104u,109u,112u}) require(select_russian_yo_m49(p,7,93,2,f)==true);
    require(!select_russian_yo_m49(p,1,7,2,1));
    require(select_russian_yo_m49(p,1,7,2,1,false)==false);
    require(select_russian_yo_m49(p,1,7,2,1,true)==true);
    require(select_russian_yo_m49(p,1,8,2,1)==false);
    require(!select_russian_yo_m49(p,6,47,2,15));
    require(!select_russian_yo_m49(p,6,47,20,4));
    const std::string stem{char(0xa1),char(0xa0),char(0xa1),char(0xa0)};
    const std::string e(1,char(0xa5)),a(1,char(0xa0));
    auto ending=decode_russian_ending_choice_m49(stem,"<"+e+a,p,6,47,2,4);
    require(ending && ending->stress_vowel==2 && ending->yo_letter_index==4);
    ending=decode_russian_ending_choice_m49(stem,a+"<"+e,p,6,47,2,4);
    require(ending && ending->stress_vowel==3 && ending->yo_letter_index==5);
    ending=decode_russian_ending_choice_m49(stem,e+a,p,6,47,2,4);
    require(ending && ending->stress_vowel==3 && !ending->yo_letter_index); // last vowel clears earlier ё
    RussianLexiconM44 l;l.valid=true;
    l.stems[stem].push_back({stem,{47,0,0,2,1,0,2}});
    l.verb_forms[47].resize(14,"**");l.verb_forms[47][3]="<"+e;
    const auto row=23*(20*6+2);l.types[row]=14;l.types[row+4]='-';
    auto query=lookup_russian_lexicon_stress_m49(l,p,"бабае");
    require(query.stress_vowel==2 && query.yo_letter_index==4);
    require(!lookup_russian_lexicon_stress_m48(l,"бабае").stress_vowel);
    RussianFrontendOptions options;options.lexicon_m44=&l;options.yo_policy_m49=&p;
    options.enable_lexicon_stress_m49=true;
    const auto front=russian_text_to_nicolai_phones("бабае",options);
    require(front.valid && front.words[0].source_utf8=="бабае" && front.words[0].pronunciation_utf8=="бабаё");
    require(front.words[0].stress_source=="legacy-lexicon-m49");
    require(front.phones==russian_text_to_nicolai_phones("бабаё").phones);
    require(russian_text_to_nicolai_phones("баба́е",options).words[0].pronunciation_utf8=="бабае");
    RussianStressDictionary exact;exact.valid=true;exact.stress_vowel_by_word["бабае"]=0;
    options.stress_dictionary=&exact;
    require(russian_text_to_nicolai_phones("бабае",options).words[0].pronunciation_utf8=="бабае");
    require(russian_text_to_nicolai_phones("бабае",options).words[0].stress_source=="dictionary");
    // Same stress but opposite е/ё choices remain ambiguous.
    l.stems[stem+e].push_back({stem+e,{0,3,0,0}});
    require(lookup_russian_lexicon_stress_m49(l,p,"бабае").status=="ambiguous");
    std::cout<<"M49 selector, form exclusions, pronunciation consensus and precedence passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
