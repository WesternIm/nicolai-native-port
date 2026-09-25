#include "nicolai/cmp16_codebook.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace nicolai;

    // Three prefix-free codes: 0, 10, 11. Parser uses doubles exactly like the
    // legacy MLT reader before truncating to integer arrays.
    const auto mlt = parse_legacy_huffman_mlt(
        "1\n"
        "3 3\n"
        "1.0 2.0 2.0\n"
        "0.0 2.0 3.0\n");
    assert(mlt.valid);
    const auto cm = compile_legacy_huffman_mlt_group(mlt.groups[3]);
    assert(cm.valid);
    assert(cm.symbol_map.size() == 3 && cm.symbol_map[2] == 2);

    const auto b = legacy_huffman_code_bits(5, 0x15); // 10101
    assert((b == std::vector<std::uint8_t>{1,0,1,0,1}));

    std::vector<std::uint16_t> bits{1,1};
    std::size_t pos = 0;
    auto mr = decode_legacy_mapped_huffman(cm, bits, pos, bits.size());
    assert(mr.status == HuffmanStatus::Ok);
    assert(mr.value == 2);
    assert(mr.bits_consumed == 2);

    // RMS parser filters zero-length rows while preserving code/symbol pairing.
    const auto rms = parse_legacy_huffman_rms(
        "1\n"
        "2 4\n"
        "1 0 2 2\n"       // lengths: entry #1 removed
        "0 0 2 3\n"       // code values
        "100 999 200 300\n");
    assert(rms.valid);
    const auto& rg = rms.groups[2];
    assert(rg.bit_lengths.size() == 3);
    assert((rg.symbol_values == std::vector<std::int32_t>{100,200,300}));
    const auto cr = compile_legacy_huffman_rms_group(rg);
    assert(cr.valid);
    bits = {1,0}; pos = 0;
    auto rr = decode_legacy_mapped_huffman(cr,bits,pos,bits.size());
    assert(rr.status == HuffmanStatus::Ok);
    assert(rr.value == 200);

    std::cout << "cmp16_codebook_test: PASSED\n";
    return 0;
}
