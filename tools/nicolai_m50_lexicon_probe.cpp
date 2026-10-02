#include "nicolai/russian_lexicon_m44.hpp"
#include "nicolai/voice_db.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#if defined(_WIN32) && !defined(_WIN64) && defined(_MSC_VER)
#include "local_original_sha256.hpp"
namespace {
struct Buffer {
    std::vector<std::uint8_t> bytes;
    explicit Buffer(std::size_t size):bytes(size+64,0xa5) {
        std::fill(bytes.begin()+32,bytes.end()-32,0);
    }
    std::uint8_t* data(){return bytes.data()+32;}
    void check() const {
        for(std::size_t i=0;i<32;++i)
            if(bytes[i]!=0xa5 || bytes[bytes.size()-1-i]!=0xa5)
                throw std::runtime_error("original_crossed_buffer_guard");
    }
};
template<class T> void put(Buffer& b,std::size_t at,T value) {
    if(at>b.bytes.size()-64 || sizeof(value)>b.bytes.size()-64-at)
        throw std::runtime_error("synthetic_buffer_bounds");
    std::memcpy(b.data()+at,&value,sizeof(value));
}
void bind_crt(std::uint8_t* base,HMODULE crt,std::size_t rva,const char* name) {
    const auto function=GetProcAddress(crt,name);DWORD previous=0,ignored=0;
    if(!function || !VirtualProtect(base+rva,sizeof(function),PAGE_READWRITE,&previous))
        throw std::runtime_error("cannot_bind_inspected_crt");
    std::memcpy(base+rva,&function,sizeof(function));
    if(!VirtualProtect(base+rva,sizeof(function),previous,&ignored))
        throw std::runtime_error("cannot_restore_iat_protection");
}
__declspec(naked) void invoke_initializer(void* entry,void* state) {
    __asm {
        push ebp
        push ebx
        push esi
        push edi
        mov ebp, dword ptr [esp+24]
        mov eax, dword ptr [esp+20]
        call eax
        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
    }
}
void original_contract(const char* path,const nicolai::RussianYoPolicyM49& yo,
    const nicolai::RussianNounYoPolicyM50& noun) {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    if(nicolai::local_original::sha256(path)!=
        "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7")
        throw std::runtime_error("unsupported_original_sha256");
    if(GetModuleHandleA("mtsyc32.dll")) throw std::runtime_error("original_already_loaded");
    const auto module=LoadLibraryExA(path,nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) throw std::runtime_error("cannot_map_original");
    struct Guard{HMODULE m;~Guard(){FreeLibrary(m);}} guard{module};
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
    const std::uint8_t head[]={0x83,0xec,8,0x53,0x8b,0x5c,0x24,0x18,0x55,0x56};
    if(nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
       nt->OptionalHeader.ImageBase!=0x10000000 || nt->OptionalHeader.SizeOfImage!=0x797000 ||
       nt->FileHeader.TimeDateStamp!=0x412a0cb4 ||
       std::memcmp(base+0x212890,head,sizeof(head)))
        throw std::runtime_error("unsupported_original_code_fingerprint");
    const auto crt=LoadLibraryExA("msvcrt.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!crt) throw std::runtime_error("cannot_load_system_crt");
    Guard crt_guard{crt};
    bind_crt(base,crt,0x23a12c,"memchr");bind_crt(base,crt,0x23a21c,"strchr");
    Buffer root(0x81258),parent(0x80),state(0x3d40),candidates(0x59c),positions(8),
        blocks(8),records(32),metadata(32),types(3680),stem(96),suffix(96);
    put(parent,0x7c,root.data());put(state,0,parent.data());put(root,0x81254,types.data());
    put(state,0x124,candidates.data());put(state,0x164,positions.data());
    put(state,0x158,blocks.data());put(state,0x15c,records.data());put(state,0x160,metadata.data());
    put(state,0x144,stem.data());state.data()[0x154]=1;blocks.data()[1]=2;
    put<std::uint32_t>(records,4,11);metadata.data()[14]=7;
    std::vector<Buffer> lists;lists.reserve(200);
    for(unsigned paradigm=3;paradigm<=202;++paradigm) {
        lists.emplace_back(20);put(state,0x384c+4*paradigm,lists.back().data());
    }
    // Run ONLY the import-free list-writing lane against owned buffers. A
    // return epilogue replaces its fall-through into unrelated initialization
    // in THIS disposable mapping. Never patch any installed module/file.
    DWORD old=0,ignored=0;
    if(!VirtualProtect(base+0x102770,4,PAGE_EXECUTE_READWRITE,&old))
        throw std::runtime_error("cannot_bound_owned_initializer");
    const std::uint8_t epilogue[]={0x83,0xc4,0x38,0xc3}; // release remaining 56 argument bytes
    std::uint8_t saved[4];std::memcpy(saved,base+0x102770,4);
    std::memcpy(base+0x102770,epilogue,4);
    if(!FlushInstructionCache(GetCurrentProcess(),base+0x102770,4))
        throw std::runtime_error("cannot_flush_owned_initializer");
    invoke_initializer(base+0x1025de,state.data());
    std::memcpy(base+0x102770,saved,4);
    if(!VirtualProtect(base+0x102770,4,old,&ignored))
        throw std::runtime_error("cannot_restore_owned_initializer");
    for(unsigned p=3;p<=202;++p) {
        lists[p-3].check();
        std::vector<std::uint8_t> expected(20);
        const auto& values=noun.forms[p-3];expected[0]=values.size();
        std::copy(values.begin(),values.end(),expected.begin()+1);
        if(std::memcmp(expected.data(),lists[p-3].data(),20))
            throw std::runtime_error("original_noun_initialization_mismatch");
    }
    for(const auto* b:{&root,&parent,&state,&candidates,&positions,&blocks,&records,&metadata,
        &types,&stem,&suffix}) b->check();
    using Author=void(__cdecl*)(void*,const char*,int,int,int,int,int,int,int);
    const auto author=reinterpret_cast<Author>(base+0x212890);
    const std::string s{char(0xa1),char(0xa0),char(0xa0)};
    const std::string e(1,char(0xa5)),a(1,char(0xa0));
    std::copy(s.begin(),s.end(),stem.data());std::size_t full=0,positive=0;
    for(unsigned p=3;p<=202;++p) {
        const unsigned kind=p<=99?1:2,forms=kind==1?8:6;
        for(unsigned type=0;type<20;++type) for(unsigned f=1;f<=forms;++f)
            for(const auto& ending:{"<"+e,e+"<"+e,e+a,"<"+a}) {
                std::fill(types.data(),types.data()+3680,0);
                types.data()[23*(20*kind+type)]=forms;types.data()[23*(20*kind+type)+f]='-';
                std::fill(candidates.data(),candidates.data()+0x59c,0);
                suffix.data()[0]=ending.size();std::copy(ending.begin(),ending.end(),suffix.data()+1);
                const auto portable=nicolai::decode_russian_ending_choice_m49(s,ending,yo,kind,p,type,f,
                    nicolai::noun_yo_form_member_m50(noun,kind,p,f));
                if(!portable) throw std::runtime_error("known_noun_filter_refused");
                author(state.data(),reinterpret_cast<const char*>(suffix.data()),kind,type,99,5,f,p,1);
                for(const auto* b:{&root,&parent,&state,&candidates,&positions,&blocks,&records,&metadata,
                    &types,&stem,&suffix}) b->check();
                lists[p-3].check();std::vector<std::uint8_t> expected(24);
                expected[4]=portable->yo_letter_index?*portable->yo_letter_index+1:0;
                expected[5]=portable->stress_vowel+1;expected[6]=5;expected[7]=kind;
                expected[8]=7;expected[9]=f;expected[16]=2;
                const std::uint32_t record=11;std::memcpy(expected.data()+20,&record,4);
                if(candidates.data()[0]!=1 || std::memcmp(expected.data(),candidates.data()+20,24))
                    throw std::runtime_error("original_noun_candidate_mismatch");
                ++full;positive+=portable->yo_letter_index.has_value();
            }
    }
    for(const auto& list:lists) list.check();
    std::cerr<<"ORIGINAL\t200\t"<<full<<'\t'<<positive<<'\n';
}
}
#endif
int main(int argc,char** argv) try {
    if(argc!=5 && argc!=7) {std::cerr<<"usage: probe voice.dat words.tsv yo49.bin noun50.bin [--original local-DLL]\n";return 2;}
    const auto db=nicolai::VoiceDb::load(argv[1]);
    const auto lex=nicolai::parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
    const auto yo=nicolai::load_russian_yo_policy_m49(argv[3]);
    const auto noun=nicolai::load_russian_noun_yo_policy_m50(argv[4]);
    if(!lex.valid || !yo.valid || !noun.valid)
        throw std::runtime_error(!lex.valid?lex.error:!yo.valid?yo.error:noun.error);
    std::cerr<<"LEXICON\t"<<lex.blocks<<'\t'<<lex.records<<'\t'<<lex.stems.size()<<'\n';
    if(argc==7) {
        if(std::string(argv[5])!="--original") throw std::runtime_error("unknown_option");
#if defined(_WIN32) && !defined(_WIN64) && defined(_MSC_VER)
        original_contract(argv[6],yo,noun);
#else
        throw std::runtime_error("original_probe_requires_win32_x86_msvc");
#endif
    }
    std::ifstream input(argv[2]);if(!input) throw std::runtime_error("cannot_read_words");
    std::string line;
    while(std::getline(input,line)) {
        if(!line.empty() && line.back()=='\r') line.pop_back();
        const auto tab=line.find('\t');if(tab==std::string::npos) continue;
        std::cout<<line.substr(0,tab);
        for(bool newer:{false,true}) {
            const auto row=newer?nicolai::lookup_russian_lexicon_stress_m50(lex,yo,noun,line.substr(tab+1)):
                nicolai::lookup_russian_lexicon_stress_m49(lex,yo,line.substr(tab+1));
            std::cout<<'\t'<<row.status<<'\t';
            if(row.stress_vowel) std::cout<<*row.stress_vowel;else std::cout<<'-';
            std::cout<<'\t'<<row.candidates<<'\t';
            if(row.yo_letter_index) std::cout<<*row.yo_letter_index;else std::cout<<'-';
        }
        std::cout<<'\n';
    }
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
