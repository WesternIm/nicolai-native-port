#pragma once

#include "nicolai/edat.hpp"
#include "nicolai/voice_catalog.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct PhoneInventory {
    bool valid = false;
    std::string error;
    std::uint32_t declared_count = 0;
    std::string packed_stream;
    std::vector<std::string> phones;
};

struct DiphoneUnit {
    std::size_t index = 0;
    std::uint8_t left_index = 0;
    std::uint8_t right_index = 0;
    std::string left_phone;
    std::string right_phone;

    std::uint32_t axm_record_relative = 0;
    std::uint64_t axm_record_file_offset = 0;

    std::uint32_t seg_offset = 0;
    std::uint32_t seg_size = 0;
    std::int32_t compressed_start = 0;
    std::int32_t signed_end = 0;
    std::uint32_t compressed_end = 0;
    std::vector<std::int16_t> metadata;
};

struct DiphoneCatalog {
    bool valid = false;
    std::string error;

    LegacyFileObject pho_object{};
    LegacyFileObject axm_object{};
    LegacyFileObject rgl_object{};
    LegacyFileObject seg_object{};
    LegacyFileObject ana_object{};

    PhoneInventory phone_inventory{};

    std::uint64_t pho_data_file_offset = 0;
    std::uint64_t axm_data_file_offset = 0;
    std::uint64_t rgl_data_file_offset = 0;
    std::uint64_t seg_data_file_offset = 0;
    std::uint64_t ana_data_file_offset = 0;
    std::uint64_t ana_data_end_file_offset = 0;

    std::uint32_t matrix_side = 0;
    std::size_t matrix_entries = 0;
    std::size_t occupied_entries = 0;
    std::size_t unique_axm_records = 0;

    std::uint64_t axm_serialized_size = 0;
    std::uint64_t seg_serialized_size = 0;
    std::uint64_t ana_serialized_size = 0;
    std::uint32_t ana_indexed_bytes = 0;
    std::uint32_t ana_trailing_bytes = 0;

    std::vector<std::uint32_t> matrix;
    std::vector<DiphoneUnit> units;
};

PhoneInventory parse_nicolai_phone_inventory(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t pho_data_file_offset);

DiphoneCatalog parse_nicolai_diphone_catalog(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout);

const DiphoneUnit* find_diphone(
    const DiphoneCatalog& catalog,
    std::uint8_t left_index,
    std::uint8_t right_index);

const DiphoneUnit* find_diphone(
    const DiphoneCatalog& catalog,
    const std::string& left_phone,
    const std::string& right_phone);

std::vector<std::uint8_t> extract_diphone_compressed_bytes(
    const std::vector<std::uint8_t>& bytes,
    const DiphoneCatalog& catalog,
    const DiphoneUnit& unit);

} // namespace nicolai
