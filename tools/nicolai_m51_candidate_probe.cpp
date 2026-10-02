#include "nicolai/russian_candidate_selection_m51.hpp"
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
    std::uint8_t* data() { return bytes.data()+32; }
};
using Payload=nicolai::RussianCandidatePayloadM51;
std::size_t run(const char* dll,const char* input) {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    if(nicolai::local_original::sha256(dll)!=
        "f6b7e926c46a0259a866260cafb9d24d6ebed3dd7198829d16179348a186abc7")
        throw std::runtime_error("unsupported_original_sha256");
    if(GetModuleHandleA("mtsyc32.dll")) throw std::runtime_error("original_already_loaded");
    const auto module=LoadLibraryExA(dll,nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) throw std::runtime_error("cannot_map_original");
    struct Guard { HMODULE m; ~Guard() { FreeLibrary(m); } } guard{module};
    auto* base=reinterpret_cast<std::uint8_t*>(module);
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
    const std::uint8_t head[]={0x8b,0x44,0x24,0x08,0x8b,0x54,0x24,0x04,0x53,0x56};
    if(nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
       nt->OptionalHeader.ImageBase!=0x10000000 || nt->OptionalHeader.SizeOfImage!=0x797000 ||
       nt->FileHeader.TimeDateStamp!=0x412a0cb4 || std::memcmp(base+0x217830,head,sizeof(head)))
        throw std::runtime_error("unsupported_original_code_fingerprint");
    using Function=unsigned char(__cdecl*)(void*,int);
    const auto original=reinterpret_cast<Function>(base+0x217830);
    // The normalizer reads two pinned globals by absolute address. Refuse a
    // relocated image rather than calling any partly resolved original code.
    const std::uint8_t normalize_head[]={0x83,0xec,0x08,0x55,0x8b,0x6c,0x24,0x10};
    if(reinterpret_cast<std::uintptr_t>(base)!=0x10000000 ||
       std::memcmp(base+0x1a1830,normalize_head,sizeof(normalize_head)))
        throw std::runtime_error("unsupported_normalizer_mapping");
    using Normalizer=void(__cdecl*)(void*);
    const auto normalizer=reinterpret_cast<Normalizer>(base+0x1a1830);
    std::size_t checked=0;
    std::size_t normalized=0,synthetic_normalized=0,real_normalization_words=0,real_normalization_payloads=0;
    auto normalize=[&](const std::vector<Payload>& rows) {
        if(rows.size()>70) throw std::runtime_error("invalid_probe_domain");
        Buffer state(0x138),morph(2*0x59c),prefix(1);
        auto* ptr=morph.data();std::memcpy(state.data()+0x124,&ptr,sizeof(ptr));
        ptr=prefix.data();std::memcpy(state.data()+0x11c,&ptr,sizeof(ptr));
        const std::uint32_t word_count=1;std::memcpy(state.data()+0x134,&word_count,4);
        auto* block=morph.data()+0x59c;block[0]=static_cast<unsigned char>(rows.size());
        // Nonempty, no spaces/quotes, and not the original special punctuation:
        // the inspected full function cannot enter deletion/CRT-import lanes.
        block[0x590]=base[0x730314]==';'?'!':';';
        for(std::size_t i=0;i<rows.size();++i)
            std::memcpy(block+20*(i+1)+4,rows[i].data(),20);
        auto expected_morph=morph.bytes;
        auto portable=rows;nicolai::normalize_russian_candidate_forms_m51(portable);
        for(std::size_t i=0;i<rows.size();++i)
            std::memcpy(expected_morph.data()+32+0x59c+20*(i+1)+4,portable[i].data(),20);
        const auto saved_state=state.bytes,saved_prefix=prefix.bytes;
        normalizer(state.data());
        if(morph.bytes!=expected_morph || state.bytes!=saved_state || prefix.bytes!=saved_prefix)
            throw std::runtime_error("original_candidate_normalization_mismatch");
        normalized+=rows.size();
        return portable;
    };
    auto check=[&](const std::vector<Payload>& rows,int index) {
        if(rows.size()>70 || index<0 || index>256) throw std::runtime_error("invalid_probe_domain");
        Buffer state(0x138),morph((index+1)*0x59c);
        auto* ptr=morph.data();std::memcpy(state.data()+0x124,&ptr,sizeof(ptr));
        auto* block=ptr+index*0x59c;block[0]=static_cast<unsigned char>(rows.size());
        for(std::size_t i=0;i<rows.size();++i)
            std::memcpy(block+20*(i+1)+4,rows[i].data(),20);
        // Full owned-buffer identity also verifies the function is read-only.
        const auto saved_state=state.bytes,saved_morph=morph.bytes;
        const bool result=original(state.data(),index)!=0;
        if(result!=nicolai::russian_candidate_pronunciation_conflict_m51(rows))
            throw std::runtime_error("original_pronunciation_conflict_mismatch");
        if(state.bytes!=saved_state || morph.bytes!=saved_morph)
            throw std::runtime_error("original_changed_owned_buffers");
        ++checked;
    };
    for(std::size_t count=0;count<=70;++count) {
        std::vector<Payload> rows(count);
        for(std::size_t i=0;i<count;++i) {
            rows[i][0]=4;rows[i][1]=2;rows[i][2]=9;
            for(std::size_t f=3;f<20;++f) rows[i][f]=(i*37+f)&255;
        }
        for(int index:{0,1,256}) {
            check(rows,index);
            if(count<2) continue;
            for(std::size_t f=0;f<3;++f)
                for(std::size_t i:{std::size_t{0},count/2,count-1}) {
                    auto changed=rows;changed[i][f]^=128;check(changed,index);
                }
        }
    }
    // Complete byte-domain authoring-form sweep, batched within block capacity.
    std::vector<Payload> batch;
    for(unsigned kind=0;kind<256;++kind) for(unsigned form=0;form<256;++form) {
        Payload row{};
        for(unsigned i=0;i<20;++i) row[i]=(i*13+kind*7+form)&255;
        row[3]=kind;row[5]=form;batch.push_back(row);
        if(batch.size()==70) { normalize(batch);batch.clear(); }
    }
    normalize(batch);synthetic_normalized=normalized;
    std::size_t captured=0;
    if(input) {
        std::ifstream stream(input,std::ios::binary);
        char magic[8];std::uint32_t count=0;
        if(!stream.read(magic,8) || (std::memcmp(magic,"N51CAND1",8) && std::memcmp(magic,"N51CAND2",8)) ||
           !stream.read(reinterpret_cast<char*>(&count),4) || count>100000)
            throw std::runtime_error("invalid_private_candidate_input");
        auto read_rows=[&]() {
            unsigned char size=0;
            if(!stream.read(reinterpret_cast<char*>(&size),1) || size>70)
                throw std::runtime_error("invalid_private_candidate_count");
            std::vector<Payload> rows(size);
            for(auto& row:rows)
                if(!stream.read(reinterpret_cast<char*>(row.data()),20))
                    throw std::runtime_error("truncated_private_candidate_payload");
            return rows;
        };
        for(std::uint32_t i=0;i<count;++i) {
            const auto rows=read_rows();
            check(rows,1);normalize(rows);++captured;
        }
        if(!std::memcmp(magic,"N51CAND2",8)) {
            std::uint32_t pairs=0;
            if(!stream.read(reinterpret_cast<char*>(&pairs),4) || pairs>100000)
                throw std::runtime_error("invalid_normalization_pair_count");
            for(std::uint32_t i=0;i<pairs;++i) {
                const auto before=read_rows(),after=read_rows();
                if(normalize(before)!=after)
                    throw std::runtime_error("real_normalization_stage_mismatch");
                ++real_normalization_words;real_normalization_payloads+=before.size();
            }
        }
        if(stream.peek()!=std::char_traits<char>::eof())
            throw std::runtime_error("trailing_private_candidate_bytes");
    }
    std::cout<<"{\"synthetic_original_matches\":"<<checked-captured
             <<",\"captured_payload_matches\":"<<captured
             <<",\"synthetic_normalized_payload_matches\":"<<synthetic_normalized
             <<",\"captured_normalized_payload_matches\":"<<normalized-synthetic_normalized-real_normalization_payloads
             <<",\"real_normalization_word_matches\":"<<real_normalization_words
             <<",\"real_normalization_payload_matches\":"<<real_normalization_payloads
             <<",\"unexpected_buffer_mutations\":0}\n";
    return checked;
}
}
#endif
int main(int argc,char** argv) try {
    if((argc!=3 && argc!=5) || std::string(argv[1])!="--original" ||
       (argc==5 && std::string(argv[3])!="--input")) {
        std::cerr<<"usage: probe --original local-DLL [--input PRIVATE-candidates.bin]\n";return 2;
    }
#if defined(_WIN32) && !defined(_WIN64) && defined(_MSC_VER)
    return run(argv[2],argc==5?argv[4]:nullptr)?0:1;
#else
    throw std::runtime_error("original_probe_requires_win32_x86_msvc");
#endif
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
