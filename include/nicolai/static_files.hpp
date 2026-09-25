#pragma once

#include "nicolai/edat.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

// Path-bearing static-allocation/resource metadata observed in EDAT Segment 1.
// M7 intentionally does not assign payload/destination semantics to the first
// tagged reference: x86 evidence shows all tagged refs pass through the common
// EDAT address/range resolver.
struct StaticFileRecord {
    std::uint64_t tagged_ref_offset = 0;
    std::uint32_t primary_ref_offset = 0;
    std::uint64_t path_offset = 0;
    std::string path;
    std::uint32_t field_104 = 0;
    std::uint32_t field_108 = 0;
    bool has_field_10c_ref = false;
    std::uint32_t field_10c_target = 0;
};


struct StaticRecordLockstepRun {
    std::size_t begin_index = 0;
    std::size_t count = 0;
    std::uint64_t serialized_stride = 0;
    std::uint32_t primary_stride = 0;
};

struct RepeatingMarkerRun {
    std::uint64_t first_offset = 0;
    std::size_t count = 0;
    std::size_t stride = 0;
    std::vector<std::uint8_t> marker;
};

std::vector<StaticFileRecord> scan_static_file_records(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout);

const StaticFileRecord* find_static_file_record(
    const std::vector<StaticFileRecord>& records,
    const std::string& suffix);


// Longest consecutive run where both the metadata-record source offset and the
// primary EDAT target advance by the requested stride.  This is an observation
// about allocation geometry, not a claim that the record is a write-patch.
StaticRecordLockstepRun longest_static_record_lockstep_run(
    const std::vector<StaticFileRecord>& records,
    std::uint64_t serialized_stride = 0x120,
    std::uint32_t primary_stride = 0x120);

RepeatingMarkerRun detect_repeating_marker_run(
    const std::vector<std::uint8_t>& bytes,
    std::uint64_t begin,
    std::uint64_t end,
    std::size_t marker_size,
    std::size_t stride,
    std::size_t search_prefix = 64);

} // namespace nicolai
