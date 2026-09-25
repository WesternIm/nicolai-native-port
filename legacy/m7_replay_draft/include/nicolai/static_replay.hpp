#pragma once

#include "nicolai/edat.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

// M7 correction of the M6 model.
//
// The original x86 engine allocates/copies a 0x120-byte runtime descriptor.
// EDAT Segment1 contains path-bearing replay records that also occupy 0x120
// bytes in the observed stream, but one 32-bit runtime pointer is represented
// as an 8-byte EDAT tagged reference.  Therefore the record is NOT a complete
// byte-for-byte image of the 0x120-byte runtime object.
//
// What is proven and portable:
//   serialized +0x000 : tagged replay/allocation target (magic + offset)
//   serialized +0x008 : path[0x104]
//   serialized +0x10c : scalar -> runtime +0x104
//   serialized +0x110 : scalar -> runtime +0x108
//   serialized +0x114 : tagged ref -> runtime +0x10c (offset surrogate)
//   serialized +0x11c : scalar -> runtime +0x110
//
// This reconstructs runtime bytes [0x000, 0x114).  Runtime fields
// +0x114/+0x118/+0x11c exist in the x86 implementation but are not claimed to
// be self-contained in this single serialized record.
constexpr std::size_t kStaticReplayRecordSpan = 0x120;
constexpr std::size_t kStaticDescriptorRuntimeSize = 0x120;
constexpr std::size_t kStaticDescriptorKnownPrefixSize = 0x114;
constexpr std::size_t kStaticDescriptorPathSize = 0x104;

struct StaticReplayPathRecord {
    std::uint64_t serialized_offset = 0;
    std::uint32_t replay_target_offset = 0;
    std::string path;
    std::uint32_t scalar_104 = 0;
    std::uint32_t scalar_108 = 0;
    bool has_pointer_ref = false;
    std::uint32_t pointer_target_offset = 0;
    std::uint32_t scalar_110 = 0;
};

struct StaticReplayRun {
    std::size_t begin_index = 0;
    std::size_t count = 0;
    std::uint32_t serialized_stride = 0;
    std::uint32_t target_stride = 0;
};

struct KnownStaticDescriptorPrefix {
    std::uint32_t replay_target_offset = 0;
    std::array<std::uint8_t, kStaticDescriptorKnownPrefixSize> bytes{};
};

std::vector<StaticReplayPathRecord> scan_static_replay_path_records(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout);

const StaticReplayPathRecord* find_static_replay_path_record(
    const std::vector<StaticReplayPathRecord>& records,
    const std::string& suffix);

// Find the longest consecutive run where both the serialized stream position
// and the Segment0 replay target advance by the same requested stride.
StaticReplayRun longest_lockstep_replay_run(
    const std::vector<StaticReplayPathRecord>& records,
    std::uint32_t stride = static_cast<std::uint32_t>(kStaticDescriptorRuntimeSize));

// Reconstruct only the x86 runtime prefix whose mapping is proven by the EDAT
// record.  Pointer values are kept as 32-bit EDAT offsets, never host pointers.
KnownStaticDescriptorPrefix materialize_known_static_descriptor_prefix_offsets(
    const StaticReplayPathRecord& record);

} // namespace nicolai
