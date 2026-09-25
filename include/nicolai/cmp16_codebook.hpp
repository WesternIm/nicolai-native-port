#pragma once

#include "nicolai/huffman.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace nicolai {

// Recovered from mtsyc32.dll 5.1:
//   HuffmanMLT parser 0x1010B6B0 -> 0x144-byte parsed object
//   HuffmanRMS parser 0x1010B9D0 -> 0x204-byte parsed object
// Both formats expose at most sixteen explicitly indexed groups.
constexpr std::size_t kCmp16HuffmanGroupSlots = 16;
constexpr std::size_t kLegacyHuffmanMltParsedBytes = 0x144;
constexpr std::size_t kLegacyHuffmanRmsParsedBytes = 0x204;

struct LegacyHuffmanMltGroup {
    bool present = false;
    std::vector<std::int32_t> bit_lengths;
    std::vector<std::int32_t> code_values;
};

struct LegacyHuffmanMltSource {
    std::int32_t group_records = 0;
    std::array<LegacyHuffmanMltGroup, kCmp16HuffmanGroupSlots> groups{};
    bool valid = false;
    std::string error;
};

// Text-file equivalent of parser 0x1010B6B0.
// Layout recovered from the fscanf loop:
//   group_record_count
//   repeated group_record_count times:
//      group_index  entry_count
//      entry_count numeric bit lengths
//      entry_count numeric code values
// The legacy reader accepts doubles for both arrays and truncates them to int32.
LegacyHuffmanMltSource parse_legacy_huffman_mlt(std::string_view text);

struct LegacyHuffmanRmsGroup {
    bool present = false;
    std::vector<std::int32_t> bit_lengths;
    std::vector<std::int32_t> code_values;
    std::vector<std::int32_t> symbol_values;
};

struct LegacyHuffmanRmsSource {
    std::int32_t group_records = 0;
    std::array<LegacyHuffmanRmsGroup, kCmp16HuffmanGroupSlots> groups{};
    bool valid = false;
    std::string error;
};

// Text-file equivalent of parser 0x1010B9D0.
// Each group contains entry_count float lengths, entry_count float codes, then
// entry_count integer symbol values. The legacy parser removes every entry for
// which the first (length) value converts to zero, keeping the three arrays in
// lock-step. This function reproduces that filtering.
LegacyHuffmanRmsSource parse_legacy_huffman_rms(std::string_view text);

// Exact logical bit order used by legacy helper 0x1010C780: fixed-width,
// MSB-first binary representation of code_value.
std::vector<std::uint8_t> legacy_huffman_code_bits(
    std::int32_t bit_length,
    std::int32_t code_value);

struct CompiledLegacyHuffmanGroup {
    bool valid = false;
    std::string error;
    std::vector<HuffmanNode> nodes;
    std::int32_t root = -1;

    // MLT: identity [0..N-1]. RMS: third source array after zero-length filtering.
    // The x86 tree itself returns an index; RMS maps that index through this array.
    std::vector<std::int32_t> symbol_map;
};

// Portable equivalents of legacy 0x10058C00 / 0x10058CE0 + 0x1010C7B0.
CompiledLegacyHuffmanGroup compile_legacy_huffman_mlt_group(
    const LegacyHuffmanMltGroup& group);
CompiledLegacyHuffmanGroup compile_legacy_huffman_rms_group(
    const LegacyHuffmanRmsGroup& group);

struct LegacyMappedHuffmanResult {
    HuffmanStatus status = HuffmanStatus::InvalidTree;
    std::int32_t value = -1;
    std::size_t bits_consumed = 0;
};

// Decode one leaf and perform the post-tree mapping used by RMS. It also works
// for MLT because its map is identity.
LegacyMappedHuffmanResult decode_legacy_mapped_huffman(
    const CompiledLegacyHuffmanGroup& group,
    const std::vector<std::uint16_t>& bit_words,
    std::size_t& bit_index,
    std::size_t max_bits);

} // namespace nicolai
