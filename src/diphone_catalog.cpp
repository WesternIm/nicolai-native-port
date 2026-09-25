#include "nicolai/diphone_catalog.hpp"
#include "nicolai/address_space.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <set>

namespace nicolai {
namespace {

std::uint16_t le16(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 2 > b.size()) return 0;
    return static_cast<std::uint16_t>(b[o]) |
           (static_cast<std::uint16_t>(b[o + 1]) << 8);
}

std::int16_t le_i16(const std::vector<std::uint8_t>& b, std::size_t o) {
    return static_cast<std::int16_t>(le16(b, o));
}

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) return 0;
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

std::int32_t le_i32(const std::vector<std::uint8_t>& b, std::size_t o) {
    return static_cast<std::int32_t>(le32(b, o));
}

std::uint32_t magnitude_i32(std::int32_t v) {
    if (v == std::numeric_limits<std::int32_t>::min()) return 0x80000000u;
    return static_cast<std::uint32_t>(v < 0 ? -v : v);
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
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

std::uint64_t next_initialization_target_after(const std::vector<std::uint8_t>& bytes,
                                               const EdatLayout& layout,
                                               std::uint64_t after) {
    const auto range = make_edat_range_descriptor(layout);
    std::uint64_t best = layout.segment0_end;
    for (const auto& ref : scan_serialized_static_refs(bytes, layout)) {
        const auto rr = resolve_static_ref({range}, StaticDuration::Initialization, ref);
        if (!rr.valid) continue;
        if (rr.file_offset > after && rr.file_offset < best) best = rr.file_offset;
    }
    return best;
}

bool starts_with(const std::string& s, std::size_t at, const char* lit) {
    const std::string x(lit);
    return at + x.size() <= s.size() && s.compare(at, x.size(), x) == 0;
}

std::vector<std::string> tokenize_nicolai_phone_stream(const std::string& packed) {
    std::string s;
    s.reserve(packed.size());
    for (const auto c : packed) if (!std::isspace(static_cast<unsigned char>(c))) s.push_back(c);

    std::vector<std::string> out;
    const std::vector<std::string> digraphs = {"sh", "zh", "sc", "ch", "CH", "SC"};
    const std::string vowel_codes = "aAuUiIyoOeE";

    for (std::size_t i = 0; i < s.size();) {
        if (s[i] == '#') {
            out.emplace_back("#");
            ++i;
            continue;
        }
        if (i + 1 < s.size() && vowel_codes.find(s[i]) != std::string::npos &&
            std::isdigit(static_cast<unsigned char>(s[i + 1]))) {
            out.push_back(s.substr(i, 2));
            i += 2;
            continue;
        }
        bool matched = false;
        for (const auto& d : digraphs) {
            if (starts_with(s, i, d.c_str())) {
                out.push_back(d);
                i += d.size();
                matched = true;
                break;
            }
        }
        if (matched) continue;
        if (i + 1 < s.size() && s[i + 1] == '\'') {
            out.push_back(s.substr(i, 2));
            i += 2;
            continue;
        }
        out.push_back(s.substr(i, 1));
        ++i;
    }
    return out;
}

} // namespace

PhoneInventory parse_nicolai_phone_inventory(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t pho_data_file_offset) {

    PhoneInventory out;
    if (pho_data_file_offset + 9 > bytes.size()) {
        out.error = "pho_out_of_bounds";
        return out;
    }
    // Runtime/preloaded .pho representation observed in Nicolai:
    //   u32 reserved (0)
    //   u32 phone_count (66)
    //   NUL-terminated compact symbol stream
    const auto reserved = le32(bytes, static_cast<std::size_t>(pho_data_file_offset));
    out.declared_count = le32(bytes, static_cast<std::size_t>(pho_data_file_offset + 4));
    if (reserved != 0 || out.declared_count == 0 || out.declared_count > 255) {
        out.error = "invalid_pho_header";
        return out;
    }

    const auto begin = static_cast<std::size_t>(pho_data_file_offset + 8);
    std::size_t end = begin;
    while (end < bytes.size() && bytes[end] != 0 && end - begin < 4096) {
        const auto c = bytes[end];
        if (c < 0x20 || c > 0x7e) {
            out.error = "invalid_pho_stream";
            return out;
        }
        ++end;
    }
    if (end == bytes.size() || end == begin) {
        out.error = "missing_pho_stream";
        return out;
    }
    out.packed_stream.assign(reinterpret_cast<const char*>(bytes.data() + begin), end - begin);
    out.phones = tokenize_nicolai_phone_stream(out.packed_stream);
    if (out.phones.size() != out.declared_count) {
        out.error = "phone_count_mismatch";
        return out;
    }
    out.valid = true;
    return out;
}

