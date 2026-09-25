#include "nicolai/static_replay.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

static void put32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) {
    b[o] = static_cast<std::uint8_t>(v);
    b[o+1] = static_cast<std::uint8_t>(v >> 8);
    b[o+2] = static_cast<std::uint8_t>(v >> 16);
    b[o+3] = static_cast<std::uint8_t>(v >> 24);
}

static std::uint32_t get32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

static void make_record(std::vector<std::uint8_t>& b, std::size_t o,
                        std::uint32_t target, const char* path,
                        std::uint32_t ptr_target) {
    put32(b, o + 0x000, nicolai::kEdatTaggedRefMagic);
    put32(b, o + 0x004, target);
    std::memset(b.data() + o + 8, 0, nicolai::kStaticDescriptorPathSize);
    std::memcpy(b.data() + o + 8, path, std::strlen(path));
    put32(b, o + 0x10c, 2);
    put32(b, o + 0x110, 2);
    put32(b, o + 0x114, nicolai::kEdatTaggedRefMagic);
    put32(b, o + 0x118, ptr_target);
    put32(b, o + 0x11c, 1);
}

int main() {
    std::vector<std::uint8_t> b(0x900, 0);
    nicolai::EdatLayout l;
    l.valid = true;
    l.segment0_offset = 0x18;
    l.segment0_end = 0x400;
    l.segment1_offset = 0x400;
    l.segment1_end = 0x900;

    make_record(b, 0x420, 0x80,  "c:\\voice\\one.dat", 0x40);
    make_record(b, 0x540, 0x1A0, "c:\\voice\\two.dat", 0x44);
    make_record(b, 0x660, 0x2C0, "c:\\voice\\three.dat", 0x48);

    const auto rs = nicolai::scan_static_replay_path_records(b, l);
    assert(rs.size() == 3);
    assert(rs[0].replay_target_offset == 0x80);
    assert(rs[0].scalar_104 == 2 && rs[0].scalar_108 == 2);
    assert(rs[0].has_pointer_ref && rs[0].pointer_target_offset == 0x40);
    assert(rs[0].scalar_110 == 1);

    const auto run = nicolai::longest_lockstep_replay_run(rs);
    assert(run.begin_index == 0);
    assert(run.count == 3);
    assert(run.serialized_stride == 0x120 && run.target_stride == 0x120);

    const auto prefix = nicolai::materialize_known_static_descriptor_prefix_offsets(rs[0]);
    static_assert(nicolai::kStaticDescriptorRuntimeSize == 0x120);
    static_assert(nicolai::kStaticDescriptorKnownPrefixSize == 0x114);
    assert(std::strcmp(reinterpret_cast<const char*>(prefix.bytes.data()), "c:\\voice\\one.dat") == 0);
    assert(get32(prefix.bytes.data() + 0x104) == 2);
    assert(get32(prefix.bytes.data() + 0x108) == 2);
    assert(get32(prefix.bytes.data() + 0x10c) == 0x40);
    assert(get32(prefix.bytes.data() + 0x110) == 1);

    std::cout << "static_replay_test: PASSED\n";
}
