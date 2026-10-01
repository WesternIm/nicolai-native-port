#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/address_space.hpp"
#include "nicolai/static_files.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>

namespace nicolai {
namespace {
std::uint16_t u16(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o > b.size() || b.size()-o < 2) throw std::runtime_error("lexicon_u16_bounds");
    return b[o] | (std::uint16_t(b[o+1]) << 8);
}
std::uint32_t u32(const std::vector<std::uint8_t>& b, std::size_t o) {
    return u16(b,o) | (std::uint32_t(u16(b,o+2)) << 16);
}
std::string fold(std::string s) {
    for (auto& c:s) {
        const auto v=static_cast<unsigned char>(c);
        if (v>=0x80 && v<=0x8f) c=static_cast<char>(v+0x20);
        else if(v>=0x90 && v<=0x9f) c=static_cast<char>(v+0x50);
        else if(v==0xf0) c=static_cast<char>(0xf1);
    }
    return s;
}
bool vowel(unsigned char c) {
    switch(c) {
    case 0xa0: case 0xa5: case 0xa8: case 0xae: case 0xe3:
    case 0xeb: case 0xed: case 0xee: case 0xef: case 0xf1: return true;
    default:return false;
    }
}
std::size_t vowels(const std::string& s) {
    return static_cast<std::size_t>(std::count_if(s.begin(),s.end(),[](unsigned char c){return vowel(c);}));
}
std::optional<std::string> cp866_word(const std::string& s) {
    std::string out;
    for(std::size_t i=0;i<s.size();i+=2) {
        if(i+1>=s.size()) return std::nullopt;
        const auto a=static_cast<unsigned char>(s[i]),b=static_cast<unsigned char>(s[i+1]);
        if((a!=0xd0 && a!=0xd1) || (b&0xc0)!=0x80) return std::nullopt;
        const auto cp=((a&0x1f)<<6)|(b&0x3f);
        if(cp>=0x430 && cp<=0x43f) out.push_back(static_cast<char>(0xa0+cp-0x430));
        else if(cp>=0x440 && cp<=0x44f) out.push_back(static_cast<char>(0xe0+cp-0x440));
        else if(cp==0x451) out.push_back(static_cast<char>(0xf1));
        else return std::nullopt;
    }
    if(out.empty() || out.size()>80) return std::nullopt;
    return out;
}
std::array<std::vector<std::string>,256> parse_forms(
    const std::vector<std::uint8_t>& b, const std::vector<std::uint8_t>& index,
    std::size_t header, const std::set<unsigned>& active) {
    if(b.size()<header || index.size()!=1024 || u32(index,0)!=0 || u16(b,0)!=header)
        throw std::runtime_error("lexicon_flx_header");
    std::set<std::size_t> ends{b.size()};
    for(std::size_t i=1;i<256;++i) {
        const auto at=u32(index,4*i);
        if(at!=u16(b,2*(i-1)))
            throw std::runtime_error("lexicon_flx_index");
        if(active.count(static_cast<unsigned>(i))) {
            if(at<header || at>=b.size()) throw std::runtime_error("lexicon_active_flx_bounds");
            ends.insert(at);
        }
    }
    std::array<std::vector<std::string>,256> result;
    for(std::size_t i=1;i<256;++i) {
        if(!active.count(static_cast<unsigned>(i))) continue;
        std::size_t at=u32(index,4*i);
        const auto end=*ends.upper_bound(at);
        while(at<end) {
            const auto n=b[at++];
            if(n==0) { // Final allocation padding is not a suffix record.
                if(std::any_of(b.begin()+at,b.begin()+end,[](auto v){return v!=0;}))
                    throw std::runtime_error("lexicon_flx_padding");
                break;
            }
            if(n>end-at) throw std::runtime_error("lexicon_flx_record_bounds");
            result[i].emplace_back(reinterpret_cast<const char*>(b.data()+at),n);
            at+=n;
        }
    }
    return result;
}
std::vector<std::uint8_t> payload(const std::vector<std::uint8_t>& b,
    const EdatLayout& l, const std::vector<StaticFileRecord>& records,
    const std::string& name, std::size_t size) {
    const auto* record=find_static_file_record(records,"rusvox\\data\\"+name);
    if(!record || !record->has_field_10c_ref || !record->field_10c_target)
        throw std::runtime_error("lexicon_missing_resource_"+name);
    const auto ref=resolve_static_ref({make_edat_range_descriptor(l)},StaticDuration::Initialization,
        kEdatTaggedRefMagic,record->field_10c_target);
    if(!ref.valid || ref.file_offset>l.segment0_end || size>l.segment0_end-ref.file_offset ||
       ref.file_offset>b.size() || size>b.size()-ref.file_offset)
        throw std::runtime_error("lexicon_resource_bounds_"+name);
    return {b.begin()+static_cast<std::size_t>(ref.file_offset),
            b.begin()+static_cast<std::size_t>(ref.file_offset)+size};
}
} // namespace

