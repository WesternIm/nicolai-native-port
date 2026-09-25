#include "nicolai/voice_catalog.hpp"

#include <algorithm>
#include <cctype>
#include <limits>

namespace nicolai {
namespace {

std::uint16_t le16(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 2 > b.size()) return 0;
    return static_cast<std::uint16_t>(b[o]) |
           (static_cast<std::uint16_t>(b[o + 1]) << 8);
}

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) return 0;
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

bool printable(std::uint8_t c) { return c >= 0x20 && c <= 0x7e; }

std::string fixed_ascii(const std::vector<std::uint8_t>& b,
                        std::size_t o,
                        std::size_t width) {
    if (o >= b.size()) return {};
    const auto end = std::min<std::size_t>(b.size(), o + width);
    std::string s;
    for (std::size_t i = o; i < end; ++i) {
        const auto c = b[i];
        if (c == 0) break;
        if (!printable(c)) return {};
        s.push_back(static_cast<char>(c));
    }
    return s;
}

std::string cstr_at(const std::vector<std::uint8_t>& b,
                    std::uint64_t o,
                    std::size_t max_len = 256) {
    if (o >= b.size()) return {};
    return fixed_ascii(b, static_cast<std::size_t>(o), max_len);
}

bool suffix_match(const std::string& full, const std::string& query) {
    const auto a = lower(full), q = lower(query);
    if (a == q) return true;
    return a.size() >= q.size() && a.compare(a.size() - q.size(), q.size(), q) == 0;
}

bool region_bounds(const EdatLayout& l,
                   StaticDuration d,
                   std::uint64_t& begin,
                   std::uint64_t& end) {
    if (!l.valid) return false;
    if (d == StaticDuration::Initialization) {
        begin = l.segment0_offset;
        end = l.segment0_end;
    } else {
        begin = l.segment1_offset;
        end = l.segment1_end;
    }
    return begin < end;
}

std::uint32_t logical_from_file(const EdatLayout& l,
                                StaticDuration d,
                                std::uint64_t file_off) {
    const auto base = d == StaticDuration::Initialization ? l.segment0_offset : l.segment1_offset;
    if (file_off < base || file_off - base > std::numeric_limits<std::uint32_t>::max()) return 0;
    return static_cast<std::uint32_t>(file_off - base);
}

bool resolve_data_ref(const std::vector<std::uint8_t>& bytes,
                      const EdatLayout& layout,
                      const LegacyFileObject& obj,
                      std::uint64_t& file_off) {
    const auto rr = resolve_static_ref({make_edat_range_descriptor(layout)},
                                       obj.duration,
                                       kEdatTaggedRefMagic,
                                       obj.data_logical);
    if (!rr.valid || rr.file_offset >= bytes.size()) return false;
    file_off = rr.file_offset;
    return true;
}

bool contains_ci(const std::string& s, const std::string& needle) {
    return lower(s).find(lower(needle)) != std::string::npos;
}

} // namespace

