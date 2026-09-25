#include "nicolai/address_space.hpp"
#include <stdexcept>

namespace nicolai {
namespace {
std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) throw std::out_of_range("le32 outside EDAT buffer");
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o+1]) << 8) |
           (static_cast<std::uint32_t>(b[o+2]) << 16) |
           (static_cast<std::uint32_t>(b[o+3]) << 24);
}
std::uint64_t begin_of(const EdatLayout& l, EdatRegion r) noexcept {
    switch (r) {
        case EdatRegion::Header: return 0;
        case EdatRegion::Segment0: return l.segment0_offset;
        case EdatRegion::Segment1: return l.segment1_offset;
        case EdatRegion::Footer: return l.footer_offset;
        default: return 0;
    }
}
} // namespace

EdatAddress resolve_edat_offset(const EdatLayout& layout,
                                std::uint64_t file_size,
                                std::uint64_t file_offset) noexcept {
    EdatAddress a;
    if (!layout.valid || file_offset >= file_size) return a;
    a.region = classify_offset(layout, file_size, file_offset);
    if (a.region == EdatRegion::Outside) return a;
    a.valid = true;
    a.file_offset = file_offset;
    a.region_begin = begin_of(layout, a.region);
    a.relative_offset = file_offset - a.region_begin;
    return a;
}

ResolvedTaggedRef resolve_tagged_ref_at(const std::vector<std::uint8_t>& bytes,
                                        const EdatLayout& layout,
                                        std::uint64_t source_offset) {
    ResolvedTaggedRef r;
    r.source_offset = source_offset;
    r.source_region = classify_offset(layout, bytes.size(), source_offset);
    if (!layout.valid || source_offset + 8 > bytes.size()) return r;
    r.magic = le32(bytes, static_cast<std::size_t>(source_offset));
    if (r.magic != kEdatTaggedRefMagic) return r;
    r.target_offset = le32(bytes, static_cast<std::size_t>(source_offset + 4));
    r.target = resolve_edat_offset(layout, bytes.size(), r.target_offset);
    r.valid = r.target.valid;
    return r;
}

std::vector<ResolvedTaggedRef> scan_resolved_tagged_refs(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout) {
    std::vector<ResolvedTaggedRef> out;
    if (!layout.valid) return out;
    for (std::uint64_t o = 0; o + 8 <= bytes.size(); o += 4) {
        const auto r = resolve_tagged_ref_at(bytes, layout, o);
        if (r.magic == kEdatTaggedRefMagic) out.push_back(r);
    }
    return out;
}
} // namespace nicolai
