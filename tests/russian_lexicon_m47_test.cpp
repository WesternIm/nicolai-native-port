#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/russian_frontend.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("M47 contract failed");}
}
int main() try {
    using namespace nicolai;
    require(!decode_russian_exact_candidate_m47({0,1,0}));
    for(unsigned s:{15u,19u,51u,59u,89u,99u,106u,253u,254u,255u})
        require(!decode_russian_exact_candidate_m47({static_cast<std::uint8_t>(s),1,0,0}));
    auto c=decode_russian_exact_candidate_m47({0,2,7,0});
    require(c && c->kind==10 && c->stress==2 && c->auxiliary==7 && c->form==0);
    c=decode_russian_exact_candidate_m47({63,1,0,0});
    require(c && c->kind==6 && c->form==4);
    c=decode_russian_exact_candidate_m47({102,1,0,0});
    require(c && c->kind==1 && c->tag==7 && c->form==4);
    RussianLexiconM44 l;l.valid=true;
    const std::string stem{char(0xa1),char(0xa0),char(0xa1),char(0xa0)}; // invented баба
    l.stems[stem].push_back({stem,{0,1,0,0}});
    require(lookup_russian_lexicon_stress_m47(l,"баба").stress_vowel==0);
    RussianFrontendOptions options;options.lexicon_m44=&l;options.enable_lexicon_stress_m47=true;
    require(russian_text_to_nicolai_phones("баба",options).words[0].stress_source=="legacy-lexicon-m47");
    require(russian_text_to_nicolai_phones("баба",options).words[0].stress_vowel_index==0);
    require(russian_text_to_nicolai_phones("баба́",options).words[0].stress_vowel_index==1);
    RussianStressDictionary exact;exact.valid=true;exact.stress_vowel_by_word["баба"]=1;
    options.stress_dictionary=&exact;
    require(russian_text_to_nicolai_phones("баба",options).words[0].stress_source=="dictionary");
    require(russian_text_to_nicolai_phones("баба",options).words[0].stress_vowel_index==1);
    require(!lookup_russian_lexicon_stress_m44(l,"баба").stress_vowel);
    require(!lookup_russian_lexicon_stress_m47(l,"бабау").stress_vowel); // exact does not inflect
    l.stems[stem].push_back({stem,{0,2,0,0}});
    require(lookup_russian_lexicon_stress_m47(l,"баба").status=="ambiguous");
    l.stems[stem].back().metadata[1]=0;
    require(!lookup_russian_lexicon_stress_m47(l,"баба").stress_vowel);
    l.stems[stem].back().metadata[1]=3; // ordinal outside word
    require(!lookup_russian_lexicon_stress_m47(l,"баба").stress_vowel);
    l.stems.clear();
    // Synthetic full adjective: m[2] is not the stress; all 26 forms share slot 1.
    l.stems[stem].push_back({stem,{204,0,56,1,0,2}});
    l.nonverb_forms[204].resize(26,"**");
    l.nonverb_forms[204][25]=std::string{'<',char(0xe3)};
    const auto row=23*(20*3+2);l.types[row]=1;l.types[row+1]='+';
    require(lookup_russian_lexicon_stress_m47(l,"бабау").stress_vowel==0);
    require(!lookup_russian_lexicon_stress_m44(l,"бабау").stress_vowel);
    l.types[row+1]='-';require(!lookup_russian_lexicon_stress_m47(l,"бабау").stress_vowel);
    l.types[row+1]='+';
    auto competing=l.stems[stem].front();competing.metadata[3]=2;l.stems[stem].push_back(competing);
    require(lookup_russian_lexicon_stress_m47(l,"бабау").status=="ambiguous");
    l.stems[stem].pop_back();
    l.nonverb_forms[204].push_back(std::string(1,char(0xa5))); // cannot read a 27th form
    require(!lookup_russian_lexicon_stress_m47(l,"бабае").stress_vowel);
    l.stems[stem].front().metadata={232,0,53,2,0,1};
    l.nonverb_forms[232]={"#",std::string(1,char(0xa5)),"**","--"};
    const auto short_row=23*(20*4+1);l.types[short_row]=4;l.types[short_row+2]='+';
    require(lookup_russian_lexicon_stress_m47(l,"бабае").stress_vowel==1);
    l.types[short_row+2]='-';require(!lookup_russian_lexicon_stress_m47(l,"бабае").stress_vowel);
    require(!lookup_russian_lexicon_stress_m47(l,"Бабае").stress_vowel);
    require(!lookup_russian_lexicon_stress_m47(l,"бабае!").stress_vowel);
    l.valid=false;require(lookup_russian_lexicon_stress_m47(l,"бабае").status=="invalid-lexicon");
    std::cout<<"M47 exact-selector, adjective, short-form, ambiguity and refusal contracts passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
