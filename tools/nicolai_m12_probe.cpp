#include "nicolai/diphone_catalog.hpp"
#include "nicolai/edat.hpp"
#include "nicolai/g711.hpp"
#include "nicolai/voice_catalog.hpp"
#include "nicolai/wav_writer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

static std::vector<std::uint8_t> read_all(const char* path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), {}};
}

struct Stats {
    std::int16_t min = 0;
    std::int16_t max = 0;
    double mean = 0.0;
    double rms = 0.0;
    std::size_t zero_crossings = 0;
};

static Stats stats(const std::vector<std::int16_t>& s) {
    Stats r;
    if (s.empty()) return r;
    r.min = *std::min_element(s.begin(), s.end());
    r.max = *std::max_element(s.begin(), s.end());
    long double sum = 0.0L, sum2 = 0.0L;
    for (std::size_t i = 0; i < s.size(); ++i) {
        sum += s[i];
        sum2 += static_cast<long double>(s[i]) * s[i];
        if (i && ((s[i - 1] < 0) != (s[i] < 0))) ++r.zero_crossings;
    }
    r.mean = static_cast<double>(sum / s.size());
    r.rms = std::sqrt(static_cast<double>(sum2 / s.size()));
    return r;
}

int main(int argc, char** argv) {
    if (argc < 2 || (argc > 3 && (argc != 6 || std::string(argv[2]) != "--unit"))) {
        std::cerr << "usage: nicolai_m12_probe nicolai16.dat [out.wav]\n"
                  << "   or: nicolai_m12_probe nicolai16.dat --unit LEFT RIGHT out.wav\n";
        return 2;
    }
    const auto bytes = read_all(argv[1]);
    const auto layout = nicolai::parse_edat_layout(bytes);
    if (!layout.valid) { std::cerr << "invalid EDAT\n"; return 1; }

    const auto init_objects = nicolai::scan_legacy_file_objects(
        bytes, layout, nicolai::StaticDuration::Initialization);
    const auto* dsc_obj = nicolai::find_legacy_file_object(init_objects, "nbr16aci.dsc");
    if (!dsc_obj) { std::cerr << "nbr16aci.dsc not found\n"; return 1; }
    const auto acoustic = nicolai::parse_acoustic_descriptor(bytes, layout, *dsc_obj);
    if (!acoustic.valid) { std::cerr << "acoustic descriptor invalid\n"; return 1; }

    const auto cat = nicolai::parse_nicolai_diphone_catalog(bytes, layout);
    if (!cat.valid) { std::cerr << "diphone catalog failed: " << cat.error << "\n"; return 1; }

    if (argc == 6) {
        const auto* unit = nicolai::find_diphone(cat, argv[3], argv[4]);
        if (!unit) { std::cerr << "requested diphone not found\n"; return 1; }
        const auto encoded = nicolai::extract_diphone_compressed_bytes(bytes, cat, *unit);
        const auto pcm = nicolai::decode_g711_alaw_pcm(encoded, static_cast<int>(acoustic.sample_rate));
        if (pcm.samples.empty()) { std::cerr << "requested diphone decoded empty\n"; return 1; }
        int max_step = 0;
        std::size_t max_step_at = 0;
        for (std::size_t i = 1; i < pcm.samples.size(); ++i) {
            const int step = std::abs(static_cast<int>(pcm.samples[i]) - pcm.samples[i - 1]);
            if (step > max_step) { max_step = step; max_step_at = i; }
        }
        nicolai::write_wav_pcm16_mono(argv[5], pcm);
        std::cout << "unit=" << argv[3] << "->" << argv[4]
                  << " samples=" << pcm.samples.size()
                  << " max_step=" << max_step
                  << " max_step_at=" << max_step_at << "\n";
        return 0;
    }

    std::cout << "Nicolai native-port M12 direct A-law waveform probe\n\n";
    std::cout << "sample_rate: " << acoustic.sample_rate << " Hz\n";
    std::cout << "legacy signal coding field (+0x30): " << acoustic.raw_30 << "\n";
    std::cout << "decoded meaning: "
              << nicolai::to_string(static_cast<nicolai::LegacySignalCoding>(acoustic.raw_30))
              << "\n";
    std::cout << "x86 mapping recovered: 1=mu-law, 2=A-law, 3=linear, 0x50=cmp16-special\n\n";

    const std::vector<std::pair<std::string,std::string>> pairs{
        {"#", "p"}, {"#", "m"}, {"m", "a0"}, {"a0", "a0"}, {"p", "a0"}, {"o0", "a0"}
    };
    for (const auto& pr : pairs) {
        const auto* u = nicolai::find_diphone(cat, pr.first, pr.second);
        if (!u) { std::cout << pr.first << " -> " << pr.second << ": missing\n"; continue; }
        const auto encoded = nicolai::extract_diphone_compressed_bytes(bytes, cat, *u);
        const auto pcm = nicolai::decode_g711_alaw_pcm(encoded, static_cast<int>(acoustic.sample_rate));
        const auto st = stats(pcm.samples);
        const auto ms = 1000.0 * pcm.samples.size() / pcm.sample_rate;
        std::cout << pr.first << " -> " << pr.second
                  << ": bytes=" << encoded.size()
                  << " samples=" << pcm.samples.size()
                  << " duration_ms=" << std::fixed << std::setprecision(3) << ms
                  << " min=" << st.min << " max=" << st.max
                  << " mean=" << std::setprecision(3) << st.mean
                  << " rms=" << st.rms
                  << " zero_crossings=" << st.zero_crossings << "\n";
    }

    const auto* voiced = nicolai::find_diphone(cat, "a0", "a0");
    if (!voiced) { std::cerr << "a0 -> a0 not found\n"; return 1; }
    const auto encoded = nicolai::extract_diphone_compressed_bytes(bytes, cat, *voiced);
    const auto pcm = nicolai::decode_g711_alaw_pcm(encoded, static_cast<int>(acoustic.sample_rate));
    if (argc == 3) {
        nicolai::write_wav_pcm16_mono(argv[2], pcm);
        std::cout << "\nwrote raw voiced diphone WAV: " << argv[2] << "\n";
    }

    std::cout << "\nM12 invariant: Nicolai type 2 maps to legacy 'loi A'; ANA bytes decode 1:1 to PCM16 samples.\n";
    return 0;
}
