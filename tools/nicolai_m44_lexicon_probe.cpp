#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/voice_db.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#if defined(_WIN32) && !defined(_WIN64)
#include <windows.h>
namespace {
template<class T> void put(std::vector<std::uint8_t>& b,std::size_t at,T value) {
    std::memcpy(b.data()+at,&value,sizeof(value));
}
void original_contract(const char* path,const nicolai::RussianLexiconM44& lex) {
    // The two inspected '+' primitives below make no import calls. Map only:
    // no DLL entry point, server startup or dependency initialization. Never
    // use this image to run full NLP or any other path with unresolved imports.
    auto module=LoadLibraryExA(path,nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) throw std::runtime_error("cannot_load_local_original");
    struct Guard {HMODULE module;~Guard(){FreeLibrary(module);}} guard{module};
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
    const unsigned char type_head[]={0x8b,0x44,0x24,0x08,0x8b,0x4c,0x24,0x0c};
    const unsigned char author_head[]={0x83,0xec,0x08,0x53,0x8b,0x5c,0x24,0x18};
    if(nt->OptionalHeader.ImageBase!=0x10000000 ||
       nt->OptionalHeader.SizeOfImage!=0x797000 ||
       nt->FileHeader.TimeDateStamp!=0x412a0cb4 ||
       std::memcmp(base+0x1039b0,type_head,sizeof(type_head)) ||
       std::memcmp(base+0x212890,author_head,sizeof(author_head)))
        throw std::runtime_error("unsupported_original_code_fingerprint");
    std::vector<std::uint8_t> root(0x81258,0),parent(0x80,0),state(0x3858,0),
        candidates(2048,0),positions(8,0),blocks(8,0),records(32,0);
    put(root,0x81254,lex.types.data());put(parent,0x7c,root.data());put(state,0,parent.data());
    put(state,0x124,candidates.data());put<std::uint32_t>(state,0x140,0);
    put(state,0x164,positions.data());put(state,0x158,blocks.data());put(state,0x15c,records.data());
    using TypeFn=const std::uint8_t* (__cdecl*)(void*,int,int,int);
    auto type=reinterpret_cast<TypeFn>(base+0x1039b0);
    std::size_t cells=0;
    for(int kind=0;kind<8;++kind) for(int row=0;row<20;++row) for(int slot=0;slot<23;++slot) {
        const auto* expected=lex.types.data()+23*(20*kind+row)+slot;
        if(type(root.data(),kind,row,slot)!=expected) throw std::runtime_error("original_type_offset_mismatch");
        ++cells;
    }
    using AuthorFn=void (__cdecl*)(void*,const char*,int,int,int,int,int,int,int);
    auto author=reinterpret_cast<AuthorFn>(base+0x212890);
    const char suffix[]={1,'x',0};
    std::size_t copied=0;
    for(int stress=1;stress<=5;++stress) for(int form=1;form<=14;++form) {
        std::fill(candidates.begin(),candidates.end(),0);
        // Synthetic main-verb class, '+' row, no explicit ё marker. This tests
        // the intermediate candidate fields, not complete original text NLP.
        author(state.data(),suffix,6,2,stress,0,form,47,1);
        if(candidates[0]!=1 || candidates[24]!=0 || candidates[25]!=stress || candidates[27]!=6 || candidates[29]!=form)
            throw std::runtime_error("original_stem_candidate_mismatch");
        ++copied;
    }
    std::cerr<<"ORIGINAL\t"<<cells<<"\t"<<copied<<'\n';
}
}
#endif

int main(int argc,char** argv) try {
    if(argc!=3 && argc!=5) {std::cerr<<"usage: nicolai_m44_lexicon_probe nicolai16.dat words.tsv [--original local-mtsyc32.dll]\n";return 2;}
    const auto db=nicolai::VoiceDb::load(argv[1]);
    const auto lex=nicolai::parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
    if(!lex.valid) throw std::runtime_error(lex.error);
    if(argc==5) {
        if(std::string(argv[3])!="--original") throw std::runtime_error("unknown_probe_option");
#if defined(_WIN32) && !defined(_WIN64)
        original_contract(argv[4],lex);
#else
        throw std::runtime_error("original_probe_requires_win32_x86");
#endif
    }
    std::cerr<<"LEXICON\t"<<lex.blocks<<"\t"<<lex.records<<"\t"<<lex.stems.size()<<'\n';
    std::ifstream file(argv[2]);if(!file) throw std::runtime_error("cannot_open_words");
    std::string line;
    while(std::getline(file,line)) {
        if(!line.empty() && line.back()=='\r') line.pop_back();
        const auto tab=line.find('\t');if(tab==std::string::npos) continue;
        const auto query=nicolai::lookup_russian_lexicon_stress_m44(lex,line.substr(tab+1));
        std::cout<<line.substr(0,tab)<<'\t'<<query.status<<'\t';
        if(query.stress_vowel) std::cout<<*query.stress_vowel;else std::cout<<'-';
        std::cout<<'\t'<<query.candidates<<'\n';
    }
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
