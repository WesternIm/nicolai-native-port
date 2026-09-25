#include "nicolai/edat.hpp"

#include <algorithm>
#include <stdexcept>

namespace nicolai {
namespace {

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) throw std::out_of_range("le32 outside EDAT buffer");
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

std::string fixed_ascii(const std::vector<std::uint8_t>& b, std::size_t o, std::size_t max_len) {
    if (o >= b.size()) return {};
    const auto end = std::min(b.size(), o + max_len);
    std::string out;
    for (std::size_t i = o; i < end && b[i] != 0; ++i) {
        const auto c = static_cast<char>(b[i]);
        if (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) > 0x7e) break;
        out.push_back(c);
    }
    return out;
}

std::string fixed_utf16le_ascii(const std::vector<std::uint8_t>& b, std::size_t o, std::size_t max_bytes) {
    if (o >= b.size()) return {};
    const auto end = std::min(b.size(), o + max_bytes);
    std::string out;
    for (std::size_t i = o; i + 1 < end; i += 2) {
        const std::uint16_t ch = static_cast<std::uint16_t>(b[i]) |
                                 (static_cast<std::uint16_t>(b[i + 1]) << 8);
        if (ch == 0) break;
        if (ch >= 0x20 && ch <= 0x7e) out.push_back(static_cast<char>(ch));
        else out.push_back('?');
    }
    return out;
}

std::size_t find_ascii_from(const std::vector<std::uint8_t>& b, const std::string& needle, std::size_t from) {
    if (from >= b.size()) return std::string::npos;
    const auto it = std::search(b.begin() + static_cast<std::ptrdiff_t>(from), b.end(), needle.begin(), needle.end());
    return it == b.end() ? std::string::npos : static_cast<std::size_t>(it - b.begin());
}

bool mode_record_looks_like_nicolai(const std::vector<std::uint8_t>& b, std::size_t s) {
    if (s + kModeRecordSize > b.size()) return false;
    if (le32(b, s + 0xAF0) != 16000u) return false;
    const auto db = fixed_ascii(b, s + 0xB00, 64);
    return db == "nicolai16";
}

ModeRecord parse_mode_record(const std::vector<std::uint8_t>& b, std::size_t s) {
    ModeRecord r;
    r.offset = s;
    r.vendor = fixed_utf16le_ascii(b, s + 0x010, 0x200);
    r.engine_name = fixed_utf16le_ascii(b, s + 0x21C, 0x200);
    r.description = fixed_utf16le_ascii(b, s + 0x438, 0x200);
    r.style = fixed_utf16le_ascii(b, s + 0x646, 0x80);
    r.voice_name = fixed_utf16le_ascii(b, s + 0x6C6, 0x200);
    r.pitch_name = fixed_utf16le_ascii(b, s + 0x8D2, 0x200);
    r.field_adc = le32(b, s + 0xADC);
    r.field_ae0 = le32(b, s + 0xAE0);
    r.field_ae4 = le32(b, s + 0xAE4);
    r.field_ae8 = le32(b, s + 0xAE8);
    r.field_aec = le32(b, s + 0xAEC);
    r.sample_rate = le32(b, s + 0xAF0);
    r.field_af4 = le32(b, s + 0xAF4);
    r.field_af8 = le32(b, s + 0xAF8);
    r.field_afc = le32(b, s + 0xAFC);
    r.database_name = fixed_ascii(b, s + 0xB00, 64);
    return r;
}

} // namespace

const char* to_string(EdatRegion region) noexcept {
    switch (region) {
        case EdatRegion::Header: return "header";
        case EdatRegion::Segment0: return "segment0";
        case EdatRegion::Segment1: return "segment1";
        case EdatRegion::Footer: return "footer";
        default: return "outside";
    }
}

