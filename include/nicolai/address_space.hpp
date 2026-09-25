#pragma once
#include "nicolai/edat.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nicolai {

// M8 correction: an EDAT serialized pointer is not an absolute file offset.
// Legacy SycStadPntr() resolves (tag, logical_offset, duration) through a
// table of 0x60-byte range descriptors.  Duration 0/1 corresponds to the two
// static-data segments (initialisation/execution in the original diagnostics).
enum class StaticDuration : std::uint8_t {
    Initialization = 0,
    Execution = 1
};

const char* to_string(StaticDuration d) noexcept;

struct LegacyStaticRef {
    bool valid = false;
    std::uint64_t source_offset = 0;
    EdatRegion source_region = EdatRegion::Outside;
    std::uint32_t tag = 0;
    std::uint32_t logical_offset = 0;
};

struct StaticRangeDescriptor {
    std::array<std::uint32_t, 2> tag{};
    // Portable stand-in for the x86 runtime segment bases.  For a mapped EDAT
    // file these are the two segment file offsets.
    std::array<std::uint64_t, 2> base_file_offset{};
    std::array<std::uint32_t, 2> max_offset{};
};

struct ResolvedStaticRef {
    bool valid = false;
    std::size_t range_index = 0;
    StaticDuration duration = StaticDuration::Initialization;
    std::uint32_t tag = 0;
    std::uint32_t logical_offset = 0;
    std::uint64_t file_offset = 0;
};

struct EncodedStaticRef {
    bool valid = false;
    std::size_t range_index = 0;
    StaticDuration duration = StaticDuration::Initialization;
    std::uint32_t tag = 0;
    std::uint32_t logical_offset = 0;
};

StaticRangeDescriptor make_edat_range_descriptor(const EdatLayout& layout) noexcept;
LegacyStaticRef decode_static_ref_at(const std::vector<std::uint8_t>& bytes,
                                     const EdatLayout& layout,
                                     std::uint64_t source_offset);
ResolvedStaticRef resolve_static_ref(const std::vector<StaticRangeDescriptor>& ranges,
                                     StaticDuration duration,
                                     std::uint32_t tag,
                                     std::uint32_t logical_offset) noexcept;
ResolvedStaticRef resolve_static_ref(const std::vector<StaticRangeDescriptor>& ranges,
                                     StaticDuration duration,
                                     const LegacyStaticRef& ref) noexcept;
ResolvedStaticRef resolve_edat_static_ref_at(const std::vector<std::uint8_t>& bytes,
                                             const EdatLayout& layout,
                                             std::uint64_t source_offset,
                                             StaticDuration duration);
EncodedStaticRef encode_static_address(const std::vector<StaticRangeDescriptor>& ranges,
                                       StaticDuration duration,
                                       std::uint64_t file_offset) noexcept;
std::vector<LegacyStaticRef> scan_serialized_static_refs(const std::vector<std::uint8_t>& bytes,
                                                         const EdatLayout& layout);

} // namespace nicolai
