#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

namespace nicolai {

// Logical, pointer-width-independent representation of the tree traversed by the
// legacy x86 decoder. It deliberately does NOT mirror the 32-bit in-memory node
// layout from mtsyc32.dll.
struct HuffmanNode {
    bool leaf = false;
    std::int16_t symbol = 0;
    std::int32_t zero = -1;
    std::int32_t one = -1;
};


struct HuffmanCode {
    std::int16_t symbol = 0;
    std::vector<std::uint8_t> bits; // each item must be 0 or 1
};

struct HuffmanBuildResult {
    bool valid = false;
    std::string error;
    std::vector<HuffmanNode> nodes;
    std::int32_t root = -1;
};

// Architecture-neutral equivalent of the legacy Mat2Arbre-style tree builder.
// It intentionally accepts already-decoded bit codes; parsing Acapela's
// serialized codebook remains a separate codec-framing step.
HuffmanBuildResult build_huffman_tree(const std::vector<HuffmanCode>& codes);

enum class HuffmanStatus {
    Ok,
    BitstreamExhausted,
    InvalidTree
};

struct HuffmanResult {
    HuffmanStatus status = HuffmanStatus::InvalidTree;
    std::int16_t symbol = -1;
    std::size_t bits_consumed = 0;
};

// Portable equivalent of the small legacy tree-walk primitive identified in
// mtsyc32.dll around RVA 0x10C930. The legacy routine consumes one 16-bit word
// per logical bit: zero follows the zero child, any non-zero word follows one.
HuffmanResult decode_huffman_word_bits(
    const std::vector<HuffmanNode>& nodes,
    std::int32_t root,
    const std::vector<std::uint16_t>& bit_words,
    std::size_t& bit_index,
    std::size_t max_bits);

const char* to_string(HuffmanStatus status) noexcept;

} // namespace nicolai
