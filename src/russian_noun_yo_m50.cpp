#include "nicolai/russian_noun_yo_m50.hpp"
#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>
namespace nicolai {
RussianNounYoPolicyM50 parse_russian_noun_yo_policy_m50(const std::vector<std::uint8_t>& b) {
    RussianNounYoPolicyM50 out;
    try {
        const std::string magic("N50NOUN\0",8);
        if(b.size()!=1848 || !std::equal(magic.begin(),magic.end(),b.begin()))
            throw std::runtime_error("noun_yo_policy_header");
        constexpr char sha[]="f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7";
        const auto digit=[](char c){return c<='9'?c-'0':c-'a'+10;};
        for(std::size_t i=0;i<32;++i)
            if(b[8+i]!=(digit(sha[2*i])*16+digit(sha[2*i+1])))
                throw std::runtime_error("noun_yo_policy_unsupported_source");
        if(b[40]!=8 || b[41]!=7 || b[42] || b[43])
            throw std::runtime_error("noun_yo_policy_payload_size");
        std::uint32_t crc=0xffffffffu;
        for(std::size_t i=48;i<b.size();++i) {
            crc^=b[i];
            for(unsigned bit=0;bit<8;++bit) crc=(crc>>1)^(0xedb88320u&(0u-(crc&1u)));
        }
        const auto checksum=b[44]|(std::uint32_t(b[45])<<8)|
            (std::uint32_t(b[46])<<16)|(std::uint32_t(b[47])<<24);
        if(checksum!=~crc) throw std::runtime_error("noun_yo_policy_checksum");
        for(std::size_t row=0;row<200;++row) {
            const auto start=48+9*row;
            const unsigned maximum=row+3<=99?8:6;
            const auto n=b[start];
            if(n>maximum) throw std::runtime_error("noun_yo_policy_list_length");
            std::set<std::uint8_t> seen;
            for(unsigned i=1;i<=n;++i) {
                const auto value=b[start+i];
                if(!value || value>maximum || !seen.insert(value).second)
                    throw std::runtime_error("noun_yo_policy_form");
                out.forms[row].push_back(value);
            }
            for(unsigned i=n+1;i<9;++i)
                if(b[start+i]) throw std::runtime_error("noun_yo_policy_padding");
        }
        out.valid=true;
    } catch(const std::exception& e) {out={};out.error=e.what();}
    return out;
}
RussianNounYoPolicyM50 load_russian_noun_yo_policy_m50(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary);
    if(!input) {RussianNounYoPolicyM50 out;out.error="missing_local_noun_yo_policy: run tools/export_noun_yo_policy_m50.py";return out;}
    std::vector<std::uint8_t> bytes(1849);
    input.read(reinterpret_cast<char*>(bytes.data()),bytes.size());bytes.resize(input.gcount());
    return parse_russian_noun_yo_policy_m50(bytes);
}
std::optional<bool> noun_yo_form_member_m50(const RussianNounYoPolicyM50& p,
    unsigned kind,unsigned paradigm,unsigned form) {
    if(!p.valid || !form || !((kind==1 && paradigm>=3 && paradigm<=99 && form<=8) ||
        (kind==2 && paradigm>=100 && paradigm<=202 && form<=6))) return std::nullopt;
    const auto& values=p.forms[paradigm-3];
    return std::find(values.begin(),values.end(),form)!=values.end();
}
}
