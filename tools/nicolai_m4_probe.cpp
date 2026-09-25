#include "nicolai/cmp16.hpp"
#include "nicolai/voice_db.hpp"
#include "nicolai/assets.hpp"
#include "nicolai/ana_index.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m4_probe /path/to/nicolai16.dat\n";
        return 2;
    }
    try {
        const auto db = nicolai::VoiceDb::load(argv[1]);
        const auto& b = db.bytes();
        const auto& meta = db.metadata();
        const auto assets = nicolai::locate_nicolai_voice_assets(b, meta.edat, meta.tagged_refs);
        if (!assets.valid) {
            std::cerr << "assets: " << assets.error << "\n";
            return 1;
        }
        const std::vector<std::uint8_t> analysis(
            b.begin() + static_cast<std::ptrdiff_t>(assets.analysis_data_offset),
            b.begin() + static_cast<std::ptrdiff_t>(assets.analysis_data_end));
        const std::vector<std::uint8_t> voice(
            b.begin() + static_cast<std::ptrdiff_t>(assets.compressed_voice_offset),
            b.begin() + static_cast<std::ptrdiff_t>(assets.compressed_voice_end));
        const auto units = nicolai::parse_ana_unit_index(analysis, static_cast<std::uint32_t>(voice.size()));

        std::cout << "Nicolai native-port M4 codec reconstruction probe\n\n";
        std::cout << "acoustic_assets: " << (assets.valid ? "yes" : "no") << "\n";
        std::cout << "ana_units: " << units.records.size() << "\n";
        std::cout << "compressed_bytes: " << voice.size() << "\n";
        std::cout << "indexed_bytes: " << units.covered_bytes << "\n\n";

        std::cout << "cmp16sbi resource manifest (recovered loader map):\n";
        for (const auto& s : nicolai::cmp16_resource_specs()) {
            std::cout << "  slot 0x" << std::hex << std::setw(4) << std::setfill('0')
                      << s.descriptor_slot << std::dec << std::setfill(' ')
                      << "  " << std::left << std::setw(12) << nicolai::to_string(s.role)
                      << std::right << " parser=0x" << std::hex << s.legacy_parser_va
                      << std::dec << "\n";
        }
        std::cout << "\nrecovered numeric transforms:\n";
        std::cout << "  QMLT: fixed Q13 = trunc(x * 8192)\n";
        std::cout << "  QRms: trunc(4 * pow(10, x * 0.05))\n";
        std::cout << "\nfirst indexed compressed range:\n";
        if (!units.records.empty()) {
            const auto& u = units.records.front();
            std::cout << "  [" << u.compressed_start << ", " << u.compressed_end << ")  "
                      << (u.compressed_end - u.compressed_start) << " bytes\n";
        }
        std::cout << "\nstatus: manifest/quantizer readers recovered; static-resource materialization still pending\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
