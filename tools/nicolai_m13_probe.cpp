#include "nicolai/diphone_catalog.hpp"
#include "nicolai/psola_join.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/wav_writer.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

static double ms(std::size_t n, int sr) { return 1000.0 * n / sr; }

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: nicolai_m13_probe nicolai16.dat [out_dir]\n";
        return 2;
    }
    const std::filesystem::path dbpath = argv[1];
    const std::filesystem::path outdir = argc >= 3 ? argv[2] : ".";
    std::filesystem::create_directories(outdir);

    auto db = nicolai::VoiceDb::load(dbpath);
    const auto& bytes = db.bytes();
    const auto catalog = nicolai::parse_nicolai_diphone_catalog(bytes, db.metadata().edat);
    if (!catalog.valid) throw std::runtime_error(catalog.error);

    const std::vector<std::string> voiced = {"m", "a0", "m"};
    auto v = nicolai::synthesize_diphone_chain_m13(bytes, catalog, voiced);
    if (!v.valid) throw std::runtime_error(v.error);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m13_mam_voiced.wav", v.pcm);

    const std::vector<std::string> mama = {"#", "m", "a0", "m", "a0", "#"};
    auto m = nicolai::synthesize_diphone_chain_m13(bytes, catalog, mama);
    if (!m.valid) throw std::runtime_error(m.error);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m13_mama_skeleton.wav", m.pcm);

    std::cout << "Nicolai native-port M13 pitch-guided Hann OLA probe\n\n";
    std::cout << "voiced chain: m -> a0 -> m\n";
    std::cout << "  diphones: " << v.diphone_labels.size() << "\n";
    std::cout << "  samples: " << v.pcm.samples.size() << "\n";
    std::cout << "  duration_ms: " << ms(v.pcm.samples.size(), v.pcm.sample_rate) << "\n";
    for (std::size_t i = 0; i < v.joins.size(); ++i) {
        const auto& j = v.joins[i];
        std::cout << "  join[" << i << "]: overlap=" << j.overlap_samples
                  << " pitch=" << j.left_period_hint << "/" << j.right_period_hint
                  << " trim=" << j.left_trim << "/" << j.right_trim
                  << " corr=" << j.normalized_correlation << "\n";
    }

    std::cout << "\nprototype phone chain: # -> m -> a0 -> m -> a0 -> #\n";
    std::cout << "  diphones: " << m.diphone_labels.size() << "\n";
    std::cout << "  joins: " << m.joins.size() << "\n";
    std::cout << "  samples: " << m.pcm.samples.size() << "\n";
    std::cout << "  duration_ms: " << ms(m.pcm.samples.size(), m.pcm.sample_rate) << "\n";
    for (std::size_t i = 0; i < m.joins.size(); ++i) {
        const auto& j = m.joins[i];
        std::cout << "  join[" << i << "]: overlap=" << j.overlap_samples
                  << " pitch=" << j.left_period_hint << "/" << j.right_period_hint
                  << " trim=" << j.left_trim << "/" << j.right_trim
                  << " corr=" << j.normalized_correlation << "\n";
    }
    std::cout << "\nNOTE: M13 is pitch-guided Hann OLA, not yet bit-exact legacy Tempo-PSOLA.\n";
    return 0;
}
