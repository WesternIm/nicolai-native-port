#include "nicolai/voice_db.hpp"
#include "nicolai/legacy_prosody.hpp"
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m26_probe nicolai16.dat\n";
        return 2;
    }
    try {
        auto db = nicolai::VoiceDb::load(argv[1]);
        auto p = nicolai::parse_legacy_russian_physical(db.bytes(), db.metadata().edat);
        if (!p.valid) {
            std::cerr << "physical.int: " << p.error << "\n";
            return 1;
        }
        std::cout << "physical.int records=" << p.records.size() << " stride=42\n";
        for (std::size_t r = 0; r < p.records.size(); ++r) {
            std::cout << "record " << r << ':';
            for (auto v : p.records[r].values) std::cout << ' ' << static_cast<int>(v);
            std::cout << '\n';
        }
        if (p.records.size() > 1) {
            for (double x : {0.0, 0.5, 1.0}) {
                const auto t = nicolai::legacy_physical_pitch_triplet(p.records[1], x, 18);
                std::cout << "class1 x=" << std::fixed << std::setprecision(2) << x
                          << " t=" << t[0] << ',' << t[1] << ',' << t[2] << '\n';
            }
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
