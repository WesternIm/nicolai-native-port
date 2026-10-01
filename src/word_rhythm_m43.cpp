#include "nicolai/word_rhythm_m43.hpp"
#include <algorithm>
#include <cmath>

namespace nicolai {
WordRhythmPlanM43 plan_word_rhythm_m43(const std::vector<std::string>& phones,
    const std::vector<WordRhythmUnitM43>& units,
    const std::vector<double>& weights, double strength) {
    WordRhythmPlanM43 out;
    out.units=units;
    if(phones.size()<2 || units.size()+1!=phones.size() || weights.size()!=phones.size() ||
       !std::isfinite(strength) || strength<0) return out;
    for(const auto& unit:units) {
        for(double n:{unit.left_source_samples,unit.right_source_samples})
            if(!std::isfinite(n) || n<0) return out;
        for(double scale:{unit.left_scale,unit.right_scale})
            if(!std::isfinite(scale) || scale<0.2 || scale>3.0) return out;
    }
    out.valid=true;
    if(strength==0) return out;
    for(std::size_t first=0;first<phones.size();) {
        if(phones[first]=="#") {++first;continue;}
        std::size_t last=first;
        while(last+1<phones.size() && phones[last+1]!="#") ++last;
        std::vector<double> source(last-first+1), baseline(source.size()), target_scale(source.size());
        double budget=0, weight_sum=0;
        bool eligible=last>first;
        for(std::size_t pi=first;pi<=last;++pi) {
            const auto j=pi-first;
            if(pi>0) {
                source[j]+=units[pi-1].right_source_samples;
                baseline[j]+=units[pi-1].right_source_samples*units[pi-1].right_scale;
            }
            if(pi<units.size()) {
                source[j]+=units[pi].left_source_samples;
                baseline[j]+=units[pi].left_source_samples*units[pi].left_scale;
            }
            if(source[j]<=1 || baseline[j]<=0 || !std::isfinite(weights[pi]) || weights[pi]<=0)
                eligible=false;
            budget+=baseline[j]; weight_sum+=weights[pi];
        }
        if(!eligible || !std::isfinite(budget) || !std::isfinite(weight_sum) || weight_sum<=0) {
            ++out.skipped_words; first=last+1; continue;
        }
        double amount=std::min(1.0,strength);
        auto limit=[&](double count,double old,double wanted) {
            if(count<=0) return;
            const double delta=wanted-old;
            const double lower=std::max(0.2,old/1.5), upper=std::min(3.0,old*1.5);
            if(delta>0) amount=std::min(amount,(upper-old)/delta);
            if(delta<0) amount=std::min(amount,(lower-old)/delta);
        };
        for(std::size_t pi=first;pi<=last;++pi) {
            const auto j=pi-first;
            target_scale[j]=budget*(weights[pi]/weight_sum)/source[j];
            if(pi>0) limit(units[pi-1].right_source_samples,units[pi-1].right_scale,target_scale[j]);
            if(pi<units.size()) limit(units[pi].left_source_samples,units[pi].left_scale,target_scale[j]);
        }
        if(!std::isfinite(amount) || amount<=1e-9) {
            ++out.skipped_words; first=last+1; continue;
        }
        double trial=0;
        for(std::size_t pi=first;pi<=last;++pi) {
            const double wanted=target_scale[pi-first];
            if(pi>0 && units[pi-1].right_source_samples>0) {
                const double old=units[pi-1].right_scale;
                out.units[pi-1].right_scale=old+amount*(wanted-old);
                trial+=units[pi-1].right_source_samples*out.units[pi-1].right_scale;
            }
            if(pi<units.size() && units[pi].left_source_samples>0) {
                const double old=units[pi].left_scale;
                out.units[pi].left_scale=old+amount*(wanted-old);
                trial+=units[pi].left_source_samples*out.units[pi].left_scale;
            }
        }
        out.words.push_back({first,last,budget,trial,amount});
        first=last+1;
    }
    return out;
}
}
