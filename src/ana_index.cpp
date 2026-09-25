#include "nicolai/ana_index.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace nicolai {
namespace {

std::int32_t le_i32(const std::vector<std::uint8_t>& b, std::size_t o) {
    const auto u = static_cast<std::uint32_t>(b[o]) |
                   (static_cast<std::uint32_t>(b[o + 1]) << 8) |
                   (static_cast<std::uint32_t>(b[o + 2]) << 16) |
                   (static_cast<std::uint32_t>(b[o + 3]) << 24);
    return static_cast<std::int32_t>(u);
}

std::int16_t le_i16(const std::vector<std::uint8_t>& b, std::size_t o) {
    const auto u = static_cast<std::uint16_t>(b[o]) |
                   (static_cast<std::uint16_t>(b[o + 1]) << 8);
    return static_cast<std::int16_t>(u);
}

std::uint32_t magnitude_i32(std::int32_t v) {
    // INT_MIN cannot be negated in a signed 32-bit type.
    if (v == std::numeric_limits<std::int32_t>::min()) return 0x80000000u;
    return static_cast<std::uint32_t>(v < 0 ? -v : v);
}

std::size_t find_next_boundary(
    const std::vector<std::uint8_t>& data,
    std::size_t from,
    std::size_t max_scan,
    std::int32_t expected_start) {

    if (from + 4 > data.size()) return data.size();
    const auto limit = std::min(data.size() - 4, from + max_scan);

    // All recovered Nicolai records are 16-bit aligned.  Restricting the scan
    // to that grid avoids accidental matches inside neighboring int32 values.
    for (std::size_t o = from; o <= limit; o += 2) {
        if (le_i32(data, o) == expected_start) return o;
    }
    return data.size();
}

} // namespace

AnaUnitIndex parse_ana_unit_index(
    const std::vector<std::uint8_t>& analysis_data,
    std::uint32_t compressed_payload_size,
    std::size_t max_record_scan) {

    AnaUnitIndex out;
    out.payload_bytes = compressed_payload_size;
    if (analysis_data.size() < 8) {
        out.error = "analysis_too_small";
        return out;
    }
    if (max_record_scan < 8) {
        out.error = "record_scan_too_small";
        return out;
    }

    std::size_t pos = 0;
    std::uint32_t expected_start = 0;

    while (pos + 8 <= analysis_data.size()) {
        const auto start_i = le_i32(analysis_data, pos);
        const auto signed_end = le_i32(analysis_data, pos + 4);
        if (start_i < 0 || static_cast<std::uint32_t>(start_i) != expected_start) {
            out.error = "broken_boundary_chain";
            return out;
        }

        const auto start = static_cast<std::uint32_t>(start_i);
        const auto end = magnitude_i32(signed_end);
        if (end <= start || end > compressed_payload_size) {
            out.error = "invalid_compressed_range";
            return out;
        }

        const auto next = find_next_boundary(
            analysis_data, pos + 8, max_record_scan, static_cast<std::int32_t>(end));
        const bool terminal = next == analysis_data.size();
        const auto record_end = terminal ? analysis_data.size() : next;
        if (record_end < pos + 8 || ((record_end - (pos + 8)) & 1u) != 0) {
            out.error = "invalid_record_alignment";
            return out;
        }

        AnaUnitRecord rec;
        rec.index = out.records.size();
        rec.table_offset = pos;
        rec.compressed_start = start_i;
        rec.signed_end = signed_end;
        rec.compressed_end = end;
        rec.record_size = record_end - pos;
        rec.terminal = terminal;
        for (std::size_t o = pos + 8; o + 2 <= record_end; o += 2) {
            rec.metadata.push_back(le_i16(analysis_data, o));
        }
        out.records.push_back(std::move(rec));

        expected_start = end;
        if (terminal) {
            pos = record_end;
            break;
        }
        pos = next;
    }

    if (out.records.empty()) {
        out.error = "no_ana_records";
        return out;
    }

    out.covered_bytes = out.records.back().compressed_end;
    out.trailing_payload_bytes = compressed_payload_size - out.covered_bytes;
    out.trailing_analysis_bytes = analysis_data.size() -
        (out.records.back().table_offset + out.records.back().record_size);

    // Nicolai leaves two bytes after the last indexed compressed range.  Keep
    // the parser generic but reject a clearly incomplete chain.
    if (out.covered_bytes + 64u < compressed_payload_size) {
        out.error = "incomplete_payload_coverage";
        return out;
    }

    out.valid = true;
    return out;
}

std::vector<std::uint8_t> extract_compressed_unit(
    const std::vector<std::uint8_t>& compressed_payload,
    const AnaUnitRecord& unit) {

    if (unit.compressed_start < 0) return {};
    const auto begin = static_cast<std::size_t>(unit.compressed_start);
    const auto end = static_cast<std::size_t>(unit.compressed_end);
    if (begin >= end || end > compressed_payload.size()) return {};
    return std::vector<std::uint8_t>(
        compressed_payload.begin() + static_cast<std::ptrdiff_t>(begin),
        compressed_payload.begin() + static_cast<std::ptrdiff_t>(end));
}

} // namespace nicolai
