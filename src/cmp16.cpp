#include "nicolai/cmp16.hpp"

#include <cmath>
#include <limits>
#include <sstream>

namespace nicolai {
namespace {

constexpr std::array<Cmp16ResourceSpec, 9> kSpecs{{
    {Cmp16ResourceRole::Qmlt,       0x000, 0x1010AD90},
    {Cmp16ResourceRole::Qnf,        0x200, 0x1010C220},
    {Cmp16ResourceRole::Qrms,       0x400, 0x1010B4E0},
    {Cmp16ResourceRole::HuffmanMlt, 0x600, 0x1010B6B0},
    {Cmp16ResourceRole::HuffmanRms, 0x800, 0x1010B9D0},
    {Cmp16ResourceRole::QvecA,      0xA00, 0x1010AFE0},
    {Cmp16ResourceRole::QvecB,      0xC00, 0x1010B140},
    {Cmp16ResourceRole::Sequence,   0xE00, 0x1010C020},
    {Cmp16ResourceRole::Noise,      0x1000,0x1010BEB0},
}};

std::int16_t low_i16_from_trunc(double value) noexcept {
    if (!std::isfinite(value)) return 0;
    // MSVCRT _ftol used by the legacy code performs an integral conversion.
    // All known table values are expected to fit; keep low-16-bit behavior
    // explicit rather than relying on implementation-defined narrowing.
    const double t = std::trunc(value);
    std::int64_t v = 0;
    if (t >= static_cast<double>(std::numeric_limits<std::int32_t>::max())) {
        v = std::numeric_limits<std::int32_t>::max();
    } else if (t <= static_cast<double>(std::numeric_limits<std::int32_t>::min())) {
        v = std::numeric_limits<std::int32_t>::min();
    } else {
        v = static_cast<std::int64_t>(t);
    }
    const auto lo = static_cast<std::uint16_t>(static_cast<std::uint64_t>(v) & 0xffffu);
    if (lo < 0x8000u) return static_cast<std::int16_t>(lo);
    return static_cast<std::int16_t>(static_cast<std::int32_t>(lo) - 0x10000);
}

template <class T>
bool read_token(std::istringstream& in, T& value) {
    return static_cast<bool>(in >> value);
}

bool no_extra_tokens(std::istringstream& in) {
    std::string extra;
    return !(in >> extra);
}

} // namespace

const char* to_string(Cmp16ResourceRole role) noexcept {
    switch (role) {
        case Cmp16ResourceRole::Qmlt: return "QMLT";
        case Cmp16ResourceRole::Qnf: return "QNF";
        case Cmp16ResourceRole::Qrms: return "QRms";
        case Cmp16ResourceRole::HuffmanMlt: return "HuffmanMLT";
        case Cmp16ResourceRole::HuffmanRms: return "HuffmanRMS";
        case Cmp16ResourceRole::QvecA: return "QVecA";
        case Cmp16ResourceRole::QvecB: return "QVecB";
        case Cmp16ResourceRole::Noise: return "noise";
        case Cmp16ResourceRole::Sequence: return "sequence";
    }
    return "unknown";
}

const std::array<Cmp16ResourceSpec, 9>& cmp16_resource_specs() noexcept {
    return kSpecs;
}

Cmp16Manifest parse_cmp16_manifest(std::string_view text) {
    Cmp16Manifest out;
    std::istringstream in{std::string(text)};

    // Exact fscanf token order in 0x1010B2C0:
    // slots 000,200,400,600,800,A00,C00,1000,E00.
    if (!read_token(in, out.qmlt) ||
        !read_token(in, out.qnf) ||
        !read_token(in, out.qrms) ||
        !read_token(in, out.huffman_mlt) ||
        !read_token(in, out.huffman_rms) ||
        !read_token(in, out.qvec_a) ||
        !read_token(in, out.qvec_b) ||
        !read_token(in, out.noise) ||
        !read_token(in, out.sequence)) {
        out.error = "manifest_missing_resource_path";
        return out;
    }

    for (auto& scalar : out.scalars) {
        int value = 0;
        if (!read_token(in, value) || value < -32768 || value > 32767) {
            out.error = "manifest_invalid_scalar";
            return out;
        }
        scalar = static_cast<std::int16_t>(value);
    }

    if (!no_extra_tokens(in)) {
        out.error = "manifest_trailing_tokens";
        return out;
    }

    out.valid = true;
    return out;
}

std::int16_t legacy_q13(double value) noexcept {
    return low_i16_from_trunc(value * 8192.0);
}

std::int16_t legacy_qrms_value(double value) noexcept {
    return low_i16_from_trunc(4.0 * std::pow(10.0, value * 0.05));
}

QmltTable parse_qmlt(std::string_view text) {
    QmltTable out;
    std::istringstream in{std::string(text)};
    int row_count = 0;
    if (!read_token(in, row_count) || row_count < 0 || row_count > 32767) {
        out.error = "qmlt_invalid_row_count";
        return out;
    }
    out.rows.reserve(static_cast<std::size_t>(row_count));

    for (int row = 0; row < row_count; ++row) {
        int count = 0;
        if (!read_token(in, count) || count < 0 || count > 32) {
            out.error = "qmlt_invalid_centroid_count";
            return out;
        }
        QmltRow r;
        r.centroids.reserve(static_cast<std::size_t>(count));
        r.centroids_q13.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            double x = 0.0;
            if (!read_token(in, x)) {
                out.error = "qmlt_missing_centroid";
                return out;
            }
            r.centroids.push_back(x);
            r.centroids_q13.push_back(legacy_q13(x));
        }
        const int threshold_count = count > 0 ? count - 1 : 0;
        r.thresholds.reserve(static_cast<std::size_t>(threshold_count));
        for (int i = 0; i < threshold_count; ++i) {
            double x = 0.0;
            if (!read_token(in, x)) {
                out.error = "qmlt_missing_threshold";
                return out;
            }
            r.thresholds.push_back(x);
        }
        out.rows.push_back(std::move(r));
    }

    if (!no_extra_tokens(in)) {
        out.error = "qmlt_trailing_tokens";
        return out;
    }
    out.valid = true;
    return out;
}

QrmsTable parse_qrms(std::string_view text) {
    QrmsTable out;
    std::istringstream in{std::string(text)};
    int count = 0;
    if (!read_token(in, count) || count < 0 || count > 32) {
        out.error = "qrms_invalid_count";
        return out;
    }
    out.source_values.reserve(static_cast<std::size_t>(count));
    out.reconstructed.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        double x = 0.0;
        if (!read_token(in, x)) {
            out.error = "qrms_missing_value";
            return out;
        }
        out.source_values.push_back(x);
        out.reconstructed.push_back(legacy_qrms_value(x));
    }
    if (!no_extra_tokens(in)) {
        out.error = "qrms_trailing_tokens";
        return out;
    }
    out.valid = true;
    return out;
}

} // namespace nicolai
