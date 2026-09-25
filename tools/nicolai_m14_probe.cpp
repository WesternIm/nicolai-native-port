#include "nicolai/diphone_catalog.hpp"
#include "nicolai/td_psola.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/wav_writer.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

static double ms(std::size_t n, int sr) { return 1000.0 * n / sr; }

static void print_unit(const nicolai::M14UnitDiagnostics& u) {
    std::cout << "  " << u.label << ": ";
    if (!u.schedule.valid) {
        std::cout << "raw/mixed SEG (" << u.schedule.error << ")\n";
        return;
    }
    std::cout << "TD-PSOLA periods=" << u.schedule.periods.size()
              << " split=" << u.schedule.split_index
              << " margins=" << u.schedule.left_margin << "/" << u.schedule.right_margin
              << " src_marks=" << u.psola.source_marks
              << " synth_marks=" << u.psola.synthesis_marks
              << " grains=" << u.psola.grains_added
              << " meanP=" << u.psola.mean_source_period << "->" << u.psola.mean_target_period
              << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: nicolai_m14_probe nicolai16.dat [out_dir]\n";
        return 2;
    }
    const std::filesystem::path dbpath = argv[1];
    const std::filesystem::path outdir = argc >= 3 ? argv[2] : ".";
    std::filesystem::create_directories(outdir);

    auto db = nicolai::VoiceDb::load(dbpath);
    const auto& bytes = db.bytes();
    const auto catalog = nicolai::parse_nicolai_diphone_catalog(bytes, db.metadata().edat);
    if (!catalog.valid) throw std::runtime_error(catalog.error);

    const std::vector<std::string> mama = {"#","m","a0","m","a0","#"};

    auto neutral = nicolai::synthesize_diphone_chain_m14(bytes, catalog, mama, {1.0,1.0});
    if (!neutral.valid) throw std::runtime_error(neutral.error);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m14_mama_tdpsola.wav", neutral.pcm);

    auto high = nicolai::synthesize_diphone_chain_m14(bytes, catalog, mama, {1.20,1.0});
    if (!high.valid) throw std::runtime_error(high.error);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m14_mama_pitch120.wav", high.pcm);

    auto slow = nicolai::synthesize_diphone_chain_m14(bytes, catalog, mama, {1.0,1.25});
    if (!slow.valid) throw std::runtime_error(slow.error);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m14_mama_slow125.wav", slow.pcm);

    const auto hh = nicolai::legacy_half_hann_q15(160);
    std::cout << "Nicolai native-port M14 TD-PSOLA probe\n\n";
    std::cout << "legacy Hanning primitive: P=160 first/mid/last="
              << hh.front() << "/" << hh[80] << "/" << hh.back() << "\n";

    std::cout << "\nneutral mama chain: samples=" << neutral.pcm.samples.size()
              << " duration_ms=" << ms(neutral.pcm.samples.size(), neutral.pcm.sample_rate) << "\n";
    for (const auto& u : neutral.units) print_unit(u);
    std::cout << "joins=" << neutral.joins.size() << "\n";

    std::cout << "\npitch 1.20x: samples=" << high.pcm.samples.size()
              << " duration_ms=" << ms(high.pcm.samples.size(), high.pcm.sample_rate) << "\n";
    for (const auto& u : high.units) print_unit(u);

    std::cout << "\nduration 1.25x: samples=" << slow.pcm.samples.size()
              << " duration_ms=" << ms(slow.pcm.samples.size(), slow.pcm.sample_rate) << "\n";
    for (const auto& u : slow.units) print_unit(u);

    std::cout << "\nNOTE: clean voiced SEG records are now grain-resynthesized pitch-synchronously.\n"
                 "Mixed/unvoiced signed SEG records still use decoded source PCM until their control grammar is recovered.\n";
    return 0;
}
