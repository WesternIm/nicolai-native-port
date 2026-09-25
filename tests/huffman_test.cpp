#include "nicolai/huffman.hpp"

#include <iostream>
#include <vector>

int main() {
    const std::vector<nicolai::HuffmanCode> codes = {
        {11, {0}}, {22, {1, 0}}, {33, {1, 1}}
    };
    const auto built = nicolai::build_huffman_tree(codes);
    if (!built.valid) {
        std::cerr << "builder: " << built.error << "\n";
        return 1;
    }

    std::vector<std::uint16_t> bits = {1, 0};
    std::size_t pos = 0;
    auto result = nicolai::decode_huffman_word_bits(built.nodes, built.root, bits, pos, bits.size());
    if (result.status != nicolai::HuffmanStatus::Ok || result.symbol != 22 || result.bits_consumed != 2) return 2;

    const std::vector<nicolai::HuffmanCode> bad = {{1, {0}}, {2, {0, 1}}};
    if (nicolai::build_huffman_tree(bad).valid) return 3;

    std::cout << "huffman_test: PASSED\n";
    return 0;
}
