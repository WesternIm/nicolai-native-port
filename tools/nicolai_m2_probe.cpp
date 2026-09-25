#include "nicolai/assets.hpp"
#include "nicolai/huffman.hpp"
#include "nicolai/voice_db.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m2_probe /path/to/nicolai16.dat\n";
        return 2;
    }

    try {
        const auto db = nicolai::VoiceDb::load(argv[1]);
        const auto& m = db.metadata();
        const auto assets = nicolai::locate_nicolai_voice_assets(db.bytes(), m.edat, m.tagged_refs);

        std::cout << "Nicolai native-port M2 acoustic payload probe\n";
        std::cout << "assets_valid: " << (assets.valid ? "yes" : "no") << "\n";
        if (!assets.valid) {
            std::cout << "error: " << assets.error << "\n";
            return 1;
        }

        auto ph = [](std::uint64_t x) {
            std::cout << "0x" << std::hex << x << std::dec;
        };

        std::cout << "ana_path_offset: "; ph(assets.ana_path_offset); std::cout << "\n";
        std::cout << "ana_descriptor_offset: "; ph(assets.ana_descriptor_offset); std::cout << "\n";
        std::cout << "analysis_object_offset: "; ph(assets.analysis_object_offset); std::cout << "\n";
        std::cout << "seg_object_offset: "; ph(assets.seg_object_offset); std::cout << "\n";
        std::cout << "analysis_data: ["; ph(assets.analysis_data_offset); std::cout << ", ";
        ph(assets.analysis_data_end); std::cout << ") size=" << assets.analysis_data_size << "\n";
        std::cout << "compressed_voice: ["; ph(assets.compressed_voice_offset); std::cout << ", ";
        ph(assets.compressed_voice_end); std::cout << ") size=" << assets.compressed_voice_size << "\n";
        std::cout << "next_object: "; ph(assets.next_object_offset);
        std::cout << " ascii=\"" << assets.next_object_ascii << "\"\n";

        // Self-test the portable Huffman traversal with a tiny known tree:
        // 0 -> symbol 11, 10 -> symbol 22, 11 -> symbol 33.
        const std::vector<nicolai::HuffmanNode> tree = {
            {false, 0, 1, 2},
            {true, 11, -1, -1},
            {false, 0, 3, 4},
            {true, 22, -1, -1},
            {true, 33, -1, -1},
        };
        const std::vector<std::uint16_t> bits = {1, 0};
        std::size_t bit_index = 0;
        const auto decoded = nicolai::decode_huffman_word_bits(tree, 0, bits, bit_index, bits.size());
        std::cout << "huffman_selftest: " << nicolai::to_string(decoded.status)
                  << " symbol=" << decoded.symbol
                  << " bits=" << decoded.bits_consumed << "\n";

        return decoded.status == nicolai::HuffmanStatus::Ok && decoded.symbol == 22 ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
