#include "nicolai/static_materializer.hpp"

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

int main() {
    std::vector<std::uint8_t> b(0x600, 0xCC);
    nicolai::EdatLayout l;
    l.valid = true;
    l.segment0_offset = 0x18;
    l.segment0_end = 0x300;
    l.segment1_offset = 0x300;
    l.segment1_end = 0x600;

    const std::size_t rec = 0x340;
    put32(b, rec + 0x000, nicolai::kEdatTaggedRefMagic);
    put32(b, rec + 0x004, 0x80); // materialization destination
    const char* path = "c:\\voice\\rsrc.dsc";
    std::memset(b.data() + rec + 8, 0, nicolai::kStaticFilePathSize);
    std::memcpy(b.data() + rec + 8, path, std::strlen(path));
    put32(b, rec + 0x10c, 2);
    put32(b, rec + 0x110, 2);
    put32(b, rec + 0x114, nicolai::kEdatTaggedRefMagic);
    put32(b, rec + 0x118, 0x40); // runtime +0x10c pointer target
    put32(b, rec + 0x11c, 1);

    auto patches = nicolai::scan_static_file_patches(b, l);
    assert(patches.size() == 1);
    const auto& p = patches[0];
    assert(p.serialized_offset == rec);
    assert(p.destination_offset == 0x80);
    assert(p.path == path);
    assert(p.field_104 == 2 && p.field_108 == 2);
    assert(p.object_ref_offset == 0x40);
    assert(p.state == 1);

    auto node = nicolai::materialize_static_file_patch_offsets(p);
    assert(node.bytes.size() == 0x114);
    assert(std::strcmp(reinterpret_cast<const char*>(node.bytes.data()), path) == 0);
    assert(get32(node.bytes.data() + 0x104) == 2);
    assert(get32(node.bytes.data() + 0x108) == 2);
    assert(get32(node.bytes.data() + 0x10c) == 0x40);
    assert(get32(node.bytes.data() + 0x110) == 1);

    assert(nicolai::apply_static_file_patches_offset_view(b, l, patches));
    assert(std::strcmp(reinterpret_cast<const char*>(b.data() + 0x80), path) == 0);
    assert(get32(b.data() + 0x80 + 0x10c) == 0x40);

    std::cout << "static_materializer_test: PASSED\n";
}
