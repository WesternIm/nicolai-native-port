#include "nicolai/cmp16_decode.hpp"

#include <stdexcept>

namespace nicolai {

std::vector<std::uint16_t> expand_cmp16_word_bits_msb(
    const std::uint8_t* data,
    std::size_t size) {
    std::vector<std::uint16_t> out;
    if (!data || size == 0) return out;
    out.reserve(size * 8);
    for (std::size_t i = 0; i < size; ++i) {
        const auto byte = data[i];
        for (int bit = 7; bit >= 0; --bit)
            out.push_back(static_cast<std::uint16_t>((byte >> bit) & 1u));
    }
    return out;
}

Cmp16BlockGeometry inspect_cmp16_block_geometry(std::size_t source_bytes) noexcept {
    Cmp16BlockGeometry out;
    out.source_bytes = source_bytes;
    out.full_legacy_blocks = source_bytes / kCmp16LegacyBlockBytes;
    out.trailing_bytes = source_bytes % kCmp16LegacyBlockBytes;
    out.expanded_word_bits = source_bytes * 8;
    return out;
}

std::size_t cmp16_manifest_path_offset(Cmp16ResourceRole role) noexcept {
    for (const auto& spec : cmp16_resource_specs())
        if (spec.role == role) return spec.descriptor_slot;
    return 0;
}

std::size_t cmp16_runtime_file_object_offset(Cmp16ResourceRole role) noexcept {
    // The constructor deliberately loads slots in a non-linear order.
    switch (role) {
        case Cmp16ResourceRole::Sequence: return 0x68;
        case Cmp16ResourceRole::Qmlt: return 0x6C;
        case Cmp16ResourceRole::Qrms: return 0x70;
        case Cmp16ResourceRole::HuffmanRms: return 0x74;
        case Cmp16ResourceRole::HuffmanMlt: return 0x78;
        case Cmp16ResourceRole::Noise: return 0x7C;
        case Cmp16ResourceRole::QvecA: return 0x80;
        case Cmp16ResourceRole::QvecB: return 0x84;
        case Cmp16ResourceRole::Qnf: return 0x88;
    }
    return 0;
}

HuffmanResult decode_cmp16_huffman(
    const std::vector<HuffmanNode>& nodes,
    std::int32_t root,
    LegacyEntropyCursor& cursor,
    std::size_t max_bits) {
    return decode_huffman_word_bits(
        nodes, root, cursor.word_bits, cursor.bit_index, max_bits);
}

} // namespace nicolai
