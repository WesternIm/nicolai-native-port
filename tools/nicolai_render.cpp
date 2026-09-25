#include "nicolai/engine.hpp"
#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"
#include "nicolai/wav_writer.hpp"

#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cerr << "usage: nicolai_render nicolai16.dat exc_rus.txt abb_rus.txt out.wav text...\n";
        return 2;
    }
    try {
        const std::filesystem::path db_path = argv[1];
        const std::filesystem::path exc_path = argv[2];
        const std::filesystem::path abb_path = argv[3];
        const std::filesystem::path out_path = argv[4];
        std::string text;
        for (int i = 5; i < argc; ++i) {
            if (!text.empty()) text += ' ';
            text += argv[i];
        }

        auto db = nicolai::VoiceDb::load(db_path);
        auto stress = nicolai::load_exc_rus_cp1251(exc_path);
        auto exceptions = nicolai::load_exc_rus_replacements_cp1251(exc_path);
        auto abbreviations = nicolai::load_abb_rus_cp1251(abb_path);
        nicolai::Engine engine(std::move(db), std::move(stress),
                               std::move(abbreviations), std::move(exceptions));
        auto result = engine.synthesize(text);
        if (result.status != nicolai::SynthStatus::Ok) {
            std::cerr << "synthesis failed: " << result.message << "\n";
            return 1;
        }
        nicolai::write_wav_pcm16_mono(out_path, result.pcm);
        std::cout << "text=" << text << "\n"
                  << "samples=" << result.pcm.samples.size() << "\n"
                  << "rate=" << result.pcm.sample_rate << "\n"
                  << "wav=" << out_path.string() << "\n"
                  << "pipeline=" << result.message << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
