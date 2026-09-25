#include "nicolai/engine.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/wav_writer.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static void print_front(const std::string& text) {
    const auto r = nicolai::russian_text_to_nicolai_phones(text);
    std::cout << "text: " << text << "\n";
    if (!r.valid) { std::cout << "  ERROR: " << r.error << "\n"; return; }
    std::cout << "  normalized: " << r.normalized_utf8 << "\n  phones:";
    for (const auto& p : r.phones) std::cout << " " << p;
    std::cout << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: nicolai_m16_probe nicolai16.dat [out_dir]\n";
        return 2;
    }
    const std::filesystem::path dbpath = argv[1];
    const std::filesystem::path outdir = argc >= 3 ? argv[2] : ".";
    std::filesystem::create_directories(outdir);

    const std::vector<std::pair<std::string,std::string>> cases = {
        {"мама", "nicolai_m16_mama_from_text.wav"},
        {"папа", "nicolai_m16_papa_from_text.wav"},
        {"яма",  "nicolai_m16_yama_from_text.wav"},
        {"мир",  "nicolai_m16_mir_from_text.wav"},
        {"привет", "nicolai_m16_privet_from_text.wav"},
        {"мама папа", "nicolai_m16_mama_papa_from_text.wav"},
    };

    for (const auto& c : cases) print_front(c.first);

    auto db = nicolai::VoiceDb::load(dbpath);
    nicolai::Engine engine(std::move(db));
    for (const auto& [text, file] : cases) {
        const auto r = engine.synthesize(text);
        std::cout << "synth '" << text << "': " << nicolai::to_string(r.status)
                  << " samples=" << r.pcm.samples.size()
                  << " message='" << r.message << "'\n";
        if (r.status == nicolai::SynthStatus::Ok) {
            nicolai::write_wav_pcm16_mono(outdir / file, r.pcm);
        } else {
            std::cout << "  synthesis skipped: " << r.message << "\n";
        }
    }
    return 0;
}