std::vector<LegacyFileObject> scan_legacy_file_objects(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    StaticDuration duration) {
    std::vector<LegacyFileObject> out;
    std::uint64_t begin = 0, end = 0;
    if (!region_bounds(layout, duration, begin, end)) return out;

    // Runtime/preloaded file objects observed in SpeechCube 5.1:
    //   char path[0x104]
    //   u32 field_104
    //   u32 field_108
    //   tagged_ref data       (+0x10c)
    //   u32 state             (+0x114)
    //   optional tagged_ref   (+0x118)
    //
    // Search on 4-byte boundaries and require the scalar/ref tail to keep
    // registry strings from being misidentified as file objects.
    for (std::uint64_t o = begin; o + 0x120 <= end && o + 0x120 <= bytes.size(); o += 4) {
        const auto path = fixed_ascii(bytes, static_cast<std::size_t>(o), 0x104);
        if (path.size() < 5 || path.find('.') == std::string::npos ||
            (path.find('\\') == std::string::npos && path.find('/') == std::string::npos)) continue;
        const auto f104 = le32(bytes, static_cast<std::size_t>(o + 0x104));
        const auto f108 = le32(bytes, static_cast<std::size_t>(o + 0x108));
        if (f104 > 4 || f108 > 4) continue;
        if (le32(bytes, static_cast<std::size_t>(o + 0x10c)) != kEdatTaggedRefMagic) continue;
        const auto data = le32(bytes, static_cast<std::size_t>(o + 0x110));
        const auto state = le32(bytes, static_cast<std::size_t>(o + 0x114));
        if (state > 4) continue;
        const auto rr = resolve_static_ref({make_edat_range_descriptor(layout)}, duration,
                                           kEdatTaggedRefMagic, data);
        if (!rr.valid || rr.file_offset >= bytes.size()) continue;

        LegacyFileObject x;
        x.valid = true;
        x.duration = duration;
        x.file_offset = o;
        x.logical_offset = logical_from_file(layout, duration, o);
        x.path = path;
        x.field_104 = f104;
        x.field_108 = f108;
        x.data_logical = data;
        x.state = state;
        if (le32(bytes, static_cast<std::size_t>(o + 0x118)) == kEdatTaggedRefMagic) {
            x.has_link_ref = true;
            x.link_logical = le32(bytes, static_cast<std::size_t>(o + 0x11c));
        }
        out.push_back(std::move(x));
    }
    return out;
}

const LegacyFileObject* find_legacy_file_object(
    const std::vector<LegacyFileObject>& objects,
    const std::string& path_or_suffix) {
    for (const auto& o : objects) if (suffix_match(o.path, path_or_suffix)) return &o;
    return nullptr;
}

VoiceCatalog parse_voice_catalog(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& voix_object) {
    VoiceCatalog out;
    out.source = voix_object;
    if (!layout.valid || !voix_object.valid || voix_object.duration != StaticDuration::Execution) return out;
    std::uint64_t base = 0;
    if (!resolve_data_ref(bytes, layout, voix_object, base)) return out;
    constexpr std::size_t kCapacity = 64;
    constexpr std::size_t kStride = 0x884;
    if (base + kCapacity * kStride > bytes.size()) return out;
    out.records_file_offset = base;
    out.records_logical = voix_object.data_logical;
    out.capacity = kCapacity;
    out.voices.reserve(kCapacity);

    const auto ranges = std::vector<StaticRangeDescriptor>{make_edat_range_descriptor(layout)};
    for (std::size_t i = 0; i < kCapacity; ++i) {
        const auto o = static_cast<std::size_t>(base + i * kStride);
        VoiceRecord v;
        v.index = i;
        v.file_offset = o;
        const auto tag = le32(bytes, o);
        const auto name_logical = le32(bytes, o + 4);
        if (tag == 0 && name_logical == 0) {
            out.voices.push_back(std::move(v));
            continue;
        }
        if (tag != kEdatTaggedRefMagic) return {};
        const auto nr = resolve_static_ref(ranges, StaticDuration::Initialization,
                                           tag, name_logical);
        if (!nr.valid) return {};
        v.name = cstr_at(bytes, nr.file_offset, 64);
        if (v.name.empty()) return {};
        v.occupied = true;
        v.name_logical = name_logical;
        v.raw_08 = le32(bytes, o + 0x08);
        v.raw_0c = le32(bytes, o + 0x0c);
        v.raw_10 = le32(bytes, o + 0x10);
        v.raw_21c = le32(bytes, o + 0x21c);
        v.raw_230 = le32(bytes, o + 0x230);
        v.raw_238 = le32(bytes, o + 0x238);
        v.raw_23c = le32(bytes, o + 0x23c);
        v.raw_64c = le32(bytes, o + 0x64c);
        v.exception_path = fixed_ascii(bytes, o + 0x118, 0x104);
        v.acoustic_dsc_path = fixed_ascii(bytes, o + 0x340, 0x104);
        v.modeinfo_path = fixed_ascii(bytes, o + 0x444, 0x200);

        bool identity = true;
        for (std::size_t s = 0; s < 255; ++s) {
            if (le16(bytes, o + 0x684 + s * 2) != s) { identity = false; break; }
        }
        v.identity_symbol_map = identity;
        ++out.occupied;
        out.voices.push_back(std::move(v));
    }
    out.valid = out.occupied > 0;
    return out;
}

