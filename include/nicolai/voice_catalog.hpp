#pragma once

#include "nicolai/address_space.hpp"
#include "nicolai/edat.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct LegacyFileObject {
    bool valid = false;
    StaticDuration duration = StaticDuration::Execution;
    std::uint64_t file_offset = 0;
    std::uint32_t logical_offset = 0;
    std::string path;
    std::uint32_t field_104 = 0;
    std::uint32_t field_108 = 0;
    std::uint32_t data_logical = 0;
    std::uint32_t state = 0;
    bool has_link_ref = false;
    std::uint32_t link_logical = 0;
};

struct VoiceRecord {
    bool occupied = false;
    std::size_t index = 0;
    std::uint64_t file_offset = 0;
    std::uint32_t name_logical = 0;
    std::string name;

    // Raw fields are kept deliberately unnamed until their legacy semantics
    // are proven from the x86 parser.
    std::uint32_t raw_08 = 0;
    std::uint32_t raw_0c = 0;
    std::uint32_t raw_10 = 0;
    std::uint32_t raw_21c = 0;
    std::uint32_t raw_230 = 0;
    std::uint32_t raw_238 = 0;
    std::uint32_t raw_23c = 0;
    std::uint32_t raw_64c = 0;

    std::string exception_path;
    std::string acoustic_dsc_path;
    std::string modeinfo_path;

    bool identity_symbol_map = false;
};

struct VoiceCatalog {
    bool valid = false;
    LegacyFileObject source{};
    std::uint64_t records_file_offset = 0;
    std::uint32_t records_logical = 0;
    std::size_t capacity = 64;
    std::size_t occupied = 0;
    std::vector<VoiceRecord> voices;
};

struct ChannelCatalogSummary {
    bool valid = false;
    LegacyFileObject source{};
    std::uint64_t data_file_offset = 0;
    std::uint32_t data_logical = 0;
    std::uint32_t slot_capacity = 0;
    std::uint32_t raw_04 = 0;
    std::vector<std::uint32_t> slot_modes;
    std::size_t nonzero_slot_modes = 0;
    std::size_t serialized_size = 0;
};

struct AcousticDescriptor {
    bool valid = false;
    LegacyFileObject source{};
    std::uint64_t data_file_offset = 0;
    std::uint32_t data_logical = 0;

    std::uint32_t raw_00 = 0;
    std::uint32_t raw_04 = 0;
    std::uint32_t raw_08 = 0;
    std::uint32_t raw_0c = 0;
    std::uint32_t raw_10 = 0;
    std::uint32_t raw_14 = 0;
    std::uint32_t raw_18 = 0;
    std::uint32_t sample_rate = 0;
    std::uint32_t raw_20 = 0;
    std::uint32_t raw_24 = 0;
    std::uint32_t raw_28 = 0;
    std::uint32_t raw_2c = 0;
    std::uint32_t raw_30 = 0;
    std::uint32_t raw_34 = 0;
    std::uint32_t raw_38 = 0;
    std::uint32_t raw_3c = 0;

    std::string rgl_path;
    std::string axm_path;
    std::string seg_path;
    std::string ana_path;
};

std::vector<LegacyFileObject> scan_legacy_file_objects(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    StaticDuration duration);

const LegacyFileObject* find_legacy_file_object(
    const std::vector<LegacyFileObject>& objects,
    const std::string& path_or_suffix);

VoiceCatalog parse_voice_catalog(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& voix_object);

const VoiceRecord* find_voice(const VoiceCatalog& catalog, const std::string& name);

ChannelCatalogSummary parse_channel_catalog_summary(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& canaux_object);

AcousticDescriptor parse_acoustic_descriptor(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const LegacyFileObject& dsc_object);

} // namespace nicolai
