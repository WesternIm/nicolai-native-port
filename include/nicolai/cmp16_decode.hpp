#pragma once

#include "nicolai/huffman.hpp"
#include "nicolai/cmp16.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace nicolai {

// The legacy decoder expands exactly 0x20 source bytes into 0x100 uint16_t
// logical bits before entering the Huffman stages.  This is recovered from
// mtsyc32.dll around 0x1021B0EF..0x1021B1C0 and 0x10058B09.
constexpr std::size_t kCmp16LegacyBlockBytes = 0x20;
constexpr std::size_t kCmp16LegacyBlockBits = kCmp16LegacyBlockBytes * 8;

// Equivalent to the 256-entry lookup table at legacy VA 0x106139F0.
// Each byte becomes eight uint16_t values, MSB first, each exactly 0 or 1.
std::vector<std::uint16_t> expand_cmp16_word_bits_msb(
    const std::uint8_t* data,
    std::size_t size);

inline std::vector<std::uint16_t> expand_cmp16_word_bits_msb(
    const std::vector<std::uint8_t>& data) {
    return expand_cmp16_word_bits_msb(data.data(), data.size());
}

struct Cmp16BlockGeometry {
    std::size_t source_bytes = 0;
    std::size_t full_legacy_blocks = 0;
    std::size_t trailing_bytes = 0;
    std::size_t expanded_word_bits = 0;
};

Cmp16BlockGeometry inspect_cmp16_block_geometry(std::size_t source_bytes) noexcept;

// Resource ordering is declared in cmp16.hpp.  M11 adds the link from each
// manifest slot to the runtime file-object field used by the actual decoder.
constexpr std::size_t kCmp16ResourceRoleCount = 9;
constexpr std::size_t kCmp16ManifestPathStride = 0x200;
constexpr std::size_t kCmp16ManifestSerializedBytes = 0x1220;

std::size_t cmp16_manifest_path_offset(Cmp16ResourceRole role) noexcept;

// Runtime file-object fields in the legacy 0x8C-byte decoder descriptor.
// The file objects are parsed into native tables later; keeping this map in one
// place prevents future milestones from rediscovering the same offsets.
std::size_t cmp16_runtime_file_object_offset(Cmp16ResourceRole role) noexcept;

struct Cmp16LegacyRuntimeLayout {
    static constexpr std::size_t descriptor_bytes = 0x8C;
    static constexpr std::size_t expanded_bits_ptr = 0x40;
    static constexpr std::size_t derived_window_count = 0x44;
    static constexpr std::size_t rms_count = 0x48;
    static constexpr std::size_t mlt_count = 0x4A;
    static constexpr std::size_t vector_count = 0x4C;
    static constexpr std::size_t source_block_bytes = 0x4E;
    static constexpr std::size_t huffman_mlt_runtime = 0x50;
    static constexpr std::size_t huffman_rms_runtime = 0x54;
    static constexpr std::size_t max_qmlt_index = 0x58;
    static constexpr std::size_t scratch_5c = 0x5C;
    static constexpr std::size_t quarter_window_count = 0x60;
    static constexpr std::size_t scratch_256_words = 0x64;
};

// Small portable cursor matching the contract used by legacy huffman_dec.c:
// the stream is uint16 words containing 0/1 and the cursor is advanced in
// logical bits rather than packed bytes.
struct LegacyEntropyCursor {
    std::vector<std::uint16_t> word_bits;
    std::size_t bit_index = 0;

    std::size_t remaining() const noexcept {
        return bit_index <= word_bits.size() ? word_bits.size() - bit_index : 0;
    }
};

HuffmanResult decode_cmp16_huffman(
    const std::vector<HuffmanNode>& nodes,
    std::int32_t root,
    LegacyEntropyCursor& cursor,
    std::size_t max_bits);

} // namespace nicolai
