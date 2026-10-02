#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/voice_db.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstring>
#if defined(_WIN32) && !defined(_WIN64)
#include "local_original_sha256.hpp"
namespace {
template<class T> void put(std::vector<std::uint8_t>& b,std::size_t at,T value) {
    std::memcpy(b.data()+at,&value,sizeof(value));
}
void original_contract(const char* path) {
    // Only inspected import-free paths, synthetic records and '+' rows.
    // No DllMain, full NLP, '-' authoring, or special verb paradigms 34/35.
    if(nicolai::local_original::sha256(path)!=
       "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7")
        throw std::runtime_error("unsupported_original_sha256");
    auto module=LoadLibraryExA(path,nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) throw std::runtime_error("cannot_map_original");
    struct Guard {HMODULE m;~Guard(){FreeLibrary(m);}} guard{module};
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    std::vector<std::uint8_t> root(0x81258),parent(0x80),state(0x3858),
        candidates(0x59c),positions(8),blocks(8),records(32),metadata(16),suffixes(4),types(3680);
    put(parent,0x7c,root.data());put(state,0,parent.data());
    put(state,0x124,candidates.data());put(state,0x164,positions.data());
    put(state,0x158,blocks.data());put(state,0x15c,records.data());
    put(state,0x160,metadata.data());put(state,0x14c,suffixes.data());
    state[0x154]=1;blocks[1]=2;put<std::uint32_t>(records,4,11);
    using ExactFn=void (__cdecl*)(void*,int);
    auto exact=reinterpret_cast<ExactFn>(base+0x211b40);
    std::size_t exact_cases=0,reserved_cases=0;
    for(unsigned selector=0;selector<256;++selector) for(unsigned stress=0;stress<5;++stress)
        for(unsigned position:{0u,3u}) {
            std::vector<std::uint8_t> m{static_cast<std::uint8_t>(selector),
                static_cast<std::uint8_t>(stress),7,0};
            const auto portable=nicolai::decode_russian_exact_candidate_m47(m);
            if(!portable && selector<254) continue;
            std::fill(metadata.begin(),metadata.end(),0);metadata[8]=4;
            std::copy(m.begin(),m.end(),metadata.begin()+9);positions[1]=position;
            std::fill(candidates.begin(),candidates.end(),0);
            exact(state.data(),0);
            if(!portable) {
                if(candidates[0]) throw std::runtime_error("reserved_exact_emitted");
                ++reserved_cases;continue;
            }
            // First-lane authoring starts at block+24. Later NLP transforms
            // this into the capture's score-prefixed 20-byte candidate view.
            std::vector<std::uint8_t> expected(24);
            expected[4]=position;expected[5]=portable->stress;expected[6]=portable->auxiliary;
            expected[7]=portable->kind;expected[8]=portable->tag;expected[9]=portable->form;
            expected[16]=2;put<std::uint32_t>(expected,20,11);
            if(candidates[0]!=1 || std::memcmp(expected.data(),candidates.data()+20,24)) {
                std::cerr<<"EXACT_FAILURE\t"<<selector<<'\t'<<stress<<'\t'<<position;
                for(unsigned i=0;i<24;++i) if(expected[i]!=candidates[20+i])
                    std::cerr<<"\t"<<i<<':'<<unsigned(expected[i])<<':'<<unsigned(candidates[20+i]);
                std::cerr<<'\n';
                throw std::runtime_error("exact_candidate_mismatch");
            }
            ++exact_cases;
        }
    put(root,0x81254,types.data());
    using AuthorFn=void (__cdecl*)(void*,const char*,int,int,int,int,int,int,int);
    auto author=reinterpret_cast<AuthorFn>(base+0x212890);
    std::size_t plus_cases=0;
    for(unsigned kind:{1u,2u,3u,4u,6u}) for(unsigned stress=1;stress<=4;++stress)
        for(unsigned form=1;form<=(kind==3?26:kind==6?14:kind==1?8:kind==2?6:4);++form)
            for(bool marked:{false,true}) {
                const unsigned slot=kind==3?1:form;
                types[23*(20*kind+2)+slot]='+';
                const char plain[]={1,'x',0},with_marker[]={2,'<','x',0};
                std::fill(candidates.begin(),candidates.end(),0);positions[1]=0;
                author(state.data(),marked?with_marker:plain,kind,2,stress,0,form,47,1);
                if(candidates[0]!=1 || candidates[24]!=0 || candidates[25]!=stress ||
                   candidates[27]!=kind || candidates[29]!=form)
                    throw std::runtime_error("plus_candidate_mismatch");
                ++plus_cases;
            }
    std::cerr<<"ORIGINAL\t"<<exact_cases<<'\t'<<reserved_cases<<'\t'<<plus_cases<<'\n';
}
}
#endif
int main(int argc,char** argv) try {
    if(argc!=3 && argc!=5) {std::cerr<<"usage: nicolai_m47_lexicon_probe voice.dat words.tsv [--original local-mtsyc32.dll]\n";return 2;}
    const auto db=nicolai::VoiceDb::load(argv[1]);
    const auto lex=nicolai::parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
    if(!lex.valid) throw std::runtime_error(lex.error);
    std::cerr<<"LEXICON\t"<<lex.blocks<<'\t'<<lex.records<<'\t'<<lex.stems.size()<<'\n';
    if(argc==5) {
        if(std::string(argv[3])!="--original") throw std::runtime_error("unknown_option");
#if defined(_WIN32) && !defined(_WIN64)
        original_contract(argv[4]);
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
            const auto result=newer?nicolai::lookup_russian_lexicon_stress_m47(lex,line.substr(tab+1)):
                nicolai::lookup_russian_lexicon_stress_m44(lex,line.substr(tab+1));
            std::cout<<'\t'<<result.status<<'\t';
            if(result.stress_vowel) std::cout<<*result.stress_vowel;else std::cout<<'-';
            std::cout<<'\t'<<result.candidates;
        }
        std::cout<<'\n';
    }
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
