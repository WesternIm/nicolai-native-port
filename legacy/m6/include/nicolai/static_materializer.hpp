#pragma once

#include "nicolai/edat.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace nicolai {

// HISTORICAL M6 EXPERIMENT.
// M7 supersedes the claim below that the complete runtime node is 0x114 bytes.
// The original x86 implementation proves the runtime descriptor is 0x120 bytes.
// Keep this API only for reproducing M6; new code should use static_replay.hpp.
//
// M6: exact path-bearing static-file patch format observed in EDAT Segment 1.
//
// Serialized form (0x120 bytes):
//   +0x000 tagged destination ref  (8 bytes: magic + Segment0 offset)
//   +0x008 char path[0x104]
//   +0x10c u32 field_104
//   +0x110 u32 field_108
//   +0x114 tagged content/object ref (8 bytes)
//   +0x11c u32 state/flag
//
// Runtime/materialized form written at destination (0x114 bytes on 32-bit x86):
//   +0x000 char path[0x104]
//   +0x104 u32 field_104
//   +0x108 u32 field_108
//   +0x10c pointer content/object
//   +0x110 u32 state/flag
//
// The portable materializer stores the EDAT target offset at runtime +0x10c as
// a 32-bit surrogate instead of inventing a host pointer.  A later relocation
// layer may convert that offset to an address in a mapped Segment0 image.
constexpr std::size_t kStaticFilePatchSerializedSize = 0x120;
constexpr std::size_t kStaticFileNodeRuntimeSize = 0x114;
constexpr std::size_t kStaticFilePathSize = 0x104;

struct StaticFilePatch {
    std::uint64_t serialized_offset = 0;
    std::uint32_t destination_offset = 0;
    std::string path;
    std::uint32_t field_104 = 0;
    std::uint32_t field_108 = 0;
    std::uint32_t object_ref_offset = 0;
    std::uint32_t state = 0;
};

struct MaterializedStaticFileNode {
    std::uint32_t destination_offset = 0;
    std::array<std::uint8_t, kStaticFileNodeRuntimeSize> bytes{};
};

std::vector<StaticFilePatch> scan_static_file_patches(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout);

MaterializedStaticFileNode materialize_static_file_patch_offsets(
    const StaticFilePatch& patch);

// Applies the path-bearing patch nodes to a copy of an EDAT image.  Pointer
// fields remain EDAT file offsets (portable surrogate form).  Returns false if
// any destination lies outside Segment0 or the image.
bool apply_static_file_patches_offset_view(
    std::vector<std::uint8_t>& image,
    const EdatLayout& layout,
    const std::vector<StaticFilePatch>& patches);

} // namespace nicolai