RussianLexiconM44 parse_russian_lexicon_payloads_m44(const RussianLexiconPayloadsM44& p) {
    RussianLexiconM44 out;
    try {
        if(p.index.size()!=3520 || p.dictionary.size()!=2000000 || p.types.size()!=3680)
            throw std::runtime_error("lexicon_payload_size");
        for(std::size_t o=0;o<p.types.size();o+=23) {
            const auto n=p.types[o];
            if(n>22) throw std::runtime_error("lexicon_type_count");
            for(std::size_t i=1;i<=n;++i)
                if(p.types[o+i]!='+' && p.types[o+i]!='-')
                    throw std::runtime_error("lexicon_type_sign");
            for(std::size_t i=n+1;i<23;++i)
                if(p.types[o+i]) throw std::runtime_error("lexicon_type_padding");
        }
        std::copy(p.types.begin(),p.types.end(),out.types.begin());
        bool sentinel=false;
        for(std::size_t block=0;block<80;++block) {
            const auto io=44*block;
            if(p.index[io]==255) {sentinel=true;break;}
            const auto zero=std::find(p.index.begin()+io,p.index.begin()+io+40,0);
            if(zero==p.index.begin()+io+40) throw std::runtime_error("lexicon_index_string");
            const auto count=u32(p.index,io+40);
            const auto start=16384*block;
            if(!count || count>=8192 || u16(p.dictionary,start)!=2*(count+1))
                throw std::runtime_error("lexicon_block_header");
            for(std::size_t i=0;i<count;++i) {
                const auto begin=u16(p.dictionary,start+2*i),end=u16(p.dictionary,start+2*(i+1));
                if(begin<2*(count+1) || end<=begin || end>16384)
                    throw std::runtime_error("lexicon_record_bounds");
                const auto at=start+begin;
                const auto n=p.dictionary[at+1];
                if(end-begin!=p.dictionary[at]+1 || n+2>end-begin || n>40)
                    throw std::runtime_error("lexicon_record_length");
                RussianLexiconEntryM44 e;
                e.stem_cp866=fold(std::string(reinterpret_cast<const char*>(p.dictionary.data()+at+2),n));
                e.metadata.assign(p.dictionary.begin()+at+2+n,p.dictionary.begin()+start+end);
                e.block=block;e.record=i;
                out.stems[e.stem_cp866].push_back(std::move(e));
                ++out.records;
            }
            ++out.blocks;
        }
        if(!sentinel || !out.records) throw std::runtime_error("lexicon_index_sentinel");
        std::set<unsigned> nonverb_active,verb_active;
        for(const auto& pair:out.stems) for(const auto& e:pair.second) {
            auto* active=e.metadata.size()==6?&nonverb_active:e.metadata.size()==7?&verb_active:nullptr;
            if(active) for(unsigned i:{unsigned(e.metadata[0]),unsigned(e.metadata[1])}) if(i) active->insert(i);
        }
        // Unused FLX slots contain legacy allocator residue. Only referenced
        // paradigms have payload semantics; inactive slots must not split a list.
        out.nonverb_forms=parse_forms(p.nonverb,p.nonverb_index,510,nonverb_active);
        out.verb_forms=parse_forms(p.verb,p.verb_index,512,verb_active);
        out.valid=true;
    } catch(const std::exception& e) { out={};out.error=e.what(); }
    return out;
}
RussianLexiconM44 parse_russian_lexicon_m44(const std::vector<std::uint8_t>& b,const EdatLayout& l) {
    try {
        if(!l.valid) throw std::runtime_error("lexicon_edat_invalid");
        const auto records=scan_static_file_records(b,l);
        RussianLexiconPayloadsM44 p;
        p.dictionary=payload(b,l,records,"d.dat",2000000);
        p.index=payload(b,l,records,"d.ind",3520);
        p.nonverb=payload(b,l,records,"non_verb1.flx",0x294a);
        p.nonverb_index=payload(b,l,records,"non_verb2.flx",1024);
        p.verb=payload(b,l,records,"vverb1.flx",0xd5c8);
        p.verb_index=payload(b,l,records,"vverb2.flx",1024);
        // types.num owns the entire shared allocation, including types.vrb.
        // Other types.* file objects have null data refs, not separate blobs.
        p.types=payload(b,l,records,"types.num",3680);
        return parse_russian_lexicon_payloads_m44(p);
    } catch(const std::exception& e) {RussianLexiconM44 out;out.error=e.what();return out;}
}

