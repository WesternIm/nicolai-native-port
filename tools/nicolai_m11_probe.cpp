#include "nicolai/cmp16_decode.hpp"
#include "nicolai/diphone_catalog.hpp"
#include "nicolai/edat.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

static std::vector<std::uint8_t> rd(const char* p) {
    std::ifstream f(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), {}};
}

static std::string bit_string(const std::vector<std::uint16_t>& bits,
                              std::size_t begin,
                              std::size_t count) {
    std::string out;
    const auto end = std::min(bits.size(), begin + count);
    out.reserve(end - begin);
    for (std::size_t i = begin; i < end; ++i) out.push_back(bits[i] ? '1' : '0');
    return out;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m11_probe nicolai16.dat\n";
        return 2;
    }
    const auto bytes = rd(argv[1]);
    const auto layout = nicolai::parse_edat_layout(bytes);
    if (!layout.valid) { std::cerr << "invalid EDAT\n"; return 1; }
    const auto cat = nicolai::parse_nicolai_diphone_catalog(bytes, layout);
    if (!cat.valid) { std::cerr << "catalog failed: " << cat.error << "\n"; return 1; }
    const auto* unit = nicolai::find_diphone(cat, "#", "p");
    if (!unit) { std::cerr << "# -> p not found\n"; return 1; }
    const auto raw = nicolai::extract_diphone_compressed_bytes(bytes, cat, *unit);
    if (raw.empty()) { std::cerr << "empty compressed unit\n"; return 1; }

    const auto geo = nicolai::inspect_cmp16_block_geometry(raw.size());
    const auto first_n = std::min<std::size_t>(nicolai::kCmp16LegacyBlockBytes, raw.size());
    const auto first_bits = nicolai::expand_cmp16_word_bits_msb(raw.data(), first_n);

    std::cout << "Nicolai native-port M11 cmp16 entropy-front-end probe\n\n";
    std::cout << "unit: # -> p\n";
    std::cout << "compressed bytes: " << raw.size() << "\n";
    std::cout << "legacy block size: " << nicolai::kCmp16LegacyBlockBytes << " bytes -> "
              << nicolai::kCmp16LegacyBlockBits << " uint16 bit-words\n";
    std::cout << "geometry if viewed as legacy 32-byte blocks: full=" << geo.full_legacy_blocks
              << " trailing=" << geo.trailing_bytes << "\n\n";

    std::cout << "first 32 compressed bytes:\n  ";
    for (std::size_t i = 0; i < first_n; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<unsigned>(raw[i]) << (i + 1 == first_n ? "" : " ");
    }
    std::cout << std::dec << "\n\n";
    std::cout << "expanded MSB-first bits (8 bits per source byte):\n";
    for (std::size_t i = 0; i < first_n; ++i)
        std::cout << "  byte[" << std::setw(2) << std::setfill(' ') << i << "]  "
                  << bit_string(first_bits, i * 8, 8) << "\n";

    std::cout << "\nresource map recovered from legacy constructor:\n";
    for (const auto& spec : nicolai::cmp16_resource_specs()) {
        const auto role = spec.role;
        std::cout << "  " << std::left << std::setw(11) << nicolai::to_string(role)
                  << " manifest+0x" << std::right << std::hex << std::setw(4) << std::setfill('0')
                  << nicolai::cmp16_manifest_path_offset(role)
                  << " runtime_file_obj+0x" << std::setw(2)
                  << nicolai::cmp16_runtime_file_object_offset(role)
                  << " parser=0x" << std::setw(8) << spec.legacy_parser_va
                  << std::dec << std::setfill(' ') << std::left << "\n";
    }

    std::cout << "\nverified legacy decoder contracts:\n";
    std::cout << "  raw byte expansion table VA: 0x106139F0\n";
    std::cout << "  source bytes per decoder block: 0x20 (32)\n";
    std::cout << "  expanded logical bits per block: 0x100 (256) WORDs\n";
    std::cout << "  huffman tree-walk VA: 0x1010C930\n";
    std::cout << "  first entropy-stage VA: 0x102287A0\n";
    std::cout << "  second entropy-stage VA: 0x10228910\n";
    std::cout << "  frame driver VA: 0x1021B050\n";
    std::cout << "\nM11 boundary: packed ANA bytes -> exact legacy WORD-bit stream is now portable.\n";
    std::cout << "Remaining before real symbols: bind the serialized HuffmanRMS/HuffmanMLT codebooks.\n";
    return 0;
}
