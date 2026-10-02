#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/voice_db.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#if defined(_WIN32) && !defined(_WIN64)
#include "local_original_sha256.hpp"
namespace {
struct Buffer {
    std::vector<std::uint8_t> bytes;
    explicit Buffer(std::size_t size):bytes(size+64,0xa5) {
        std::fill(bytes.begin()+32,bytes.end()-32,0);
    }
    std::uint8_t* data() {return bytes.data()+32;}
    void check() const {
        for(std::size_t i=0;i<32;++i)
            if(bytes[i]!=0xa5 || bytes[bytes.size()-1-i]!=0xa5)
                throw std::runtime_error("original_crossed_buffer_guard");
    }
};
template<class T> void put(Buffer& b,std::size_t at,T value) {
    if(at> b.bytes.size()-64 || sizeof(value)>b.bytes.size()-64-at)
        throw std::runtime_error("synthetic_buffer_bounds");
    std::memcpy(b.data()+at,&value,sizeof(value));
}
void bind_crt(std::uint8_t* base,HMODULE crt,std::size_t rva,const char* name) {
    const auto function=GetProcAddress(crt,name);
    if(!function) throw std::runtime_error("missing_inspected_crt_import");
    DWORD previous=0,ignored=0;
    if(!VirtualProtect(base+rva,sizeof(function),PAGE_READWRITE,&previous))
        throw std::runtime_error("cannot_bind_inspected_crt_import");
    std::memcpy(base+rva,&function,sizeof(function));
    if(!VirtualProtect(base+rva,sizeof(function),previous,&ignored))
        throw std::runtime_error("cannot_restore_iat_protection");
}
void original_contract(const char* path,const nicolai::RussianYoPolicyM49& policy) {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    if(nicolai::local_original::sha256(path)!=
       "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7")
        throw std::runtime_error("unsupported_original_sha256");
    if(GetModuleHandleA("mtsyc32.dll")) throw std::runtime_error("original_already_loaded");
    // This maps only in this disposable x86 process: no DllMain, activation,
    // server, voice resource or original dictionary entry is executed.
    const auto module=LoadLibraryExA(path,nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) throw std::runtime_error("cannot_map_original");
    struct Guard {HMODULE m;~Guard(){FreeLibrary(m);}} guard{module};
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
    const std::uint8_t head[]={0x83,0xec,8,0x53,0x8b,0x5c,0x24,0x18,0x55,0x56};
    if(nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
       nt->OptionalHeader.ImageBase!=0x10000000 ||
       nt->OptionalHeader.SizeOfImage!=0x797000 ||
       nt->FileHeader.TimeDateStamp!=0x412a0cb4 ||
       std::memcmp(base+0x212890,head,sizeof(head)))
        throw std::runtime_error("unsupported_original_code_fingerprint");
    // The inspected '-' path reaches only these two system CRT imports.
    // The string conversion helper copies bytes with an internal loop.
    const auto crt=LoadLibraryExA("msvcrt.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!crt) throw std::runtime_error("cannot_load_system_crt");
    Guard crt_guard{crt};
    bind_crt(base,crt,0x23a12c,"memchr");bind_crt(base,crt,0x23a21c,"strchr");
    Buffer root(0x81258),parent(0x80),state(0x3c50),candidates(0x59c),
        positions(8),blocks(8),records(32),metadata(32),types(3680),
        stem(96),suffix(96),empty_list(4);
    put(parent,0x7c,root.data());put(state,0,parent.data());
    put(root,0x81254,types.data());put(state,0x124,candidates.data());
    put(state,0x164,positions.data());put(state,0x158,blocks.data());
    put(state,0x15c,records.data());put(state,0x160,metadata.data());
    put(state,0x144,stem.data());put(state,0x384c+4*47,empty_list.data());
    state.data()[0x154]=1;blocks.data()[1]=2;
    put<std::uint32_t>(records,4,11);metadata.data()[8+6]=7;
    using Author=void (__cdecl*)(void*,const char*,int,int,int,int,int,int,int);
    const auto author=reinterpret_cast<Author>(base+0x212890);
    Buffer yes_list(12);
    yes_list.data()[0]=8;
    for(unsigned f=1;f<=8;++f) yes_list.data()[f]=f;
    const std::string synthetic_stem{char(0xa1),char(0xa0),char(0xa0)};
    const std::string ending{'<',char(0xa5),char(0xa0)};
    std::copy(synthetic_stem.begin(),synthetic_stem.end(),stem.data());
    suffix.data()[0]=ending.size();
    std::copy(ending.begin(),ending.end(),suffix.data()+1);
    std::size_t full_cases=0,yo_cases=0,noun_refusals=0;
    for(unsigned kind:{1u,2u,3u,4u,5u,6u,7u}) {
        const unsigned forms=kind==1?8:kind==2?6:kind==3?26:kind==4?4:kind==5?1:kind==6?14:113;
        const unsigned first=kind<=2?1:kind==3?212:kind==4?232:kind==5?47:kind==6?1:89;
        const unsigned last=kind<=2?255:kind<=5?first:kind==6?88:255;
        for(unsigned paradigm=first;paradigm<=last;++paradigm)
            for(unsigned type=0;type<20;++type) for(unsigned form=1;form<=forms;++form)
                for(unsigned membership=0;membership<(kind<=2?2u:1u);++membership) {
                    const auto portable=nicolai::decode_russian_ending_choice_m49(
                        synthetic_stem,ending,policy,kind,paradigm,type,form,bool(membership));
                    if(!portable) throw std::runtime_error("refused_known_synthetic_selector");
                    if(kind<=2 && membership && !nicolai::select_russian_yo_m49(policy,kind,paradigm,type,form))
                        ++noun_refusals;
                    const auto f=form-1;
                    const unsigned slot=kind==3 || kind==5?1:kind==7?
                        (f<26?1:f<52?2:f<78?3:f<104?4:f<108?5:f<112?6:8):form;
                    std::fill(types.data(),types.data()+3680,0);
                    types.data()[23*(20*kind+type)]=22;
                    types.data()[23*(20*kind+type)+slot]='-';
                    if(kind<=2) put(state,0x384c+4*paradigm,
                        membership?yes_list.data():empty_list.data());
                    std::fill(candidates.data(),candidates.data()+0x59c,0);
                    author(state.data(),reinterpret_cast<const char*>(suffix.data()),kind,type,99,5,form,paradigm,1);
                    for(const auto* buf:{&root,&parent,&state,&candidates,&positions,&blocks,
                                       &records,&metadata,&types,&stem,&suffix,&empty_list,&yes_list}) buf->check();
                    std::vector<std::uint8_t> expected(24);
                    expected[4]=portable->yo_letter_index?*portable->yo_letter_index+1:0;
                    expected[5]=portable->stress_vowel+1;expected[6]=5;expected[7]=kind;
                    expected[8]=kind<=2?7:0;expected[9]=form;expected[16]=2;
                    const std::uint32_t record=11;std::memcpy(expected.data()+20,&record,4);
                    if(candidates.data()[0]!=1 || std::memcmp(expected.data(),candidates.data()+20,24)) {
                        std::cerr<<"FAILURE "<<kind<<' '<<paradigm<<' '<<type<<' '<<form<<' '<<membership
                            <<" actual="<<unsigned(candidates.data()[24])<<" expected="<<unsigned(expected[4])<<'\n';
                        throw std::runtime_error("original_yo_candidate_mismatch");
                    }
                    ++full_cases;yo_cases+=portable->yo_letter_index.has_value();
                }
    }
    std::cerr<<"ORIGINAL\t"<<full_cases<<'\t'<<yo_cases<<'\t'<<noun_refusals<<'\n';
}
}
#endif
int main(int argc,char** argv) try {
    if(argc!=4 && argc!=6) {std::cerr<<"usage: nicolai_m49_lexicon_probe voice.dat words.tsv private-policy.bin [--original local-mtsyc32.dll]\n";return 2;}
    const auto db=nicolai::VoiceDb::load(argv[1]);
    const auto lex=nicolai::parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
    const auto policy=nicolai::load_russian_yo_policy_m49(argv[3]);
    if(!lex.valid || !policy.valid) throw std::runtime_error(lex.valid?policy.error:lex.error);
    std::cerr<<"LEXICON\t"<<lex.blocks<<'\t'<<lex.records<<'\t'<<lex.stems.size()<<'\n';
    if(argc==6) {
        if(std::string(argv[4])!="--original") throw std::runtime_error("unknown_option");
#if defined(_WIN32) && !defined(_WIN64)
        original_contract(argv[5],policy);
#else
        throw std::runtime_error("original_probe_requires_win32_x86");
#endif
    }
    std::ifstream input(argv[2]);if(!input) throw std::runtime_error("cannot_read_words");
    std::string line;
    while(std::getline(input,line)) {
        if(!line.empty() && line.back()=='\r') line.pop_back();
        const auto tab=line.find('\t');if(tab==std::string::npos) continue;
        std::cout<<line.substr(0,tab);
        for(bool newer:{false,true}) {
            const auto result=newer?nicolai::lookup_russian_lexicon_stress_m49(lex,policy,line.substr(tab+1)):
                nicolai::lookup_russian_lexicon_stress_m48(lex,line.substr(tab+1));
            std::cout<<'\t'<<result.status<<'\t';
            if(result.stress_vowel) std::cout<<*result.stress_vowel;else std::cout<<'-';
            std::cout<<'\t'<<result.candidates;
            if(newer) {std::cout<<'\t';if(result.yo_letter_index) std::cout<<*result.yo_letter_index;else std::cout<<'-';}
        }
        std::cout<<'\n';
    }
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
