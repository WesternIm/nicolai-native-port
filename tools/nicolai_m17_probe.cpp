#include "nicolai/engine.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/russian_stress.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/wav_writer.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static void print_front(const std::string& text, const nicolai::RussianStressDictionary* dict) {
    nicolai::RussianFrontendOptions o;
    o.stress_dictionary = dict;
    const auto r = nicolai::russian_text_to_nicolai_phones(text, o);
    std::cout << "text: " << text << "\n";
    if (!r.valid) { std::cout << "  ERROR: " << r.error << "\n"; return; }
    std::cout << "  phones:";
    for (const auto& p : r.phones) std::cout << " " << p;
    std::cout << "\n";
    for (const auto& w : r.words)
        std::cout << "  word='" << w.source_utf8 << "' stress_vowel=" << w.stress_vowel_index
                  << " source=" << w.stress_source << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: nicolai_m17_probe nicolai16.dat [exc_rus.txt] [out_dir]\n";
        return 2;
    }
    const std::filesystem::path dbpath = argv[1];
    const bool have_dict = argc >= 3 && std::string(argv[2]) != "-";
    const std::filesystem::path dictpath = have_dict ? argv[2] : "";
    const std::filesystem::path outdir = argc >= 4 ? argv[3] : ".";
    std::filesystem::create_directories(outdir);

    nicolai::RussianStressDictionary dict;
    if (have_dict) {
        dict = nicolai::load_exc_rus_cp1251(dictpath);
        std::cout << "stress dictionary: valid=" << (dict.valid ? "yes" : "no")
                  << " entries=" << dict.entries << " lines=" << dict.parsed_lines
                  << " error='" << dict.error << "'\n";
    }

    const std::vector<std::pair<std::string,std::string>> cases = {
        {"молоко", "nicolai_m17_moloko.wav"},
        {"мо\xCC\x81локо", "nicolai_m17_moloko_stress_first.wav"},
        {"привет", "nicolai_m17_privet.wav"},
        {"сказка", "nicolai_m17_skazka.wav"},
        {"вокзал", "nicolai_m17_vokzal.wav"},
        {"подписка", "nicolai_m17_podpiska.wav"},
        {"мама папа", "nicolai_m17_mama_papa.wav"},
    };

    for (const auto& c : cases) print_front(c.first, dict.valid ? &dict : nullptr);

    auto db = nicolai::VoiceDb::load(dbpath);
    nicolai::Engine engine = dict.valid
        ? nicolai::Engine(std::move(db), dict)
        : nicolai::Engine(std::move(db));

    for (const auto& [text, file] : cases) {
        const auto r = engine.synthesize(text);
        std::cout << "synth '" << text << "': " << nicolai::to_string(r.status)
                  << " samples=" << r.pcm.samples.size()
                  << " message='" << r.message << "'\n";
        if (r.status == nicolai::SynthStatus::Ok)
            nicolai::write_wav_pcm16_mono(outdir / file, r.pcm);
    }
    return 0;
}
