#include "nicolai/voice_db.hpp"
#include "nicolai/seg_schedule.hpp"
#include "nicolai/legacy_tds.hpp"
#include <array>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#if defined(_WIN32) && !defined(_WIN64)
#include <windows.h>
#endif

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "usage: nicolai_m33_probe nicolai16.dat [original-mtsyc32.dll (Win32 only)]\n";
        return 2;
    }
    try {
        auto db = nicolai::VoiceDb::load(argv[1]);
        auto catalog = nicolai::parse_nicolai_diphone_catalog(db.bytes(), db.metadata().edat);
        if (!catalog.valid) throw std::runtime_error(catalog.error);
        std::size_t valid = 0, rejected = 0, node_matches = 0, step_matches = 0;
#if defined(_WIN32) && !defined(_WIN64)
        HMODULE dll = nullptr;
        using TimelineFn = void(__cdecl*)(const void*, int, int, void*, void*);
        using StepFn = int(__cdecl*)(void*, void*, int, int, int, int, int, int, int);
        TimelineFn oracle_timeline = nullptr;
        StepFn oracle_step = nullptr;
        if (argc == 3) {
            dll = LoadLibraryExA(argv[2], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!dll) throw std::runtime_error("cannot_load_original_dll");
            auto base = reinterpret_cast<const unsigned char*>(dll);
            auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
            auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
            if (nt->OptionalHeader.ImageBase != 0x10000000 ||
                nt->OptionalHeader.SizeOfImage != 0x797000 ||
                nt->FileHeader.TimeDateStamp != 0x412a0cb4)
                throw std::runtime_error("unsupported_dll_image_do_not_call_rvas");
            oracle_timeline = reinterpret_cast<TimelineFn>(reinterpret_cast<std::uintptr_t>(dll) + 0x1a30b0);
            oracle_step = reinterpret_cast<StepFn>(reinterpret_cast<std::uintptr_t>(dll) + 0x109800);
        }
#else
        if (argc == 3) throw std::runtime_error("original_dll_probe_requires_Win32_build");
#endif
        for (const auto& unit : catalog.units) {
            auto s = nicolai::parse_seg_schedule_m15(unit);
            const auto count = unit.compressed_end - static_cast<std::uint32_t>(unit.compressed_start);
            auto timeline = nicolai::source_timeline_seg_m33(s, count, unit.signed_end >= 0);
            if (!timeline.valid) { ++rejected; continue; }
            ++valid;
#if defined(_WIN32) && !defined(_WIN64)
            if (oracle_timeline) {
                std::vector<std::int16_t> raw(4 + unit.metadata.size());
                const std::int32_t start = 0, end = unit.signed_end >= 0 ? count : -static_cast<int>(count);
                std::memcpy(raw.data(), &start, 4);
                std::memcpy(raw.data() + 2, &end, 4);
                std::copy(unit.metadata.begin(), unit.metadata.end(), raw.begin() + 4);
                std::array<std::int32_t, 0x840 / 4> descriptor{};
                oracle_timeline(raw.data(), 16000, 2, descriptor.data(), nullptr);
                bool matches = descriptor[0x3c / 4] == static_cast<int>(timeline.nodes.size()) &&
                    descriptor[0x28 / 4] == static_cast<int>(timeline.split_node);
                for (std::size_t i = 0; i < timeline.nodes.size(); ++i) {
                    matches = matches && descriptor[0x40 / 4 + 2 * i] == static_cast<int>(timeline.nodes[i].sample) &&
                        descriptor[0x44 / 4 + 2 * i] == static_cast<int>(timeline.nodes[i].voiced);
                }
                if (!matches) throw std::runtime_error("timeline_oracle_mismatch_" + unit.left_phone + "_" + unit.right_phone);
                ++node_matches;
            }
#endif
        }
        // Deterministic synthetic contract cases: unity, dropped/repeated
        // grains, carry, rising/falling periods, saturation, short support.
        std::vector<std::array<int,6>> cases{
            {{100,2048,2048,100,2048,0}}, {{100,2048,1024,100,2048,0}},
            {{100,2048,1024,100,2048,50}}, {{100,2048,4096,100,2048,0}},
            {{160,2048,3072,180,2048,17}}, {{180,2048,3072,160,2048,-9}},
            {{200,4096,2048,210,4096,0}}, {{100,2048,1,100,2048,0}}
        };
        for (int width : {80,120,160,240,320})
            for (int pitch : {1536,2048,3072})
                for (int duration : {1024,1536,2048,3072,4096})
                    for (int next_width : {width-40,width,width+40})
                        for (int carry : {-20,0,20})
                            cases.push_back({width,pitch,duration,next_width,pitch,carry});
        for (const auto& c : cases) {
            auto step = nicolai::legacy_tds_step_m33(c[0],c[1],c[2],c[3],c[4],c[5]);
            if (!step.valid) throw std::runtime_error("step_case_invalid");
#if defined(_WIN32) && !defined(_WIN64)
            if (oracle_step) {
                std::array<std::int16_t,5> record{{0,0,0,0,static_cast<std::int16_t>(c[5])}};
                oracle_step(nullptr,record.data(),c[0],c[1],c[2],0,c[3],c[4],0);
                if (record[1]!=step.count || record[2]!=step.first_period ||
                    record[3]!=step.delta_q11 || record[4]!=step.carry)
                    throw std::runtime_error("step_oracle_mismatch");
                ++step_matches;
            }
#endif
        }
        std::cout << "{\"catalog_units\":" << catalog.units.size() << ",\"timeline_valid\":" << valid
                  << ",\"timeline_rejected\":" << rejected << ",\"original_timeline_matches\":" << node_matches
                  << ",\"step_cases\":" << cases.size()
                  << ",\"original_step_matches\":" << step_matches << "}\n";
        return rejected ? 3 : 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n"; return 1;
    }
}
