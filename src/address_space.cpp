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
std::size_t di(StaticDuration d) noexcept { return static_cast<std::size_t>(d); }
}

const char* to_string(StaticDuration d) noexcept {
    return d == StaticDuration::Initialization ? "initialization" : "execution";
}

StaticRangeDescriptor make_edat_range_descriptor(const EdatLayout& l) noexcept {
    StaticRangeDescriptor r;
    r.tag[0] = l.tag0;
    r.tag[1] = l.tag1;
    r.base_file_offset[0] = l.segment0_offset;
    r.base_file_offset[1] = l.segment1_offset;
    r.max_offset[0] = l.segment0_size;
    r.max_offset[1] = l.segment1_size;
    return r;
}

LegacyStaticRef decode_static_ref_at(const std::vector<std::uint8_t>& bytes,
                                     const EdatLayout& layout,
                                     std::uint64_t source_offset) {
    LegacyStaticRef r;
    r.source_offset = source_offset;
    r.source_region = classify_offset(layout, bytes.size(), source_offset);
    if (!layout.valid || source_offset + 8 > bytes.size()) return r;
    r.tag = le32(bytes, static_cast<std::size_t>(source_offset));
    r.logical_offset = le32(bytes, static_cast<std::size_t>(source_offset + 4));
    // A zero tag is a special passthrough case in the x86 SycStadPntr routine,
    // not a serialized EDAT reference.  Keep the portable ref strict.
    r.valid = (r.tag == kEdatTaggedRefMagic);
    return r;
}

ResolvedStaticRef resolve_static_ref(const std::vector<StaticRangeDescriptor>& ranges,
                                     StaticDuration duration,
                                     std::uint32_t tag,
                                     std::uint32_t logical_offset) noexcept {
    ResolvedStaticRef out;
    out.duration = duration;
    out.tag = tag;
    out.logical_offset = logical_offset;
    if (tag == 0) return out;
    const auto d = di(duration);
    for (std::size_t i = 0; i < ranges.size(); ++i) {
        const auto& r = ranges[i];
        if (r.tag[d] != tag) continue;
        if (logical_offset > r.max_offset[d]) return out;
        out.valid = true;
        out.range_index = i;
        out.file_offset = r.base_file_offset[d] + logical_offset;
        return out;
    }
    return out;
}

ResolvedStaticRef resolve_static_ref(const std::vector<StaticRangeDescriptor>& ranges,
                                     StaticDuration duration,
                                     const LegacyStaticRef& ref) noexcept {
    if (!ref.valid) return {};
    return resolve_static_ref(ranges, duration, ref.tag, ref.logical_offset);
}

ResolvedStaticRef resolve_edat_static_ref_at(const std::vector<std::uint8_t>& bytes,
                                             const EdatLayout& layout,
                                             std::uint64_t source_offset,
                                             StaticDuration duration) {
    const auto ref = decode_static_ref_at(bytes, layout, source_offset);
    if (!ref.valid) return {};
    return resolve_static_ref({make_edat_range_descriptor(layout)}, duration, ref);
}

EncodedStaticRef encode_static_address(const std::vector<StaticRangeDescriptor>& ranges,
                                       StaticDuration duration,
                                       std::uint64_t file_offset) noexcept {
    EncodedStaticRef out;
    out.duration = duration;
    const auto d = di(duration);
    for (std::size_t i = 0; i < ranges.size(); ++i) {
        const auto& r = ranges[i];
        const auto base = r.base_file_offset[d];
        if (file_offset < base) continue;
        const auto delta = file_offset - base;
        if (delta > r.max_offset[d]) continue;
        out.valid = true;
        out.range_index = i;
        out.tag = r.tag[d];
        out.logical_offset = static_cast<std::uint32_t>(delta);
        return out;
    }
    return out;
}

std::vector<LegacyStaticRef> scan_serialized_static_refs(const std::vector<std::uint8_t>& bytes,
                                                         const EdatLayout& layout) {
    std::vector<LegacyStaticRef> out;
    if (!layout.valid) return out;
    // The 0x18-byte EDAT header also contains tag+file-offset pairs, but those
    // are segment descriptors, not SycStadPntr references.  Scan only payload.
    for (std::uint64_t o = layout.segment0_offset; o + 8 <= layout.segment1_end; o += 4) {
        const auto r = decode_static_ref_at(bytes, layout, o);
        if (r.valid) out.push_back(r);
    }
    return out;
}

} // namespace nicolai
