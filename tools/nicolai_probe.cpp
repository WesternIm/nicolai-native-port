#include "nicolai/engine.hpp"
#include "nicolai/edat.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <utility>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_probe /path/to/nicolai16.dat\n";
        return 2;
    }
    try {
        auto db = nicolai::VoiceDb::load(argv[1]);
        const auto& m = db.metadata();
        std::cout << "Nicolai native-port M1 EDAT probe\n";
        std::cout << "size_bytes: " << m.size_bytes << " (0x" << std::hex << m.size_bytes << std::dec << ")\n";
        std::cout << "tagged_magic: 0x" << std::hex << m.tagged_magic << std::dec << "\n";
        std::cout << "edat_valid: " << (m.edat.valid ? "yes" : "no") << "\n";
        if (m.edat.valid) {
            std::cout << "segment0: [0x" << std::hex << m.edat.segment0_offset << ", 0x" << m.edat.segment0_end
                      << ") size=0x" << m.edat.segment0_size << std::dec << "\n";
            std::cout << "segment1: [0x" << std::hex << m.edat.segment1_offset << ", 0x" << m.edat.segment1_end
                      << ") size=0x" << m.edat.segment1_size << std::dec << "\n";
            std::cout << "footer:   [0x" << std::hex << m.edat.footer_offset << ", 0x" << m.size_bytes
                      << ") size=0x" << m.edat.footer_size << std::dec << "\n";
        }

        std::map<std::pair<nicolai::EdatRegion, nicolai::EdatRegion>, std::size_t> counts;
        std::size_t invalid_targets = 0;
        for (const auto& r : m.tagged_refs) {
            ++counts[{r.source_region, r.target_region}];
            if (r.target_offset >= m.size_bytes) ++invalid_targets;
        }
        std::cout << "tagged_refs: " << m.tagged_refs.size() << "\n";
        std::cout << "invalid_tagged_targets: " << invalid_targets << "\n";
        for (const auto& [edge, count] : counts) {
            std::cout << "  " << nicolai::to_string(edge.first) << " -> " << nicolai::to_string(edge.second)
                      << ": " << count << "\n";
        }

        std::cout << "mode_records: " << m.mode_records.size() << "\n";
        for (std::size_t i = 0; i < m.mode_records.size(); ++i) {
            const auto& r = m.mode_records[i];
            std::cout << "  [" << i << "] offset=0x" << std::hex << r.offset << std::dec << "\n";
            std::cout << "      engine: " << r.engine_name << "\n";
            std::cout << "      description: " << r.description << "\n";
            std::cout << "      voice: " << r.voice_name << "\n";
            std::cout << "      style: " << r.style << " / " << r.pitch_name << "\n";
            std::cout << "      database: " << r.database_name << "\n";
            std::cout << "      sample_rate: " << r.sample_rate << "\n";
        }

        std::cout << "resource_paths: " << m.resource_paths.size() << "\n";
        for (const auto& p : m.resource_paths) {
            std::cout << "  0x" << std::hex << p.offset << std::dec << ": " << p.value << "\n";
        }

        nicolai::Engine engine(std::move(db));
        const auto result = engine.synthesize("test");
        std::cout << "synthesis_status: " << nicolai::to_string(result.status) << "\n";
        std::cout << "synthesis_message: " << result.message << "\n";
        return m.edat.valid && !m.mode_records.empty() ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
