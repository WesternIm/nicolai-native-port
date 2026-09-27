#include "nicolai/legacy_phone_features.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#if defined(_WIN32) && !defined(_WIN64)
#include <windows.h>
#endif

namespace {
using nicolai::LegacyPhoneDescriptorM36;
using nicolai::LegacyPhoneFeatureRecordM36;

struct CaseData {
    LegacyPhoneFeatureRecordM36 features;
    LegacyPhoneDescriptorM36 previous;
    LegacyPhoneDescriptorM36 next;
};

LegacyPhoneDescriptorM36 descriptor(std::vector<int> positions, int split,
    std::vector<std::int16_t> voicing) {
    LegacyPhoneDescriptorM36 d;
    d.source_position = std::move(positions);
    d.split_index = split;
    d.voicing = std::move(voicing);
    d.duration_q11.assign(d.source_position.size() - 1,
        static_cast<std::int16_t>(0x1357));
    d.pitch_q11.assign(d.source_position.size() - 1,
        static_cast<std::int16_t>(0x2468));
    return d;
}

#if defined(_WIN32) && !defined(_WIN64)
template <class T, class Buf>
void put(Buf& b, std::size_t off, T value) {
    if (off + sizeof(T) > b.size())
        throw std::runtime_error("probe_buffer_overflow");
    std::memcpy(b.data() + off, &value, sizeof(T));
}

template <class T, class Buf>
T get(const Buf& b, std::size_t off) {
    if (off + sizeof(T) > b.size())
        throw std::runtime_error("probe_buffer_overflow");
    T value{};
    std::memcpy(&value, b.data() + off, sizeof(T));
    return value;
}

std::vector<std::uint8_t> pack_descriptor(const LegacyPhoneDescriptorM36& d) {
    std::vector<std::uint8_t> raw(0x2200, 0);
    put<std::int32_t>(raw, 0x0c,
        static_cast<std::int32_t>(d.source_position.size()));
    put<std::int32_t>(raw, 0x10, d.split_index);
    for (std::size_t i = 0; i < d.source_position.size(); ++i)
        put<std::int32_t>(raw, 0x14 + i * 4, d.source_position[i]);
    for (std::size_t i = 0; i < d.voicing.size(); ++i) {
        put<std::int16_t>(raw, 0xfb4 + i * 2, d.voicing[i]);
        put<std::int16_t>(raw, 0x1784 + i * 2, d.duration_q11[i]);
        put<std::int16_t>(raw, 0x1f54 + i * 2, d.pitch_q11[i]);
    }
    return raw;
}

std::array<std::uint8_t, 0x80> pack_features(
    const LegacyPhoneFeatureRecordM36& f) {
    std::array<std::uint8_t, 0x80> raw{};
    put<std::int32_t>(raw, 0,
        static_cast<std::int32_t>(f.pitch_anchor.size()));
    for (std::size_t i = 0; i < f.interval_duration.size(); ++i)
        put<std::int16_t>(raw, 4 + i * 2, f.interval_duration[i]);
    for (std::size_t i = 0; i < f.pitch_anchor.size(); ++i)
        put<std::int16_t>(raw, 0x18 + i * 2, f.pitch_anchor[i]);
    return raw;
}

bool compare_descriptor(const std::vector<std::uint8_t>& raw,
    const LegacyPhoneDescriptorM36& d) {
    for (std::size_t i = 0; i < d.voicing.size(); ++i) {
        if (get<std::int16_t>(raw, 0xfb4 + i * 2) != d.voicing[i])
            return false;
        if (get<std::int16_t>(raw, 0x1784 + i * 2) != d.duration_q11[i])
            return false;
        if (get<std::int16_t>(raw, 0x1f54 + i * 2) != d.pitch_q11[i])
            return false;
    }
    return true;
}
#endif

CaseData generated_case(int seed) {
    std::vector<int> pp{0}, np{1000};
    for (int i = 0; i < 4; ++i) {
        pp.push_back(pp.back() + 60 + ((seed * 17 + i * 23) % 71));
        np.push_back(np.back() + 60 + ((seed * 29 + i * 19) % 71));
    }
    const int ps = 1 + seed % 3;
    const int ns = 1 + (seed / 3) % 3;
    std::vector<std::int16_t> pv(4, 1), nv(4, 1);
    if ((seed % 5) == 0) pv[(seed / 5) % 4] = 0;
    if ((seed % 7) == 0) nv[(seed / 7) % 4] = 0;

    const int support =
        (pp.back() - pp[ps]) + (np[ns] - np.front());
    const int n = 2 + seed % 3;
    std::vector<std::int16_t> duration(static_cast<std::size_t>(n));
    int remain = support;
    for (int i = 0; i < n; ++i) {
        const int left = n - i;
        const int value = (i == n - 1) ? remain : (remain / left);
        duration[static_cast<std::size_t>(i)] =
            static_cast<std::int16_t>(value);
        remain -= value;
    }

    static constexpr std::int16_t anchors[] = {70, 83, 100, 120, 160, 0};
    std::vector<std::int16_t> pitch(static_cast<std::size_t>(n + 1));
    for (int i = 0; i <= n; ++i)
        pitch[static_cast<std::size_t>(i)] = anchors[(seed + i * 2) % 6];

    CaseData c;
    c.features.interval_duration = std::move(duration);
    c.features.pitch_anchor = std::move(pitch);
    c.previous = descriptor(std::move(pp), ps, std::move(pv));
    c.next = descriptor(std::move(np), ns, std::move(nv));
    return c;
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 1 && argc != 2) {
        std::cerr << "usage: nicolai_m36_phone_probe "
                     "[original-mtsyc32.dll (Win32 x86 only)]\n";
        return 2;
    }
    try {
        std::size_t portable_cases = 0;
        std::size_t original_matches = 0;
#if defined(_WIN32) && !defined(_WIN64)
        using Builder = int(__cdecl*)(void*, void*, void*);
        Builder original = nullptr;
        HMODULE dll = nullptr;
        if (argc == 2) {
            dll = LoadLibraryExA(argv[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!dll) throw std::runtime_error("cannot_load_original_dll");
            const auto base = reinterpret_cast<const unsigned char*>(dll);
            const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
            const auto nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(
                base + dos->e_lfanew);
            if (nt->OptionalHeader.ImageBase != 0x10000000 ||
                nt->OptionalHeader.SizeOfImage != 0x797000 ||
                nt->FileHeader.TimeDateStamp != 0x412a0cb4)
                throw std::runtime_error(
                    "unsupported_dll_image_do_not_call_rvas");
            original = reinterpret_cast<Builder>(
                reinterpret_cast<std::uintptr_t>(dll) + 0x1a2780);
        }
#else
        (void)argv;
        if (argc == 2)
            throw std::runtime_error(
                "original_dll_probe_requires_Win32_x86_build");
#endif

        std::vector<CaseData> cases;
        {
            CaseData c;
            c.previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
            c.next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
            cases.push_back(std::move(c));
        }
        {
            CaseData c;
            c.features = {{160, 160}, {80, 80, 80}};
            c.previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
            c.next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
            cases.push_back(std::move(c));
        }
        {
            CaseData c;
            c.features = {{80, 80}, {80, 0, 120}};
            c.previous = descriptor({0, 80, 160, 240}, 1, {1, 1, 1});
            c.next = descriptor({1000, 1080, 1160, 1240}, 2, {1, 1, 1});
            cases.push_back(std::move(c));
        }
        for (int seed = 0; seed < 256; ++seed)
            cases.push_back(generated_case(seed));

        for (auto source : cases) {
            auto portable = source;
            const auto built = nicolai::legacy_phone_features_m36(
                portable.features, portable.previous, portable.next);
            if (!built.valid)
                throw std::runtime_error(
                    "portable_builder_rejected_supported_case");
            ++portable_cases;
#if defined(_WIN32) && !defined(_WIN64)
            if (original) {
                auto feature_raw = pack_features(source.features);
                auto previous_raw = pack_descriptor(source.previous);
                auto next_raw = pack_descriptor(source.next);
                const int rc = original(feature_raw.data(), previous_raw.data(),
                    next_raw.data());
                if (rc != 0)
                    throw std::runtime_error("original_builder_returned_error");
                for (std::size_t i = 0;
                    i < portable.features.pitch_anchor.size(); ++i) {
                    if (get<std::int16_t>(feature_raw, 0x18 + i * 2) !=
                        portable.features.pitch_anchor[i])
                        throw std::runtime_error(
                            "original_pitch_anchor_repair_mismatch");
                }
                if (!compare_descriptor(previous_raw, portable.previous) ||
                    !compare_descriptor(next_raw, portable.next))
                    throw std::runtime_error(
                        "original_phone_feature_lane_mismatch");
                ++original_matches;
            }
#endif
        }
#if defined(_WIN32) && !defined(_WIN64)
        if (dll) FreeLibrary(dll);
#endif
        std::cout << "{\"portable_cases\":" << portable_cases
                  << ",\"original_matches\":" << original_matches << "}\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
