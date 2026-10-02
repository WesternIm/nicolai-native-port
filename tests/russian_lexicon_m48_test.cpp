#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/russian_frontend.hpp"
#include <iostream>
#include <stdexcept>
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("M48 contract failed");}
}
int main() try {
    using namespace nicolai;
    const std::string stem{char(0xa1),char(0xa0),char(0xa1),char(0xa0)};
    const std::string u(1,char(0xe3)),a(1,char(0xa0)),e(1,char(0xa5));
    auto d=decode_russian_ending_stress_m48(stem,"<"+u+a);
    require(d && d->stress_vowel==2 && !d->needs_yo_selection);
    d=decode_russian_ending_stress_m48(stem,u+a);
    require(d && d->stress_vowel==3);
    d=decode_russian_ending_stress_m48(stem,"<"+u+e);
    require(d && !d->needs_yo_selection); // unselected е does not veto
    d=decode_russian_ending_stress_m48(stem,u+"<"+e+a);
    require(d && d->stress_vowel==3 && d->needs_yo_selection);
    for(const auto& suffix:std::vector<std::string>{"","#","**","--","<",a+"<",a+"<x","<"+u+"<"+a,"x","/"})
        require(!decode_russian_ending_stress_m48(stem,suffix));
    require(!decode_russian_ending_stress_m48("",u));
    require(!decode_russian_ending_stress_m48("x",u));
    require(!decode_russian_ending_stress_m48(std::string(80,char(0xa0)),u+a));
    RussianLexiconM44 l;l.valid=true;
    l.stems[stem].push_back({stem,{47,0,0,2,1,0,2}});
    l.verb_forms[47].resize(14,"**");l.verb_forms[47][3]="<"+u+a;
    auto row=23*(20*6+2);l.types[row]=14;l.types[row+4]='-';
    require(lookup_russian_lexicon_stress_m48(l,"бабауа").stress_vowel==2);
    RussianFrontendOptions options;options.lexicon_m44=&l;
    options.enable_lexicon_stress_m48=true;
    auto front=russian_text_to_nicolai_phones("бабауа",options);
    require(front.valid && front.words[0].stress_source=="legacy-lexicon-m48");
    require(front.words[0].stress_vowel_index==2);
    require(russian_text_to_nicolai_phones("ба́бауа",options).words[0].stress_vowel_index==0);
    RussianStressDictionary exact;exact.valid=true;exact.stress_vowel_by_word["бабауа"]=1;
    options.stress_dictionary=&exact;
    require(russian_text_to_nicolai_phones("бабауа",options).words[0].stress_source=="dictionary");
    require(russian_text_to_nicolai_phones("бабауа",options).words[0].stress_vowel_index==1);
    require(!lookup_russian_lexicon_stress_m47(l,"бабауа").stress_vowel);
    l.types[row+4]='+';
    require(lookup_russian_lexicon_stress_m48(l,"бабауа").stress_vowel==0);
    l.types[row+4]='-';
    // Neither entry order nor an exact entry may overrule a conflict.
    const std::string whole=stem+u+a;
    l.stems[whole].push_back({whole,{0,1,0,0}});
    require(lookup_russian_lexicon_stress_m48(l,"бабауа").status=="ambiguous");
    l.stems[whole].front().metadata[1]=3;
    require(lookup_russian_lexicon_stress_m48(l,"бабауа").stress_vowel==2);
    l.verb_forms[47][3]="<"+e;
    require(!lookup_russian_lexicon_stress_m48(l,"бабае").stress_vowel);
    l.verb_forms[47][3]=u+"/<"+u; // both variants yield the same ordinal
    require(lookup_russian_lexicon_stress_m48(l,"бабау").stress_vowel==2);
    l.verb_forms[47][3]=u+a+"/<"+u+a; // disagreement is not an arbitrary first choice
    require(lookup_russian_lexicon_stress_m48(l,"бабауа").status=="ambiguous");
    l.stems[whole].clear();
    l.verb_forms[47][3]="<"+u+a;
    l.stems[stem].front().metadata[3]=1; // unsupported reflexive lane
    require(!lookup_russian_lexicon_stress_m48(l,"бабауа").stress_vowel);
    l.stems[stem].front().metadata={120,0,1,0,3,253};
    l.nonverb_forms[120].resize(6,"**");l.nonverb_forms[120][4]="<"+a+u;
    row=23*(20*2+3);l.types[row]=6;l.types[row+5]='-';
    require(lookup_russian_lexicon_stress_m48(l,"бабаау").stress_vowel==2);
    require(!lookup_russian_lexicon_stress_m44(l,"бабаау").stress_vowel);
    l.valid=false;require(lookup_russian_lexicon_stress_m48(l,"бабаау").status=="invalid-lexicon");
    std::cout<<"M48 ending ordinals, markers, yo refusal, competing candidates and isolation passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
