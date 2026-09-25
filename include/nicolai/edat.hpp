#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

constexpr std::uint32_t kEdatTaggedRefMagic = 0xA4F22A56u;
constexpr std::size_t kEdatHeaderSize = 0x18;
constexpr std::size_t kModeRecordSize = 0xB50;

enum class EdatRegion {
    Header,
    Segment0,
    Segment1,
    Footer,
    Outside
};

const char* to_string(EdatRegion region) noexcept;

struct EdatLayout {
    std::uint32_t segment0_size = 0;
    std::uint32_t segment1_size = 0;
    std::uint32_t tag0 = 0;
    std::uint32_t segment0_offset = 0;
    std::uint32_t tag1 = 0;
    std::uint32_t segment1_offset = 0;
    std::uint64_t segment0_end = 0;
    std::uint64_t segment1_end = 0;
    std::uint64_t footer_offset = 0;
    std::uint64_t footer_size = 0;
    bool valid = false;
};

struct TaggedRef {
    std::uint64_t source_offset = 0;
    std::uint32_t target_offset = 0;
    EdatRegion source_region = EdatRegion::Outside;
    EdatRegion target_region = EdatRegion::Outside;
};

struct ModeRecord {
    std::uint64_t offset = 0;
    std::string vendor;
    std::string engine_name;
    std::string description;
    std::string style;
    std::string voice_name;
    std::string pitch_name;
    std::string database_name;
    std::uint32_t sample_rate = 0;
    std::uint32_t field_adc = 0;
    std::uint32_t field_ae0 = 0;
    std::uint32_t field_ae4 = 0;
    std::uint32_t field_ae8 = 0;
    std::uint32_t field_aec = 0;
    std::uint32_t field_af4 = 0;
    std::uint32_t field_af8 = 0;
    std::uint32_t field_afc = 0;
};

struct OffsetString {
    std::uint64_t offset = 0;
    std::string value;
};

EdatLayout parse_edat_layout(const std::vector<std::uint8_t>& bytes);
EdatRegion classify_offset(const EdatLayout& layout, std::uint64_t file_size, std::uint64_t offset) noexcept;
std::vector<TaggedRef> scan_tagged_refs(const std::vector<std::uint8_t>& bytes, const EdatLayout& layout);
std::vector<ModeRecord> scan_nicolai_mode_records(const std::vector<std::uint8_t>& bytes, const EdatLayout& layout);
std::vector<OffsetString> find_nicolai_resource_paths(const std::vector<std::uint8_t>& bytes);

} // namespace nicolai
