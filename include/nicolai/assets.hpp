#pragma once

#include "nicolai/edat.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

struct VoiceAssetMap {
    bool valid = false;
    std::string error;

    std::uint64_t ana_path_offset = 0;
    std::uint64_t ana_descriptor_offset = 0;
    std::uint64_t analysis_object_offset = 0;
    std::uint64_t seg_object_offset = 0;

    std::uint64_t analysis_data_offset = 0;
    std::uint64_t analysis_data_end = 0;
    std::uint64_t analysis_data_size = 0;

    std::uint64_t compressed_voice_offset = 0;
    std::uint64_t compressed_voice_end = 0;
    std::uint64_t compressed_voice_size = 0;

    std::uint64_t next_object_offset = 0;
    std::string next_object_ascii;
};

// Locate the Nicolai analysis table and opaque compressed voice payload using
// the M8/M9 logical static-address model. Returned offsets are real EDAT file
// offsets, not serialized logical offsets. No legacy executable code is run.
VoiceAssetMap locate_nicolai_voice_assets(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout,
    const std::vector<TaggedRef>& refs);

} // namespace nicolai
