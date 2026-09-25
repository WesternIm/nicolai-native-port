#include "nicolai/cmp16_decode.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    using namespace nicolai;

    const std::vector<std::uint8_t> bytes = {0x00, 0x01, 0x55, 0x80, 0xA5, 0xFF};
    const auto bits = expand_cmp16_word_bits_msb(bytes);
    assert(bits.size() == bytes.size() * 8);

    const std::vector<std::uint16_t> expected = {
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,
        0,1,0,1,0,1,0,1,
        1,0,0,0,0,0,0,0,
        1,0,1,0,0,1,0,1,
        1,1,1,1,1,1,1,1,
    };
    assert(bits == expected);

    const auto g = inspect_cmp16_block_geometry(1759);
    assert(g.full_legacy_blocks == 54);
    assert(g.trailing_bytes == 31);
    assert(g.expanded_word_bits == 14072);

    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::Qmlt) == 0x000);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::Qnf) == 0x200);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::Qrms) == 0x400);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::HuffmanMlt) == 0x600);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::HuffmanRms) == 0x800);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::QvecA) == 0xA00);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::QvecB) == 0xC00);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::Sequence) == 0xE00);
    assert(cmp16_manifest_path_offset(Cmp16ResourceRole::Noise) == 0x1000);

    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::Sequence) == 0x68);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::Qmlt) == 0x6C);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::Qrms) == 0x70);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::HuffmanRms) == 0x74);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::HuffmanMlt) == 0x78);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::Noise) == 0x7C);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::QvecA) == 0x80);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::QvecB) == 0x84);
    assert(cmp16_runtime_file_object_offset(Cmp16ResourceRole::Qnf) == 0x88);

    const std::vector<HuffmanCode> codes = {
        {11, {0}}, {22, {1,0}}, {33, {1,1}}
    };
    const auto tree = build_huffman_tree(codes);
    assert(tree.valid);
    LegacyEntropyCursor cursor;
    cursor.word_bits = expand_cmp16_word_bits_msb(std::vector<std::uint8_t>{0x80}); // 10...
    auto r = decode_cmp16_huffman(tree.nodes, tree.root, cursor, cursor.word_bits.size());
    assert(r.status == HuffmanStatus::Ok);
    assert(r.symbol == 22);
    assert(cursor.bit_index == 2);

    static_assert(Cmp16LegacyRuntimeLayout::source_block_bytes == 0x4E);
    static_assert(kCmp16LegacyBlockBytes == 32);
    static_assert(kCmp16LegacyBlockBits == 256);

    std::cout << "cmp16_decode_test: PASSED\n";
    return 0;
}
