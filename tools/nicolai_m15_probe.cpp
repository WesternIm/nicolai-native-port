#include "nicolai/diphone_catalog.hpp"
#include "nicolai/hybrid_psola.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/wav_writer.hpp"

#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

static double ms(std::size_t n, int sr) { return 1000.0 * n / sr; }

static std::string run_shape(const nicolai::SegScheduleM15& s) {
    std::string x;
    for (const auto& r : s.runs) {
        if (!x.empty()) x += ",";
        x += r.voiced ? "+" : "-";
        x += std::to_string(r.slots);
    }
    return x;
}

static void print_chain(const char* name, const nicolai::DiphoneChainM15Result& r) {
    std::cout << "\n" << name << ": samples=" << r.pcm.samples.size()
              << " duration_ms=" << ms(r.pcm.samples.size(), r.pcm.sample_rate)
              << " joins=" << r.joins.size() << "\n";
    for (const auto& u : r.units) {
        std::cout << "  " << u.label
                  << " mode=" << u.schedule.mode
                  << " slots=" << u.schedule.total_slots
                  << " split=" << u.schedule.split_index
                  << " runs=[" << run_shape(u.schedule) << "]"
                  << " split_sample~" << u.layout.split_sample_estimate
                  << "\n";
        for (std::size_t i=0;i<u.runs.size();++i) {
            const auto& rd = u.runs[i];
            std::cout << "    run" << i << " " << (rd.voiced ? "voiced" : "unvoiced")
                      << " slots=" << rd.slots
                      << " src=[" << rd.source_begin << "," << rd.source_end << ")"
                      << " out=" << rd.output_samples;
            if (rd.voiced) {
                std::cout << " periods=" << rd.periods.size()
                          << " reset=" << (rd.signed_period_reset ? "yes" : "no")
                          << " TDPSOLA=" << (rd.used_td_psola ? "yes" : "no")
                          << " marks=" << rd.psola.source_marks << "->" << rd.psola.synthesis_marks
                          << " meanP=" << rd.psola.mean_source_period << "->" << rd.psola.mean_target_period;
            } else {
                std::cout << " duration-stretch=" << (rd.used_unvoiced_stretch ? "yes" : "no");
            }
            std::cout << "\n";
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: nicolai_m15_probe nicolai16.dat [out_dir]\n";
        return 2;
    }
    const std::filesystem::path dbpath = argv[1];
    const std::filesystem::path outdir = argc >= 3 ? argv[2] : ".";
    std::filesystem::create_directories(outdir);

    auto db = nicolai::VoiceDb::load(dbpath);
    const auto& bytes = db.bytes();
    const auto catalog = nicolai::parse_nicolai_diphone_catalog(bytes, db.metadata().edat);
    if (!catalog.valid) throw std::runtime_error(catalog.error);

    std::size_t parsed = 0, laid_out = 0;
    std::size_t all_voiced = 0, all_unvoiced = 0, mixed = 0;
    std::size_t signed_resets = 0;
    std::map<std::size_t,std::size_t> run_hist;
    for (const auto& u : catalog.units) {
        const auto s = nicolai::parse_seg_schedule_m15(u);
        if (!s.valid) {
            std::cerr << "SEG parse failure " << u.left_phone << "->" << u.right_phone
                      << ": " << s.error << "\n";
            return 3;
        }
        ++parsed;
        signed_resets += static_cast<std::size_t>(s.signed_period_resets);
        ++run_hist[s.runs.size()];
        if (s.voiced_slots == 0) ++all_unvoiced;
        else if (s.unvoiced_slots == 0) ++all_voiced;
        else ++mixed;
        const auto pcm_samples = static_cast<std::size_t>(u.compressed_end - static_cast<std::uint32_t>(u.compressed_start));
        const auto l = nicolai::layout_seg_runs_m15(s, pcm_samples);
        if (!l.valid || l.runs.empty() || l.runs.back().source_end != pcm_samples) {
            std::cerr << "SEG layout failure " << u.left_phone << "->" << u.right_phone
                      << ": " << l.error << "\n";
            return 4;
        }
        ++laid_out;
    }

    const std::vector<std::string> mama = {"#","m","a0","m","a0","#"};
    const std::vector<std::string> pa = {"#","p","a0","#"};

    auto neutral = nicolai::synthesize_diphone_chain_m15(bytes, catalog, mama, {1.0,1.0});
    auto high = nicolai::synthesize_diphone_chain_m15(bytes, catalog, mama, {1.20,1.0});
    auto slow = nicolai::synthesize_diphone_chain_m15(bytes, catalog, mama, {1.0,1.25});
    auto pa_neutral = nicolai::synthesize_diphone_chain_m15(bytes, catalog, pa, {1.0,1.0});
    if (!neutral.valid) throw std::runtime_error(neutral.error);
    if (!high.valid) throw std::runtime_error(high.error);
    if (!slow.valid) throw std::runtime_error(slow.error);
    if (!pa_neutral.valid) throw std::runtime_error(pa_neutral.error);

    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m15_mama_hybrid.wav", neutral.pcm);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m15_mama_pitch120.wav", high.pcm);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m15_mama_slow125.wav", slow.pcm);
    nicolai::write_wav_pcm16_mono(outdir / "nicolai_m15_pa_transition.wav", pa_neutral.pcm);

    std::cout << "Nicolai native-port M15 full SEG grammar + hybrid renderer\n\n";
    std::cout << "catalog units: " << catalog.units.size() << "\n";
    std::cout << "SEG parsed:    " << parsed << "\n";
    std::cout << "SEG laid out:  " << laid_out << "\n";
    std::cout << "all voiced:    " << all_voiced << "\n";
    std::cout << "all unvoiced:  " << all_unvoiced << "\n";
    std::cout << "mixed:         " << mixed << "\n";
    std::cout << "signed pitch resets: " << signed_resets << "\n";
    std::cout << "run-count histogram:";
    for (const auto& [runs,count] : run_hist) std::cout << " " << runs << "=>" << count;
    std::cout << "\n";

    print_chain("neutral mama", neutral);
    print_chain("pitch 1.20x mama", high);
    print_chain("duration 1.25x mama", slow);
    print_chain("neutral pa transition", pa_neutral);

    std::cout << "\nM15: no whole-unit raw fallback remains for signed/mixed SEG records.\n"
                 "Voiced runs use TD-PSOLA; unvoiced runs use duration-only processing;\n"
                 "run boundaries and diphone boundaries use raised-cosine OLA.\n";
    return 0;
}
