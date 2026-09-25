#include "nicolai/static_materializer.hpp"

#include <algorithm>
#include <cstring>
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

void put32(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
    p[2] = static_cast<std::uint8_t>(v >> 16);
    p[3] = static_cast<std::uint8_t>(v >> 24);
}

bool plausible_path(const std::uint8_t* p, std::size_t n, std::string& out) {
    out.clear();
    for (std::size_t i = 0; i < n; ++i) {
        const auto c = p[i];
        if (c == 0) break;
        if (c < 0x20 || c > 0x7e) return false;
        out.push_back(static_cast<char>(c));
    }
    if (out.size() < 4) return false;
    return (out.find('\\') != std::string::npos || out.find('/') != std::string::npos) &&
           out.find('.') != std::string::npos;
}

} // namespace

std::vector<StaticFilePatch> scan_static_file_patches(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout) {
    std::vector<StaticFilePatch> out;
    if (!layout.valid || layout.segment1_end > bytes.size()) return out;

    // Search on dword boundaries.  Unlike the earlier M5 heuristic, M6 accepts
    // a record only if the *complete* 0x120-byte path patch shape is present.
    for (std::size_t o = static_cast<std::size_t>(layout.segment1_offset);
         o + kStaticFilePatchSerializedSize <= layout.segment1_end;
         o += 4) {
        if (le32(bytes, o) != kEdatTaggedRefMagic) continue;
        const auto dst = le32(bytes, o + 4);
        if (classify_offset(layout, bytes.size(), dst) != EdatRegion::Segment0) continue;

        std::string path;
        if (!plausible_path(bytes.data() + o + 8, kStaticFilePathSize, path)) continue;

        // Runtime +0x10c is a 32-bit pointer.  EDAT expands it to magic+offset.
        if (le32(bytes, o + 0x114) != kEdatTaggedRefMagic) continue;
        const auto object_ref = le32(bytes, o + 0x118);
        if (object_ref != 0 && classify_offset(layout, bytes.size(), object_ref) == EdatRegion::Outside) {
            continue;
        }

        StaticFilePatch p;
        p.serialized_offset = o;
        p.destination_offset = dst;
        p.path = std::move(path);
        p.field_104 = le32(bytes, o + 0x10c);
        p.field_108 = le32(bytes, o + 0x110);
        p.object_ref_offset = object_ref;
        p.state = le32(bytes, o + 0x11c);
        out.push_back(std::move(p));
    }
    return out;
}

MaterializedStaticFileNode materialize_static_file_patch_offsets(
    const StaticFilePatch& patch) {
    MaterializedStaticFileNode out;
    out.destination_offset = patch.destination_offset;
    out.bytes.fill(0);

    const auto n = std::min<std::size_t>(patch.path.size(), kStaticFilePathSize - 1);
    std::memcpy(out.bytes.data(), patch.path.data(), n);
    put32(out.bytes.data() + 0x104, patch.field_104);
    put32(out.bytes.data() + 0x108, patch.field_108);
    put32(out.bytes.data() + 0x10c, patch.object_ref_offset);
    put32(out.bytes.data() + 0x110, patch.state);
    return out;
}

bool apply_static_file_patches_offset_view(
    std::vector<std::uint8_t>& image,
    const EdatLayout& layout,
    const std::vector<StaticFilePatch>& patches) {
    if (!layout.valid) return false;
    for (const auto& p : patches) {
        const auto begin = static_cast<std::uint64_t>(p.destination_offset);
        const auto end = begin + kStaticFileNodeRuntimeSize;
        if (begin < layout.segment0_offset || end > layout.segment0_end || end > image.size()) return false;
        const auto m = materialize_static_file_patch_offsets(p);
        std::copy(m.bytes.begin(), m.bytes.end(), image.begin() + static_cast<std::ptrdiff_t>(begin));
    }
    return true;
}

} // namespace nicolai
