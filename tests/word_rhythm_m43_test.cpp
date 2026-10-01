#include "nicolai/word_rhythm_m43.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

int main() try {
    using namespace nicolai;
    auto require=[](bool ok){if(!ok) throw std::runtime_error("word rhythm M43 contract failed");};
    const std::vector<std::string> phones={"#","m","a0","#","p","a4","#"};
    const std::vector<WordRhythmUnitM43> units={
        {160,100,1,1},{100,200,1,1},{200,160,1,1},
        {320,100,0.5,1},{100,200,1,1},{200,640,1,1.4}};
    const std::vector<double> weights={0,1,3,0,1,2,0};
    for(double strength:{0.25,0.5,1.0,2.0}) {
        const auto plan=plan_word_rhythm_m43(phones,units,weights,strength);
        require(plan.valid && plan.words.size()==2);
        for(const auto& word:plan.words) {
            require(std::abs(word.baseline_samples-word.trial_samples)<1e-8);
            require(word.effective_strength>0 && word.effective_strength<=std::min(1.0,strength));
        }
        require(plan.units[0].left_scale==1 && plan.units[2].right_scale==1);
        require(plan.units[3].left_scale==0.5 && plan.units[5].right_scale==1.4);
        const double m=100*plan.units[0].right_scale+100*plan.units[1].left_scale;
        const double a=200*plan.units[1].right_scale+200*plan.units[2].left_scale;
        require(std::abs(m+a-600)<1e-8 && a>400);
        require(std::abs(plan.units[0].right_scale-plan.units[1].left_scale)<1e-12);
    }
    const auto zero=plan_word_rhythm_m43(phones,units,weights,0);
    require(zero.valid && zero.words.empty());
    for(std::size_t i=0;i<units.size();++i)
        require(zero.units[i].left_scale==units[i].left_scale && zero.units[i].right_scale==units[i].right_scale);
    auto missing=weights; missing[2]=0;
    const auto skipped=plan_word_rhythm_m43(phones,units,missing,0.5);
    require(skipped.valid && skipped.skipped_words==1 && skipped.words.size()==1);
    require(skipped.units[0].right_scale==1 && skipped.units[1].left_scale==1);
    require(!plan_word_rhythm_m43(phones,{},weights,0.5).valid);
    require(!plan_word_rhythm_m43(phones,units,weights,std::numeric_limits<double>::quiet_NaN()).valid);
    auto bad=units; bad[1].left_scale=std::numeric_limits<double>::infinity();
    require(!plan_word_rhythm_m43(phones,bad,weights,0.5).valid);
    auto extreme=weights; extreme[1]=100000;
    const auto bounded=plan_word_rhythm_m43(phones,units,extreme,1);
    require(bounded.valid && bounded.words.size()==2);
    for(std::size_t i=0;i<units.size();++i) {
        require(bounded.units[i].left_scale>=units[i].left_scale/1.5-1e-12);
        require(bounded.units[i].left_scale<=units[i].left_scale*1.5+1e-12);
        require(bounded.units[i].right_scale>=units[i].right_scale/1.5-1e-12);
        require(bounded.units[i].right_scale<=units[i].right_scale*1.5+1e-12);
    }
    require(std::abs(bounded.words[0].baseline_samples-bounded.words[0].trial_samples)<1e-8);
    auto different=units;
    different[0].right_scale=0.8; different[1].left_scale=1.2;
    const auto shared=plan_word_rhythm_m43(phones,different,{0,1,2,0,1,2,0},1);
    require(shared.valid && shared.words[0].effective_strength==1);
    require(std::abs(shared.units[0].right_scale-shared.units[1].left_scale)<1e-12);
    require(std::abs(shared.words[0].baseline_samples-shared.words[0].trial_samples)<1e-8);
    const auto single=plan_word_rhythm_m43({"#","a0","#"},{{160,100,1,1},{100,160,1,1}},{0,1,0},1);
    require(single.valid && single.words.empty() && single.skipped_words==1);
    std::cout<<"word rhythm M43 contracts passed\n";
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
