#include "nicolai/russian_yo_m49.hpp"
#include "nicolai/russian_lexicon_m44.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <set>
#include <stdexcept>
namespace nicolai {
namespace {
std::uint32_t crc32(const std::uint8_t* data,std::size_t size) {
    std::uint32_t crc=0xffffffffu;
    for(std::size_t i=0;i<size;++i) {
        crc^=data[i];
        for(unsigned bit=0;bit<8;++bit) crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));
    }
    return ~crc;
}
bool member(const std::vector<std::uint8_t>& values,unsigned value) {
    return std::find(values.begin(),values.end(),value)!=values.end();
}
}
RussianYoPolicyM49 parse_russian_yo_policy_m49(const std::vector<std::uint8_t>& b) {
    RussianYoPolicyM49 out;
    try {
        const std::string magic("N49YOv1\0",8);
        if(b.size()!=536 || !std::equal(magic.begin(),magic.end(),b.begin()))
            throw std::runtime_error("yo_policy_header");
        constexpr char sha[]="f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7";
        const auto digit=[](char c){return c<='9'?c-'0':c-'a'+10;};
        for(std::size_t i=0;i<32;++i)
            if(b[8+i]!=(digit(sha[2*i])*16+digit(sha[2*i+1])))
                throw std::runtime_error("yo_policy_unsupported_source");
        if(b[40]!=0xe8 || b[41]!=1 || b[42] || b[43])
            throw std::runtime_error("yo_policy_payload_size");
        const std::uint32_t checksum=b[44]|(std::uint32_t(b[45])<<8)|
            (std::uint32_t(b[46])<<16)|(std::uint32_t(b[47])<<24);
        if(checksum!=crc32(b.data()+48,488)) throw std::runtime_error("yo_policy_checksum");
        for(std::size_t kind=0;kind<8;++kind) for(unsigned lane=0;lane<2;++lane) {
            const auto start=48+61*kind+30*lane;
            const auto n=b[start];if(n>29) throw std::runtime_error("yo_policy_list_length");
            std::set<std::uint8_t> seen;
            auto& values=lane?out.types[kind]:out.paradigms[kind];
            for(std::size_t i=1;i<=n;++i) {
                const auto value=b[start+i];
                if((lane && value>=20) || (!lane && !value) || !seen.insert(value).second)
                    throw std::runtime_error("yo_policy_selector");
                values.push_back(value);
            }
            for(std::size_t i=n+1;i<30;++i)
                if(b[start+i]) throw std::runtime_error("yo_policy_padding");
        }
        out.valid=true;
    } catch(const std::exception& e) {out={};out.error=e.what();}
    return out;
}
RussianYoPolicyM49 load_russian_yo_policy_m49(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary);
    if(!input) {RussianYoPolicyM49 out;out.error="missing_local_yo_policy: run tools/export_yo_policy_m49.py";return out;}
    // Read a fixed maximum; a huge untrusted local file cannot cause allocation.
    std::vector<std::uint8_t> bytes(537);
    input.read(reinterpret_cast<char*>(bytes.data()),bytes.size());bytes.resize(input.gcount());
    return parse_russian_yo_policy_m49(bytes);
}
std::optional<bool> select_russian_yo_m49(const RussianYoPolicyM49& p,unsigned kind,
    unsigned paradigm,unsigned type,unsigned form,std::optional<bool> noun_member) {
    if(!p.valid || kind<1 || kind>7 || !paradigm || paradigm>255 || type>=20 || !form)
        return std::nullopt;
    const unsigned forms=kind==1?8:kind==2?6:kind==3?26:kind==4?4:kind==5?1:kind==6?14:113;
    if(form>forms) return std::nullopt;
    if(!member(p.paradigms[kind],paradigm) || !member(p.types[kind],type)) return false;
    if(kind==7 && !((form>=79 && form<=104)||(form>=109 && form<=112))) return false;
    if(kind==6 && ((paradigm==27 && (form==12 || form==13)) ||
       ((paradigm==30 || paradigm==32 || paradigm==33) && (form==1 || form==8 || form==11))))
        return false;
    if(kind==1 || kind==2) {
        if(!noun_member) return std::nullopt;
        if(!*noun_member) return false;
    }
    return true;
}
std::optional<RussianEndingChoiceM49> decode_russian_ending_choice_m49(
    const std::string& stem,const std::string& suffix,const RussianYoPolicyM49& p,
    unsigned kind,unsigned paradigm,unsigned type,unsigned form,std::optional<bool> noun_member) {
    const auto ending=decode_russian_ending_stress_m48(stem,suffix);
    if(!ending) return std::nullopt;
    RussianEndingChoiceM49 result{ending->stress_vowel,std::nullopt};
    if(!ending->needs_yo_selection) return result;
    const auto selected=select_russian_yo_m49(p,kind,paradigm,type,form,noun_member);
    if(!selected) return std::nullopt;
    if(*selected) {
        const auto marker=suffix.find('<');
        const auto end=marker==std::string::npos?suffix.size():marker+2;
        const auto position=suffix.rfind(char(0xa5),end-1);
        if(position==std::string::npos) return std::nullopt;
        result.yo_letter_index=stem.size()+position-(marker!=std::string::npos && marker<position?1:0);
    }
    return result;
}
}