DiphoneCatalog parse_nicolai_diphone_catalog(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout) {

    DiphoneCatalog out;
    if (!layout.valid) {
        out.error = "invalid_edat_layout";
        return out;
    }

    const auto objects = scan_legacy_file_objects(bytes, layout, StaticDuration::Initialization);
    const auto* pho = find_legacy_file_object(objects, "nbr16aci.dsc.pho");
    const auto* axm = find_legacy_file_object(objects, "nbr16aci.axm");
    const auto* rgl = find_legacy_file_object(objects, "nbr.rgl");
    const auto* seg = find_legacy_file_object(objects, "nbr16aci.seg");
    const auto* ana = find_legacy_file_object(objects, "nbr16aci.ana");
    if (!pho || !axm || !rgl || !seg || !ana) {
        out.error = "nicolai_diphone_assets_missing";
        return out;
    }
    out.pho_object = *pho;
    out.axm_object = *axm;
    out.rgl_object = *rgl;
    out.seg_object = *seg;
    out.ana_object = *ana;

    if (!resolve_data_ref(bytes, layout, *pho, out.pho_data_file_offset) ||
        !resolve_data_ref(bytes, layout, *axm, out.axm_data_file_offset) ||
        !resolve_data_ref(bytes, layout, *rgl, out.rgl_data_file_offset) ||
        !resolve_data_ref(bytes, layout, *seg, out.seg_data_file_offset) ||
        !resolve_data_ref(bytes, layout, *ana, out.ana_data_file_offset)) {
        out.error = "nicolai_diphone_asset_ref_failed";
        return out;
    }
    if (!(out.pho_data_file_offset < out.axm_data_file_offset &&
          out.axm_data_file_offset < out.rgl_data_file_offset &&
          out.rgl_data_file_offset < out.seg_data_file_offset &&
          out.seg_data_file_offset < out.ana_data_file_offset)) {
        out.error = "unexpected_nicolai_asset_order";
        return out;
    }

    out.ana_data_end_file_offset = next_initialization_target_after(bytes, layout, out.ana_data_file_offset);
    if (out.ana_data_end_file_offset <= out.ana_data_file_offset ||
        out.ana_data_end_file_offset > layout.segment0_end) {
        out.error = "ana_payload_end_not_found";
        return out;
    }

    out.phone_inventory = parse_nicolai_phone_inventory(bytes, out.pho_data_file_offset);
    if (!out.phone_inventory.valid) {
        out.error = "phone_inventory_" + out.phone_inventory.error;
        return out;
    }
    out.matrix_side = out.phone_inventory.declared_count;
    out.matrix_entries = static_cast<std::size_t>(out.matrix_side) * out.matrix_side;
    const auto matrix_bytes = out.matrix_entries * sizeof(std::uint32_t);
    if (out.axm_data_file_offset + matrix_bytes > bytes.size()) {
        out.error = "axm_matrix_out_of_bounds";
        return out;
    }

    out.matrix.reserve(out.matrix_entries);
    std::set<std::uint32_t> unique_offsets;
    for (std::size_t i = 0; i < out.matrix_entries; ++i) {
        const auto rel = le32(bytes, static_cast<std::size_t>(out.axm_data_file_offset + i * 4));
        out.matrix.push_back(rel);
        if (rel != 0) unique_offsets.insert(rel);
    }
    out.occupied_entries = unique_offsets.size();
    out.unique_axm_records = unique_offsets.size();
    if (out.occupied_entries == 0) {
        out.error = "empty_axm_matrix";
        return out;
    }

    constexpr std::uint32_t kAxmRecordSize = 0x18;
    const auto expected_axm_size = matrix_bytes + out.occupied_entries * kAxmRecordSize;
    out.axm_serialized_size = out.rgl_data_file_offset - out.axm_data_file_offset;
    if (out.axm_serialized_size != expected_axm_size) {
        out.error = "axm_size_mismatch";
        return out;
    }

    out.seg_serialized_size = out.ana_data_file_offset - out.seg_data_file_offset;
    out.ana_serialized_size = out.ana_data_end_file_offset - out.ana_data_file_offset;
    if (out.seg_serialized_size > std::numeric_limits<std::uint32_t>::max() ||
        out.ana_serialized_size > std::numeric_limits<std::uint32_t>::max()) {
        out.error = "asset_too_large";
        return out;
    }

    out.units.reserve(out.occupied_entries);
    std::set<std::uint32_t> seen_records;
    for (std::size_t cell = 0; cell < out.matrix_entries; ++cell) {
        const auto rel = out.matrix[cell];
        if (rel == 0) continue;
        if (rel < matrix_bytes || (rel - matrix_bytes) % kAxmRecordSize != 0 ||
            rel + kAxmRecordSize > out.axm_serialized_size) {
            out.error = "invalid_axm_record_offset";
            return out;
        }
        if (!seen_records.insert(rel).second) {
            out.error = "duplicate_axm_record_offset";
            return out;
        }

        const auto row = static_cast<std::uint8_t>(cell / out.matrix_side);
        const auto col = static_cast<std::uint8_t>(cell % out.matrix_side);
        const auto ro = static_cast<std::size_t>(out.axm_data_file_offset + rel);
        if (le32(bytes, ro) != 0 || le32(bytes, ro + 4) != 0 ||
            bytes[ro + 8] != 1 || bytes[ro + 9] != 2 ||
            bytes[ro + 10] != row || bytes[ro + 11] != col ||
            le32(bytes, ro + 20) != 0) {
            out.error = "invalid_axm_record";
            return out;
        }

        DiphoneUnit u;
        u.index = out.units.size();
        u.left_index = row;
        u.right_index = col;
        u.left_phone = out.phone_inventory.phones[row];
        u.right_phone = out.phone_inventory.phones[col];
        u.axm_record_relative = rel;
        u.axm_record_file_offset = ro;
        u.seg_offset = le32(bytes, ro + 12);
        u.seg_size = le32(bytes, ro + 16);
        if (u.seg_size < 8 || ((u.seg_size - 8) & 1u) != 0 ||
            static_cast<std::uint64_t>(u.seg_offset) + u.seg_size > out.seg_serialized_size) {
            out.error = "invalid_seg_slice";
            return out;
        }

        const auto so = static_cast<std::size_t>(out.seg_data_file_offset + u.seg_offset);
        u.compressed_start = le_i32(bytes, so);
        u.signed_end = le_i32(bytes, so + 4);
        u.compressed_end = magnitude_i32(u.signed_end);
        if (u.compressed_start < 0 || u.compressed_end <= static_cast<std::uint32_t>(u.compressed_start) ||
            u.compressed_end > out.ana_serialized_size) {
            out.error = "invalid_ana_slice";
            return out;
        }
        for (std::size_t p = so + 8; p < so + u.seg_size; p += 2)
            u.metadata.push_back(le_i16(bytes, p));
        out.units.push_back(std::move(u));
    }

    if (out.units.size() != out.occupied_entries) {
        out.error = "unit_count_mismatch";
        return out;
    }

    // AXM records partition SEG exactly, and the SEG records form one chained
    // coverage of the compressed ANA payload.  These are much stronger
    // invariants than the heuristic boundary search used by M3.
    auto by_seg = out.units;
    std::sort(by_seg.begin(), by_seg.end(), [](const auto& a, const auto& b) {
        return a.seg_offset < b.seg_offset;
    });
    std::uint32_t expected_seg = 0;
    std::uint32_t expected_ana = 0;
    for (const auto& u : by_seg) {
        if (u.seg_offset != expected_seg ||
            static_cast<std::uint32_t>(u.compressed_start) != expected_ana) {
            out.error = "non_contiguous_diphone_chain";
            return out;
        }
        expected_seg = u.seg_offset + u.seg_size;
        expected_ana = u.compressed_end;
    }
    if (expected_seg != out.seg_serialized_size) {
        out.error = "seg_not_fully_covered";
        return out;
    }
    out.ana_indexed_bytes = expected_ana;
    out.ana_trailing_bytes = static_cast<std::uint32_t>(out.ana_serialized_size) - expected_ana;
    if (out.ana_trailing_bytes > 64) {
        out.error = "ana_payload_not_fully_indexed";
        return out;
    }

    out.valid = true;
    return out;
}