EdatLayout parse_edat_layout(const std::vector<std::uint8_t>& bytes) {
    EdatLayout l;
    if (bytes.size() < kEdatHeaderSize) return l;

    l.segment0_size = le32(bytes, 0x00);
    l.segment1_size = le32(bytes, 0x04);
    l.tag0 = le32(bytes, 0x08);
    l.segment0_offset = le32(bytes, 0x0C);
    l.tag1 = le32(bytes, 0x10);
    l.segment1_offset = le32(bytes, 0x14);
    l.segment0_end = static_cast<std::uint64_t>(l.segment0_offset) + l.segment0_size;
    l.segment1_end = static_cast<std::uint64_t>(l.segment1_offset) + l.segment1_size;
    l.footer_offset = l.segment1_end;
    l.footer_size = l.segment1_end <= bytes.size() ? bytes.size() - l.segment1_end : 0;

    l.valid =
        l.tag0 == kEdatTaggedRefMagic &&
        l.tag1 == kEdatTaggedRefMagic &&
        l.segment0_offset == kEdatHeaderSize &&
        l.segment0_end == l.segment1_offset &&
        l.segment1_end <= bytes.size();
    return l;
}

EdatRegion classify_offset(const EdatLayout& l, std::uint64_t file_size, std::uint64_t o) noexcept {
    if (o < kEdatHeaderSize) return EdatRegion::Header;
    if (o >= l.segment0_offset && o < l.segment0_end) return EdatRegion::Segment0;
    if (o >= l.segment1_offset && o < l.segment1_end) return EdatRegion::Segment1;
    if (o >= l.footer_offset && o < file_size) return EdatRegion::Footer;
    return EdatRegion::Outside;
}

std::vector<TaggedRef> scan_tagged_refs(const std::vector<std::uint8_t>& bytes, const EdatLayout& layout) {
    std::vector<TaggedRef> out;
    if (!layout.valid) return out;
    for (std::size_t o = 0; o + 8 <= bytes.size(); o += 4) {
        if (le32(bytes, o) != kEdatTaggedRefMagic) continue;
        const auto target = le32(bytes, o + 4);
        TaggedRef r;
        r.source_offset = o;
        r.target_offset = target;
        r.source_region = classify_offset(layout, bytes.size(), o);
        r.target_region = classify_offset(layout, bytes.size(), target);
        out.push_back(r);
    }
    return out;
}

std::vector<ModeRecord> scan_nicolai_mode_records(const std::vector<std::uint8_t>& bytes, const EdatLayout& layout) {
    std::vector<ModeRecord> out;
    if (!layout.valid || layout.segment1_end > bytes.size()) return out;

    // ReadStad in the original x86 engine operates on 0xB50-byte records.
    // Search Segment 1 on that natural record grid and accept only records whose
    // database name and numeric sample rate agree with Nicolai 16 kHz.
    for (std::size_t s = layout.segment1_offset; s + kModeRecordSize <= layout.segment1_end; ++s) {
        if (!mode_record_looks_like_nicolai(bytes, s)) continue;
        out.push_back(parse_mode_record(bytes, s));
        s += kModeRecordSize - 1;
    }
    return out;
}

std::vector<OffsetString> find_nicolai_resource_paths(const std::vector<std::uint8_t>& bytes) {
    const std::vector<std::string> needles = {
        "\\nicolai\\", "nicolai\\", "nicolai/"
    };
    std::vector<OffsetString> out;
    std::size_t pos = 0;
    while (pos < bytes.size()) {
        std::size_t best = std::string::npos;
        for (const auto& needle : needles) {
            const auto p = find_ascii_from(bytes, needle, pos);
            if (p != std::string::npos && (best == std::string::npos || p < best)) best = p;
        }
        if (best == std::string::npos) break;

        std::size_t begin = best;
        while (begin > 0 && bytes[begin - 1] >= 0x20 && bytes[begin - 1] <= 0x7e) --begin;
        std::size_t end = best;
        while (end < bytes.size() && bytes[end] >= 0x20 && bytes[end] <= 0x7e && end - begin < 1024) ++end;
        std::string value(reinterpret_cast<const char*>(bytes.data() + begin), end - begin);
        if (std::find_if(out.begin(), out.end(), [&](const OffsetString& x) { return x.offset == begin; }) == out.end()) {
            out.push_back({begin, std::move(value)});
        }
        pos = std::max(end, best + 1);
    }
    return out;
}

} // namespace nicolai
