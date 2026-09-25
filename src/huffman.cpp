#include "nicolai/huffman.hpp"

namespace nicolai {

const char* to_string(HuffmanStatus status) noexcept {
    switch (status) {
        case HuffmanStatus::Ok: return "ok";
        case HuffmanStatus::BitstreamExhausted: return "bitstream_exhausted";
        default: return "invalid_tree";
    }
}

HuffmanResult decode_huffman_word_bits(
    const std::vector<HuffmanNode>& nodes,
    std::int32_t root,
    const std::vector<std::uint16_t>& bit_words,
    std::size_t& bit_index,
    std::size_t max_bits) {

    HuffmanResult result;
    const auto start = bit_index;
    auto node_index = root;

    // A malformed tree could cycle forever. A valid binary Huffman tree can be
    // traversed to a leaf in fewer than node_count edges.
    for (std::size_t guard = 0; guard <= nodes.size(); ++guard) {
        if (node_index < 0 || static_cast<std::size_t>(node_index) >= nodes.size()) {
            result.status = HuffmanStatus::InvalidTree;
            result.bits_consumed = bit_index - start;
            return result;
        }

        const auto& node = nodes[static_cast<std::size_t>(node_index)];
        if (node.leaf) {
            result.status = HuffmanStatus::Ok;
            result.symbol = node.symbol;
            result.bits_consumed = bit_index - start;
            return result;
        }

        if (bit_index >= max_bits || bit_index >= bit_words.size()) {
            result.status = HuffmanStatus::BitstreamExhausted;
            result.bits_consumed = bit_index - start;
            return result;
        }

        const auto bit = bit_words[bit_index++];
        node_index = bit == 0 ? node.zero : node.one;
    }

    result.status = HuffmanStatus::InvalidTree;
    result.bits_consumed = bit_index - start;
    return result;
}


HuffmanBuildResult build_huffman_tree(const std::vector<HuffmanCode>& codes) {
    HuffmanBuildResult out;
    out.nodes.push_back(HuffmanNode{});
    out.root = 0;
    if (codes.empty()) {
        out.error = "empty_codebook";
        return out;
    }

    for (const auto& code : codes) {
        if (code.bits.empty()) {
            out.error = "empty_code";
            return out;
        }
        std::int32_t node = out.root;
        for (std::size_t i = 0; i < code.bits.size(); ++i) {
            const auto bit = code.bits[i];
            if (bit > 1) {
                out.error = "invalid_bit";
                return out;
            }
            if (out.nodes[static_cast<std::size_t>(node)].leaf) {
                out.error = "prefix_collision";
                return out;
            }
            const bool last = i + 1 == code.bits.size();
            auto next = bit == 0
                ? out.nodes[static_cast<std::size_t>(node)].zero
                : out.nodes[static_cast<std::size_t>(node)].one;
            if (next < 0) {
                next = static_cast<std::int32_t>(out.nodes.size());
                if (bit == 0) out.nodes[static_cast<std::size_t>(node)].zero = next;
                else out.nodes[static_cast<std::size_t>(node)].one = next;
                out.nodes.push_back(HuffmanNode{});
            }
            node = next;
            if (last) {
                auto& leaf = out.nodes[static_cast<std::size_t>(node)];
                if (leaf.leaf || leaf.zero >= 0 || leaf.one >= 0) {
                    out.error = "duplicate_or_prefix_collision";
                    return out;
                }
                leaf.leaf = true;
                leaf.symbol = code.symbol;
            }
        }
    }

    out.valid = true;
    return out;
}

} // namespace nicolai