const DiphoneUnit* find_diphone(
    const DiphoneCatalog& catalog,
    std::uint8_t left_index,
    std::uint8_t right_index) {
    for (const auto& u : catalog.units)
        if (u.left_index == left_index && u.right_index == right_index) return &u;
    return nullptr;
}

const DiphoneUnit* find_diphone(
    const DiphoneCatalog& catalog,
    const std::string& left_phone,
    const std::string& right_phone) {
    const auto l = lower(left_phone), r = lower(right_phone);
    for (const auto& u : catalog.units)
        if (lower(u.left_phone) == l && lower(u.right_phone) == r) return &u;
    return nullptr;
}

std::vector<std::uint8_t> extract_diphone_compressed_bytes(
    const std::vector<std::uint8_t>& bytes,
    const DiphoneCatalog& catalog,
    const DiphoneUnit& unit) {
    if (!catalog.valid || unit.compressed_start < 0) return {};
    const auto begin = catalog.ana_data_file_offset + static_cast<std::uint32_t>(unit.compressed_start);
    const auto end = catalog.ana_data_file_offset + unit.compressed_end;
    if (begin >= end || end > catalog.ana_data_end_file_offset || end > bytes.size()) return {};
    return std::vector<std::uint8_t>(
        bytes.begin() + static_cast<std::ptrdiff_t>(begin),
        bytes.begin() + static_cast<std::ptrdiff_t>(end));
}

} // namespace nicolai
