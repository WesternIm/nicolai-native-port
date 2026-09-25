#include "nicolai/ana_index.hpp"
#include "nicolai/assets.hpp"
#include "nicolai/huffman.hpp"
#include "nicolai/voice_db.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m3_probe /path/to/nicolai16.dat\n";
        return 2;
    }

    try {
        const auto db = nicolai::VoiceDb::load(argv[1]);
        const auto& meta = db.metadata();
        const auto assets = nicolai::locate_nicolai_voice_assets(db.bytes(), meta.edat, meta.tagged_refs);
        if (!assets.valid) {
            std::cerr << "asset discovery failed: " << assets.error << "\n";
            return 1;
        }

        const auto& bytes = db.bytes();
        std::vector<std::uint8_t> analysis(
            bytes.begin() + static_cast<std::ptrdiff_t>(assets.analysis_data_offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(assets.analysis_data_end));
        std::vector<std::uint8_t> voice(
            bytes.begin() + static_cast<std::ptrdiff_t>(assets.compressed_voice_offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(assets.compressed_voice_end));

        const auto idx = nicolai::parse_ana_unit_index(
            analysis, static_cast<std::uint32_t>(voice.size()));

        std::cout << "Nicolai native-port M3 acoustic unit directory\n";
        std::cout << "index_valid: " << (idx.valid ? "yes" : "no") << "\n";
        if (!idx.valid) {
            std::cout << "error: " << idx.error << "\n";
            return 1;
        }
        std::cout << "units: " << idx.records.size() << "\n";
        std::cout << "payload_bytes: " << idx.payload_bytes << "\n";
        std::cout << "covered_bytes: " << idx.covered_bytes << "\n";
        std::cout << "trailing_payload_bytes: " << idx.trailing_payload_bytes << "\n";
        std::cout << "trailing_analysis_bytes: " << idx.trailing_analysis_bytes << "\n";

        std::size_t negative_ends = 0;
        std::uint64_t sum = 0;
        std::size_t min_len = static_cast<std::size_t>(-1), max_len = 0;
        std::map<std::size_t, std::size_t> record_sizes;
        for (const auto& r : idx.records) {
            if (r.signed_end < 0) ++negative_ends;
            const auto len = static_cast<std::size_t>(r.compressed_end) -
                             static_cast<std::size_t>(r.compressed_start);
            min_len = std::min(min_len, len);
            max_len = std::max(max_len, len);
            sum += len;
            ++record_sizes[r.record_size];
        }
        std::cout << "signed_end_negative: " << negative_ends << "\n";
        std::cout << "signed_end_positive: " << idx.records.size() - negative_ends << "\n";
        std::cout << "compressed_unit_len_min: " << min_len << "\n";
        std::cout << "compressed_unit_len_max: " << max_len << "\n";
        std::cout << "compressed_unit_len_mean: "
                  << std::fixed << std::setprecision(2)
                  << (static_cast<double>(sum) / idx.records.size()) << "\n";

        std::cout << "first_units:\n";
        const auto show = std::min<std::size_t>(8, idx.records.size());
        for (std::size_t i = 0; i < show; ++i) {
            const auto& r = idx.records[i];
            std::cout << "  #" << r.index
                      << " ana=0x" << std::hex << r.table_offset << std::dec
                      << " voice=[" << r.compressed_start << "," << r.compressed_end << ")"
                      << " bytes=" << (r.compressed_end - r.compressed_start)
                      << " signed_end=" << r.signed_end
                      << " rec_size=" << r.record_size
                      << " metadata_words=" << r.metadata.size() << "\n";
        }

        const auto& last = idx.records.back();
        std::cout << "terminal_unit: #" << last.index
                  << " ana=0x" << std::hex << last.table_offset << std::dec
                  << " voice=[" << last.compressed_start << "," << last.compressed_end << ")"
                  << " record_size=" << last.record_size << "\n";

        // Verify we can isolate an individual legacy acoustic unit without
        // executing any x86 code.
        const auto first = nicolai::extract_compressed_unit(voice, idx.records.front());
        std::cout << "unit0_extracted_bytes: " << first.size() << "\n";

        // M3 also promotes the recovered tree construction into a portable
        // primitive. This is still a synthetic codebook until the serialized
        // Acapela codebooks are decoded.
        const std::vector<nicolai::HuffmanCode> codes = {
            {11, {0}}, {22, {1, 0}}, {33, {1, 1}}
        };
        const auto built = nicolai::build_huffman_tree(codes);
        if (!built.valid) {
            std::cerr << "huffman builder failed: " << built.error << "\n";
            return 1;
        }
        std::vector<std::uint16_t> logical_bits = {1, 0};
        std::size_t bit_index = 0;
        const auto dec = nicolai::decode_huffman_word_bits(
            built.nodes, built.root, logical_bits, bit_index, logical_bits.size());
        std::cout << "portable_huffman_builder: " << nicolai::to_string(dec.status)
                  << " symbol=" << dec.symbol << "\n";
        return dec.status == nicolai::HuffmanStatus::Ok && dec.symbol == 22 ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
