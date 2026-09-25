#include "nicolai/static_replay.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>

namespace nicolai {
namespace {

std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) throw std::out_of_range("le32 outside buffer");
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o + 1]) << 8) |
           (static_cast<std::uint32_t>(b[o + 2]) << 16) |
           (static_cast<std::uint32_t>(b[o + 3]) << 24);
}

void put32(std::uint8_t* p, std::uint32_t v) {
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
    p[2] = static_cast<std::uint8_t>(v >> 16);
    p[3] = static_cast<std::uint8_t>(v >> 24);
}

bool plausible_path(const std::uint8_t* p, std::size_t n, std::string& out) {
    out.clear();
    for (std::size_t i = 0; i < n; ++i) {
        const auto c = p[i];
        if (c == 0) break;
        if (c < 0x20 || c > 0x7e) return false;
        out.push_back(static_cast<char>(c));
    }
    if (out.size() < 4) return false;
    return (out.find('\\') != std::string::npos || out.find('/') != std::string::npos) &&
           out.find('.') != std::string::npos;
}

std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

} // namespace

std::vector<StaticReplayPathRecord> scan_static_replay_path_records(
    const std::vector<std::uint8_t>& bytes,
    const EdatLayout& layout) {
    std::vector<StaticReplayPathRecord> out;
    if (!layout.valid || layout.segment1_end > bytes.size()) return out;

    for (std::size_t o = static_cast<std::size_t>(layout.segment1_offset);
         o + kStaticReplayRecordSpan <= layout.segment1_end;
         o += 4) {
        if (le32(bytes, o) != kEdatTaggedRefMagic) continue;
        const auto target = le32(bytes, o + 4);
        if (classify_offset(layout, bytes.size(), target) != EdatRegion::Segment0) continue;

        std::string path;
        if (!plausible_path(bytes.data() + o + 8, kStaticDescriptorPathSize, path)) continue;

        StaticReplayPathRecord r;
        r.serialized_offset = o;
        r.replay_target_offset = target;
        r.path = std::move(path);
        r.scalar_104 = le32(bytes, o + 0x10c);
        r.scalar_108 = le32(bytes, o + 0x110);
        r.has_pointer_ref = le32(bytes, o + 0x114) == kEdatTaggedRefMagic;
        if (r.has_pointer_ref) {
            r.pointer_target_offset = le32(bytes, o + 0x118);
        }
        r.scalar_110 = le32(bytes, o + 0x11c);
        out.push_back(std::move(r));
    }
    return out;
}

const StaticReplayPathRecord* find_static_replay_path_record(
    const std::vector<StaticReplayPathRecord>& records,
    const std::string& suffix) {
    const auto want = lower(suffix);
    for (const auto& r : records) {
        const auto p = lower(r.path);
        if (p.size() >= want.size() && p.compare(p.size() - want.size(), want.size(), want) == 0) {
            return &r;
        }
    }
    return nullptr;
}

StaticReplayRun longest_lockstep_replay_run(
    const std::vector<StaticReplayPathRecord>& records,
    std::uint32_t stride) {
    StaticReplayRun best;
    if (records.empty()) return best;
    for (std::size_t begin = 0; begin < records.size(); ++begin) {
        std::size_t count = 1;
        while (begin + count < records.size()) {
            const auto& a = records[begin + count - 1];
            const auto& b = records[begin + count];
            const auto ds = b.serialized_offset - a.serialized_offset;
            const auto dt = static_cast<std::uint64_t>(b.replay_target_offset) - a.replay_target_offset;
            if (ds != stride || dt != stride) break;
            ++count;
        }
        if (count > best.count) {
            best.begin_index = begin;
            best.count = count;
            best.serialized_stride = stride;
            best.target_stride = stride;
        }
    }
    return best;
}

KnownStaticDescriptorPrefix materialize_known_static_descriptor_prefix_offsets(
    const StaticReplayPathRecord& record) {
    KnownStaticDescriptorPrefix out;
    out.replay_target_offset = record.replay_target_offset;
    out.bytes.fill(0);

    const auto n = std::min<std::size_t>(record.path.size(), kStaticDescriptorPathSize - 1);
    std::memcpy(out.bytes.data(), record.path.data(), n);
    put32(out.bytes.data() + 0x104, record.scalar_104);
    put32(out.bytes.data() + 0x108, record.scalar_108);
    put32(out.bytes.data() + 0x10c, record.pointer_target_offset);
    put32(out.bytes.data() + 0x110, record.scalar_110);
    return out;
}

} // namespace nicolai
