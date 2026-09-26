#include "nicolai/legacy_tds.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#if defined(_WIN32) && !defined(_WIN64)
#include <windows.h>
#endif

int main(int argc, char** argv) {
    if (argc != 1 && argc != 2) { std::cerr << "usage: nicolai_m34_probe [original-mtsyc32.dll (Win32 only)]\n"; return 2; }
    try {
        std::size_t writes=0, write_matches=0, reciprocal=0, reciprocal_matches=0;
        std::size_t sequential=0, sequential_matches=0;
#if defined(_WIN32) && !defined(_WIN64)
        using Writer = void(__cdecl*)(void*,const void*,const void*,void*,int,const void*,int,const void*,int,int);
        using Reciprocal = int(__cdecl*)(int,int,int,int);
        using Step = int(__cdecl*)(void*,void*,int,int,int,int,int,int,int);
        Writer writer=nullptr; Reciprocal interp=nullptr; Step step_fn=nullptr;
        if (argc==2) {
            const auto dll=LoadLibraryExA(argv[1],nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!dll) throw std::runtime_error("cannot_load_original_dll");
            const auto base=reinterpret_cast<const unsigned char*>(dll);
            const auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
            const auto nt=reinterpret_cast<const IMAGE_NT_HEADERS*>(base+dos->e_lfanew);
            if(nt->OptionalHeader.ImageBase!=0x10000000 || nt->OptionalHeader.SizeOfImage!=0x797000 ||
               nt->FileHeader.TimeDateStamp!=0x412a0cb4) throw std::runtime_error("unsupported_dll_image_do_not_call_rvas");
            writer=reinterpret_cast<Writer>(reinterpret_cast<std::uintptr_t>(dll)+0x109980);
            interp=reinterpret_cast<Reciprocal>(reinterpret_cast<std::uintptr_t>(dll)+0x1a2c00);
            step_fn=reinterpret_cast<Step>(reinterpret_cast<std::uintptr_t>(dll)+0x109800);
        }
#else
        if(argc==2) throw std::runtime_error("original_dll_probe_requires_Win32_build");
#endif
        // Caller-valid short/equal/overlapping/gapped windows. Synthetic,
        // arbitrary signed sample/window data exercises rounding, not speech.
        for(int period : {1,2,3,20,80,160,320,400})
            for(int ll : {0,period/4,period/2,period})
                for(int rl : {0,period/4,period/2,period})
                    for(int seed=1;seed<=8;++seed) {
                        std::vector<std::int16_t> left(ll),right(rl),lw(ll),rw(rl);
                        for(int i=0;i<ll;++i) { left[i]=static_cast<std::int16_t>((i*193+seed*719)%60001-30000); lw[i]=static_cast<std::int16_t>((i*811+seed*53)%32768); }
                        for(int i=0;i<rl;++i) { right[i]=static_cast<std::int16_t>((i*419+seed*331)%60001-30000); rw[i]=static_cast<std::int16_t>((i*397+seed*997)%32768); }
                        std::vector<std::int16_t> actual(period+18,1234);
                        if(!nicolai::legacy_tds_write_m34(actual,7,period,left,right,lw,rw)) throw std::runtime_error("writer_rejected_valid_case");
                        ++writes;
#if defined(_WIN32) && !defined(_WIN64)
                        if(writer) {
                            auto expected=std::vector<std::int16_t>(period+18,1234);
                            std::array<std::uint32_t,0x90/4> state{};
                            state[8/4]=reinterpret_cast<std::uintptr_t>(expected.data());
                            state[0x48/4]=7; state[0x3c/4]=11;
                            writer(nullptr,left.data(),right.data(),state.data(),period,lw.data(),ll,rw.data(),rl,0);
                            if(expected!=actual || state[0x3c/4]!=static_cast<unsigned>(11+period) || state[0x48/4]!=7)
                                throw std::runtime_error("writer_oracle_mismatch");
                            ++write_matches;
                        }
#endif
                    }
        for(int start : {40,83,100,160,300}) for(int end : {40,83,100,160,300})
            for(int length : {1,20,100,701}) for(int pos : {0,length/4,length/2,length}) {
                const auto actual=nicolai::legacy_reciprocal_pitch_m34(start,length,end,pos); ++reciprocal;
                if(!actual) throw std::runtime_error("reciprocal_rejected_valid_case");
#if defined(_WIN32) && !defined(_WIN64)
                if(interp) { if(interp(start,length,end,pos)!=actual) throw std::runtime_error("reciprocal_oracle_mismatch"); ++reciprocal_matches; }
#endif
            }
        int carry=0;
#if defined(_WIN32) && !defined(_WIN64)
        std::array<std::int16_t,5> record{};
#endif
        for(int i=0;i<1000;++i) {
            const int width=80+(i*37)%180, nw=80+((i+1)*37)%180;
            const int pitch=1536+(i%3)*512, np=1536+((i+1)%3)*512;
            const int duration=1024+(i%5)*384;
            const auto step=nicolai::legacy_tds_step_m33(width,pitch,duration,nw,np,carry);
            if(!step.valid) throw std::runtime_error("sequence_rejected_valid_case");
            carry=step.carry; ++sequential;
#if defined(_WIN32) && !defined(_WIN64)
            if(step_fn) {
                step_fn(nullptr,record.data(),width,pitch,duration,0,nw,np,0);
                if(record[1]!=step.count || record[2]!=step.first_period || record[3]!=step.delta_q11 || record[4]!=step.carry)
                    throw std::runtime_error("sequence_oracle_mismatch");
                ++sequential_matches;
            }
#endif
        }
        std::cout << "{\"writer_cases\":" << writes << ",\"original_writer_matches\":" << write_matches
            << ",\"reciprocal_cases\":" << reciprocal << ",\"original_reciprocal_matches\":" << reciprocal_matches
            << ",\"sequential_step_cases\":" << sequential << ",\"original_sequential_step_matches\":" << sequential_matches << "}\n";
        return 0;
    } catch(const std::exception& e) { std::cerr << e.what() << "\n"; return 1; }
}
