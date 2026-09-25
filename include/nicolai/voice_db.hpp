#pragma once

#include "nicolai/edat.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace nicolai {

struct VoiceDbMetadata {
    std::uint64_t size_bytes = 0;
    std::uint32_t header_word0 = 0;
    std::uint32_t header_word1 = 0;
    std::uint32_t tagged_magic = 0;
    std::uint64_t nicolai_name_offset = 0;
    int sample_rate_hint = 0;
    bool has_nicolai_name = false;
    bool has_tempo_psola_marker = false;
    bool has_russian_frontend_marker = false;
    EdatLayout edat;
    std::vector<TaggedRef> tagged_refs;
    std::vector<ModeRecord> mode_records;
    std::vector<OffsetString> resource_paths;
    std::vector<std::string> evidence_strings;
};

class VoiceDb {
public:
    static VoiceDb load(const std::filesystem::path& path);

    const VoiceDbMetadata& metadata() const noexcept { return metadata_; }
    const std::vector<std::uint8_t>& bytes() const noexcept { return bytes_; }

private:
    std::vector<std::uint8_t> bytes_;
    VoiceDbMetadata metadata_;
};

} // namespace nicolai
