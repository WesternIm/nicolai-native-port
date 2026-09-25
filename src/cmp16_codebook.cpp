#include "nicolai/cmp16_codebook.hpp"

#include <cmath>
#include <limits>
#include <sstream>

namespace nicolai {
namespace {

std::int32_t legacy_trunc(double x) {
    if (!std::isfinite(x)) return 0;
    if (x > static_cast<double>(std::numeric_limits<std::int32_t>::max()))
        return std::numeric_limits<std::int32_t>::max();
    if (x < static_cast<double>(std::numeric_limits<std::int32_t>::min()))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(x); // truncates toward zero, like legacy ftol path here
}

template<class T>
bool read_one(std::istringstream& in, T& x) {
    in >> x;
    return static_cast<bool>(in);
}

CompiledLegacyHuffmanGroup compile_group(
    const std::vector<std::int32_t>& lengths,
    const std::vector<std::int32_t>& codes,
    std::vector<std::int32_t> map) {
    CompiledLegacyHuffmanGroup out;
    if (lengths.empty() || lengths.size() != codes.size() || lengths.size() != map.size()) {
        out.error = "invalid_group_arrays";
        return out;
    }
    if (lengths.size() > static_cast<std::size_t>(std::numeric_limits<std::int16_t>::max())) {
        out.error = "too_many_symbols";
        return out;
    }

    std::vector<HuffmanCode> hc;
    hc.reserve(lengths.size());
    for (std::size_t i = 0; i < lengths.size(); ++i) {
        auto bits = legacy_huffman_code_bits(lengths[i], codes[i]);
        if (bits.empty()) {
            out.error = "invalid_legacy_code";
            return out;
        }
        hc.push_back(HuffmanCode{static_cast<std::int16_t>(i), std::move(bits)});
    }
    auto tree = build_huffman_tree(hc);
    if (!tree.valid) {
        out.error = tree.error;
        return out;
    }
    out.nodes = std::move(tree.nodes);
    out.root = tree.root;
    out.symbol_map = std::move(map);
    out.valid = true;
    return out;
}

} // namespace

LegacyHuffmanMltSource parse_legacy_huffman_mlt(std::string_view text) {
    LegacyHuffmanMltSource out;
    std::istringstream in{std::string(text)};
    int records = 0;
    if (!read_one(in, records) || records < 0 || records > static_cast<int>(kCmp16HuffmanGroupSlots)) {
        out.error = "invalid_group_record_count";
        return out;
    }
    out.group_records = records;
    std::array<bool, kCmp16HuffmanGroupSlots> seen{};
    for (int r = 0; r < records; ++r) {
        int index = -1, count = -1;
        if (!read_one(in, index) || !read_one(in, count) ||
            index < 0 || index >= static_cast<int>(kCmp16HuffmanGroupSlots) || count < 0) {
            out.error = "invalid_group_header";
            return out;
        }
        if (seen[static_cast<std::size_t>(index)]) {
            out.error = "duplicate_group_index";
            return out;
        }
        seen[static_cast<std::size_t>(index)] = true;
        auto& g = out.groups[static_cast<std::size_t>(index)];
        g.present = true;
        g.bit_lengths.resize(static_cast<std::size_t>(count));
        g.code_values.resize(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            double x = 0;
            if (!read_one(in, x)) { out.error = "truncated_lengths"; return out; }
            g.bit_lengths[static_cast<std::size_t>(i)] = legacy_trunc(x);
        }
        for (int i = 0; i < count; ++i) {
            double x = 0;
            if (!read_one(in, x)) { out.error = "truncated_codes"; return out; }
            g.code_values[static_cast<std::size_t>(i)] = legacy_trunc(x);
        }
    }
    out.valid = true;
    return out;
}

LegacyHuffmanRmsSource parse_legacy_huffman_rms(std::string_view text) {
    LegacyHuffmanRmsSource out;
    std::istringstream in{std::string(text)};
    int records = 0;
    if (!read_one(in, records) || records < 0 || records > static_cast<int>(kCmp16HuffmanGroupSlots)) {
        out.error = "invalid_group_record_count";
        return out;
    }
    out.group_records = records;
    std::array<bool, kCmp16HuffmanGroupSlots> seen{};
    for (int r = 0; r < records; ++r) {
        int index = -1, count = -1;
        if (!read_one(in, index) || !read_one(in, count) ||
            index < 0 || index >= static_cast<int>(kCmp16HuffmanGroupSlots) || count < 0) {
            out.error = "invalid_group_header";
            return out;
        }
        if (seen[static_cast<std::size_t>(index)]) {
            out.error = "duplicate_group_index";
            return out;
        }
        seen[static_cast<std::size_t>(index)] = true;

        std::vector<std::int32_t> lens(static_cast<std::size_t>(count));
        std::vector<std::int32_t> codes(static_cast<std::size_t>(count));
        std::vector<std::int32_t> symbols(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            float x = 0;
            if (!read_one(in, x)) { out.error = "truncated_lengths"; return out; }
            lens[static_cast<std::size_t>(i)] = legacy_trunc(x);
        }
        for (int i = 0; i < count; ++i) {
            float x = 0;
            if (!read_one(in, x)) { out.error = "truncated_codes"; return out; }
            codes[static_cast<std::size_t>(i)] = legacy_trunc(x);
        }
        for (int i = 0; i < count; ++i) {
            int x = 0;
            if (!read_one(in, x)) { out.error = "truncated_symbols"; return out; }
            symbols[static_cast<std::size_t>(i)] = x;
        }

        auto& g = out.groups[static_cast<std::size_t>(index)];
        g.present = true;
        for (std::size_t i = 0; i < lens.size(); ++i) {
            if (lens[i] == 0) continue; // exact filter recovered at 0x1010BB7A..BDxx
            g.bit_lengths.push_back(lens[i]);
            g.code_values.push_back(codes[i]);
            g.symbol_values.push_back(symbols[i]);
        }
    }
    out.valid = true;
    return out;
}

std::vector<std::uint8_t> legacy_huffman_code_bits(
    std::int32_t bit_length,
    std::int32_t code_value) {
    if (bit_length <= 0 || bit_length > 31 || code_value < 0) return {};
    const auto u = static_cast<std::uint32_t>(code_value);
    if (bit_length < 31 && u >= (1u << bit_length)) return {};
    std::vector<std::uint8_t> bits;
    bits.reserve(static_cast<std::size_t>(bit_length));
    for (int shift = bit_length - 1; shift >= 0; --shift)
        bits.push_back(static_cast<std::uint8_t>((u >> shift) & 1u));
    return bits;
}

CompiledLegacyHuffmanGroup compile_legacy_huffman_mlt_group(
    const LegacyHuffmanMltGroup& group) {
    if (!group.present) return {};
    std::vector<std::int32_t> map(group.bit_lengths.size());
    for (std::size_t i = 0; i < map.size(); ++i) map[i] = static_cast<std::int32_t>(i);
    return compile_group(group.bit_lengths, group.code_values, std::move(map));
}

CompiledLegacyHuffmanGroup compile_legacy_huffman_rms_group(
    const LegacyHuffmanRmsGroup& group) {
    if (!group.present) return {};
    return compile_group(group.bit_lengths, group.code_values, group.symbol_values);
}

LegacyMappedHuffmanResult decode_legacy_mapped_huffman(
    const CompiledLegacyHuffmanGroup& group,
    const std::vector<std::uint16_t>& bit_words,
    std::size_t& bit_index,
    std::size_t max_bits) {
    LegacyMappedHuffmanResult out;
    if (!group.valid) return out;
    const auto r = decode_huffman_word_bits(group.nodes, group.root, bit_words, bit_index, max_bits);
    out.status = r.status;
    out.bits_consumed = r.bits_consumed;
    if (r.status != HuffmanStatus::Ok) return out;
    if (r.symbol < 0 || static_cast<std::size_t>(r.symbol) >= group.symbol_map.size()) {
        out.status = HuffmanStatus::InvalidTree;
        return out;
    }
    out.value = group.symbol_map[static_cast<std::size_t>(r.symbol)];
    return out;
}

} // namespace nicolai