const VoiceRecord* find_voice(const VoiceCatalog& catalog, const std::string& name) {
    const auto q = lower(name);
    for (const auto& v : catalog.voices)
        if (v.occupied && lower(v.name) == q) return &v;
    return nullptr;
}

ChannelCatalogSummary parse_channel_catalog_summary(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& canaux_object) {
    ChannelCatalogSummary out;
    out.source = canaux_object;
    if (!layout.valid || !canaux_object.valid || canaux_object.duration != StaticDuration::Execution) return out;
    std::uint64_t base = 0;
    if (!resolve_data_ref(bytes, layout, canaux_object, base)) return out;
    // x86 parser callback 0x10011a40 requests a 0x45f0-byte static object.
    constexpr std::size_t kSerializedSize = 0x45f0;
    if (base + kSerializedSize > bytes.size()) return out;
    out.data_file_offset = base;
    out.data_logical = canaux_object.data_logical;
    out.serialized_size = kSerializedSize;
    out.slot_capacity = le32(bytes, static_cast<std::size_t>(base));
    out.raw_04 = le32(bytes, static_cast<std::size_t>(base + 4));
    if (out.slot_capacity == 0 || out.slot_capacity > 256 || base + 8 + out.slot_capacity * 4 > bytes.size()) return out;
    out.slot_modes.reserve(out.slot_capacity);
    for (std::size_t i = 0; i < out.slot_capacity; ++i) {
        const auto x = le32(bytes, static_cast<std::size_t>(base + 8 + i * 4));
        out.slot_modes.push_back(x);
        if (x != 0) ++out.nonzero_slot_modes;
    }
    out.valid = true;
    return out;
}

AcousticDescriptor parse_acoustic_descriptor(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& dsc_object) {
    AcousticDescriptor out;
    out.source = dsc_object;
    if (!layout.valid || !dsc_object.valid || dsc_object.duration != StaticDuration::Initialization) return out;
    std::uint64_t base = 0;
    if (!resolve_data_ref(bytes, layout, dsc_object, base)) return out;
    if (base + 0x144 > bytes.size()) return out;
    out.data_file_offset = base;
    out.data_logical = dsc_object.data_logical;
    auto at = [&](std::size_t rel) { return le32(bytes, static_cast<std::size_t>(base + rel)); };
    out.raw_00 = at(0x00); out.raw_04 = at(0x04); out.raw_08 = at(0x08); out.raw_0c = at(0x0c);
    out.raw_10 = at(0x10); out.raw_14 = at(0x14); out.raw_18 = at(0x18); out.sample_rate = at(0x1c);
    out.raw_20 = at(0x20); out.raw_24 = at(0x24); out.raw_28 = at(0x28); out.raw_2c = at(0x2c);
    out.raw_30 = at(0x30); out.raw_34 = at(0x34); out.raw_38 = at(0x38); out.raw_3c = at(0x3c);
    out.rgl_path = fixed_ascii(bytes, static_cast<std::size_t>(base + 0x40), 0x104);

    // These resources are stored later in the same initialized voice object.
    // Find them within a conservative window rather than claiming unproven
    // fixed offsets for every SpeechCube voice family.
    const auto search_end = std::min<std::uint64_t>(layout.segment0_end, base + 0x30000);
    for (std::uint64_t o = base + 0x40; o < search_end;) {
        const auto s = cstr_at(bytes, o, 0x104);
        if (!s.empty() && (contains_ci(s, "\\nicolai\\") || contains_ci(s, "nbr16aci"))) {
            if (contains_ci(s, ".axm")) out.axm_path = s;
            else if (contains_ci(s, ".seg")) out.seg_path = s;
            else if (contains_ci(s, ".ana")) out.ana_path = s;
            o += std::max<std::size_t>(1, s.size() + 1);
        } else ++o;
        if (!out.axm_path.empty() && !out.seg_path.empty() && !out.ana_path.empty()) break;
    }
    out.valid = out.sample_rate > 0 && !out.rgl_path.empty();
    return out;
}

} // namespace nicolai
