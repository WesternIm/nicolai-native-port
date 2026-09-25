#include "nicolai/static_files.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace nicolai {
namespace {

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) throw std::out_of_range("le32 outside buffer");
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

bool path_char(std::uint8_t c) {
    return c >= 0x20 && c <= 0x7e;
}

bool plausible_path(const std::string& s) {
    if (s.size() < 4 || s.size() > 0x103) return false;
    const bool has_sep = s.find('\\') != std::string::npos || s.find('/') != std::string::npos;
    const bool has_dot = s.find('.') != std::string::npos;
    return has_sep && has_dot;
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

} // namespace

std::vector<StaticFileRecord> scan_static_file_records(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout) {
    std::vector<StaticFileRecord> out;
    if (!layout.valid) return out;

    // Static/preloaded file nodes are observed in Segment 1 as:
    //   tagged_ref(primary) + char path[0x104] + u32 field_104 + u32 field_108 + ...
    // Pointers in the serialized tail may themselves expand to 8-byte tagged refs,
    // so only the two scalar fields are read at fixed offsets here.
    for (std::size_t o = static_cast<std::size_t>(layout.segment1_offset);
         o + 8 + 0x10c <= layout.segment1_end && o + 8 + 0x10c <= bytes.size();
         o += 4) {
        if (le32(bytes, o) != kEdatTaggedRefMagic) continue;
        const auto target = le32(bytes, o + 4);
        if (classify_offset(layout, bytes.size(), target) != EdatRegion::Segment0) continue;

        const auto path_off = o + 8;
        std::string path;
        for (std::size_t i = path_off; i < std::min<std::size_t>(bytes.size(), path_off + 0x104); ++i) {
            const auto c = bytes[i];
            if (c == 0) break;
            if (!path_char(c)) { path.clear(); break; }
            path.push_back(static_cast<char>(c));
        }
        if (!plausible_path(path)) continue;

        StaticFileRecord r;
        r.tagged_ref_offset = o;
        r.primary_ref_offset = target;
        r.path_offset = path_off;
        r.path = std::move(path);
        r.field_104 = le32(bytes, path_off + 0x104);
        r.field_108 = le32(bytes, path_off + 0x108);
        if (path_off + 0x114 <= bytes.size() && le32(bytes, path_off + 0x10c) == kEdatTaggedRefMagic) {
            r.has_field_10c_ref = true;
            r.field_10c_target = le32(bytes, path_off + 0x110);
        }
        out.push_back(std::move(r));
    }
    return out;
}

const StaticFileRecord* find_static_file_record(
    const std::vector<StaticFileRecord>& records,
    const std::string& suffix) {
    const auto want = lower(suffix);
    for (const auto& r : records) {
        const auto p = lower(r.path);
        if (p.size() >= want.size() && p.compare(p.size() - want.size(), want.size(), want) == 0) {
            return &r;
        }
    }
    return nullptr;
}


StaticRecordLockstepRun longest_static_record_lockstep_run(
    const std::vector<StaticFileRecord>& records,
    std::uint64_t serialized_stride,
    std::uint32_t primary_stride) {
    StaticRecordLockstepRun best;
    if (records.empty()) return best;
    for (std::size_t begin = 0; begin < records.size(); ++begin) {
        std::size_t count = 1;
        while (begin + count < records.size()) {
            const auto& a = records[begin + count - 1];
            const auto& b = records[begin + count];
            if (b.tagged_ref_offset < a.tagged_ref_offset ||
                b.primary_ref_offset < a.primary_ref_offset) break;
            const auto ds = b.tagged_ref_offset - a.tagged_ref_offset;
            const auto dt = b.primary_ref_offset - a.primary_ref_offset;
            if (ds != serialized_stride || dt != primary_stride) break;
            ++count;
        }
        if (count > best.count) {
            best.begin_index = begin;
            best.count = count;
            best.serialized_stride = serialized_stride;
            best.primary_stride = primary_stride;
        }
    }
    return best;
}

RepeatingMarkerRun detect_repeating_marker_run(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t begin,
    std::uint64_t end,
    std::size_t marker_size,
    std::size_t stride,
    std::size_t search_prefix) {
    RepeatingMarkerRun best;
    if (marker_size == 0 || stride < marker_size || begin >= end || end > bytes.size()) return best;
    const auto limit = std::min<std::uint64_t>(end, begin + search_prefix);
    for (std::uint64_t first = begin; first + marker_size <= limit; ++first) {
        std::vector<std::uint8_t> marker(bytes.begin() + static_cast<std::ptrdiff_t>(first),
                                         bytes.begin() + static_cast<std::ptrdiff_t>(first + marker_size));
        if (std::all_of(marker.begin(), marker.end(), [](std::uint8_t x) { return x == 0; })) continue;
        std::size_t count = 1;
        auto p = first + stride;
        while (p + marker_size <= end &&
               std::equal(marker.begin(), marker.end(), bytes.begin() + static_cast<std::ptrdiff_t>(p))) {
            ++count;
            p += stride;
        }
        const auto nz = std::count_if(marker.begin(), marker.end(), [](std::uint8_t x) { return x != 0; });
        const auto best_nz = std::count_if(best.marker.begin(), best.marker.end(), [](std::uint8_t x) { return x != 0; });
        if (count > best.count || (count == best.count && nz > best_nz)) {
            best.first_offset = first;
            best.count = count;
            best.stride = stride;
            best.marker = std::move(marker);
        }
    }
    return best;
}

} // namespace nicolai
