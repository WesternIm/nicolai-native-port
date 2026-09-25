#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace nicolai {

enum class Cmp16ResourceRole {
    Qmlt,
    Qnf,
    Qrms,
    HuffmanMlt,
    HuffmanRms,
    QvecA,
    QvecB,
    Noise,
    Sequence,
};

const char* to_string(Cmp16ResourceRole role) noexcept;

struct Cmp16ResourceSpec {
    Cmp16ResourceRole role;
    std::uint32_t descriptor_slot;
    std::uint32_t legacy_parser_va;
};

// Recovered from mtsyc32.dll / cmp16sbi/init.c.
const std::array<Cmp16ResourceSpec, 9>& cmp16_resource_specs() noexcept;

struct Cmp16Manifest {
    std::string qmlt;
    std::string qnf;
    std::string qrms;
    std::string huffman_mlt;
    std::string huffman_rms;
    std::string qvec_a;
    std::string qvec_b;
    std::string noise;
    std::string sequence;

    // Ten %%hd tokens follow the nine %%s tokens in rsrc.dsc.  Their semantic
    // names are not assigned until use-sites are fully reconstructed.
    std::array<std::int16_t, 10> scalars{};

    bool valid = false;
    std::string error;
};

Cmp16Manifest parse_cmp16_manifest(std::string_view text);

struct QmltRow {
    std::vector<double> centroids;
    std::vector<std::int16_t> centroids_q13;
    std::vector<double> thresholds;
};

struct QmltTable {
    std::vector<QmltRow> rows;
    bool valid = false;
    std::string error;
};

// Reimplementation of the text-reader at legacy VA 0x1010AD90.
// Format: row_count; for each row: centroid_count, centroid_count doubles,
// then centroid_count-1 threshold doubles.  The decoder stores centroids as
// fixed-point Q13 (value * 8192) and merely consumes the thresholds.
QmltTable parse_qmlt(std::string_view text);

struct QrmsTable {
    std::vector<double> source_values;
    std::vector<std::int16_t> reconstructed;
    bool valid = false;
    std::string error;
};

// Reimplementation of the text-reader at legacy VA 0x1010B4E0.
// Format: count followed by count doubles.  Each x becomes
// trunc(4 * pow(10, x * 0.05)).
QrmsTable parse_qrms(std::string_view text);

std::int16_t legacy_q13(double value) noexcept;
std::int16_t legacy_qrms_value(double value) noexcept;

} // namespace nicolai
