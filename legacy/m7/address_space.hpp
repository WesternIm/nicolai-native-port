#pragma once
#include "nicolai/edat.hpp"
#include <cstdint>
#include <vector>

namespace nicolai {

// M7: platform-neutral form of a legacy 32-bit EDAT address.
// The x86 loader uses absolute EDAT offsets and segment/range mappings before
// producing process pointers.  We keep offsets as offsets on ARM64.
struct EdatAddress {
    bool valid = false;
    EdatRegion region = EdatRegion::Outside;
    std::uint64_t file_offset = 0;
    std::uint64_t region_begin = 0;
    std::uint64_t relative_offset = 0;
};

struct ResolvedTaggedRef {
    bool valid = false;
    std::uint64_t source_offset = 0;
    EdatRegion source_region = EdatRegion::Outside;
    std::uint32_t magic = 0;
    std::uint32_t target_offset = 0;
    EdatAddress target{};
};

EdatAddress resolve_edat_offset(const EdatLayout& layout,
                                std::uint64_t file_size,
                                std::uint64_t file_offset) noexcept;
ResolvedTaggedRef resolve_tagged_ref_at(const std::vector<std::uint8_t>& bytes,
                                        const EdatLayout& layout,
                                        std::uint64_t source_offset);
std::vector<ResolvedTaggedRef> scan_resolved_tagged_refs(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout);

} // namespace nicolai