RussianLexiconStressM44 lookup_russian_lexicon_stress_m44(
    const RussianLexiconM44& l,const std::string& word) {
    RussianLexiconStressM44 out;
    if(!l.valid) {out.status="invalid-lexicon";return out;}
    const auto encoded=cp866_word(word);
    if(!encoded) {out.status="invalid-word";return out;}
    bool unresolved=false,eligible=false;
    std::set<std::size_t> stresses;
    for(std::size_t split=1;split<=encoded->size();++split) {
        const auto stem=encoded->substr(0,split),suffix=encoded->substr(split);
        const auto entries=l.stems.find(stem);
        if(entries==l.stems.end()) continue;
        for(const auto& e:entries->second) {
            const auto& m=e.metadata;
            auto scan=[&](bool verb,unsigned paradigm,std::size_t count,unsigned kind,
                          unsigned type,unsigned stress,bool supported) {
                if(!paradigm || paradigm>=256) return;
                const auto& forms=verb?l.verb_forms[paradigm]:l.nonverb_forms[paradigm];
                for(std::size_t f=0;f<std::min(count,forms.size());++f) {
                    const auto& form=forms[f];
                    std::size_t cursor=0;
                    while(cursor<=form.size()) {
                        auto end=form.find('/',cursor);if(end==std::string::npos) end=form.size();
                        auto variant=form.substr(cursor,end-cursor),plain=variant;
                        plain.erase(std::remove(plain.begin(),plain.end(),'<'),plain.end());
                        if(plain=="#") plain.clear();
                        if(plain==suffix && variant!="**" && variant!="--") {
                            ++out.candidates;
                            const std::size_t slot=kind==7 ?
                                (f<26?1:f<52?2:f<78?3:f<104?4:f<108?5:f<112?6:8):f+1;
                            const auto row=23*(20*kind+type);
                            const bool plus=kind<8 && type<20 && slot<=l.types[row] && l.types[row+slot]=='+';
                            // Marked suffixes and unknown selectors remain audit-only.
                            if(!plus || variant.find('<')!=std::string::npos || !stress || stress>vowels(stem))
                                unresolved=true;
                            else {stresses.insert(stress-1);eligible|=supported;}
                        }
                        if(end==form.size()) break;cursor=end+1;
                    }
                }
            };
            if(m.size()==7) {
                const bool main=m[0]>=1 && m[0]<=88;
                const bool supported=main && m[3]%3!=1 && m[0]!=34 && m[0]!=35;
                scan(true,m[0],main?14:113,main?6:7,m[6],m[4],supported);
                scan(true,m[1],113,7,m[6],m[4],false);
            } else if(m.size()==6) {
                const auto cls=[](unsigned p){return p>=1 && p<=99?1:p>=100 && p<=202?2:8;};
                for(unsigned p:{unsigned(m[0]),unsigned(m[1])}) {
                    const auto kind=cls(p);
                    scan(false,p,kind==1?8:kind==2?6:113,kind,m[4],m[2],kind<3);
                }
            } else if(suffix.empty()) unresolved=true;
        }
    }
    if(unresolved && out.candidates) out.status="unsupported-candidate";
    else if(stresses.size()>1) out.status="ambiguous";
    else if(eligible && stresses.size()==1) {out.stress_vowel=*stresses.begin();out.status="accepted";}
    else if(out.candidates) out.status="unsupported-candidate";
    return out;
}
} // namespace nicolai
