#include "nicolai/address_space.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

static void put32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) {
    b[o]=static_cast<std::uint8_t>(v); b[o+1]=static_cast<std::uint8_t>(v>>8);
    b[o+2]=static_cast<std::uint8_t>(v>>16); b[o+3]=static_cast<std::uint8_t>(v>>24);
}
int main() {
    std::vector<std::uint8_t> b(0x260,0);
    nicolai::EdatLayout l; l.valid=true;
    l.segment0_offset=0x18; l.segment0_size=0x100; l.segment0_end=0x118;
    l.segment1_offset=0x118; l.segment1_size=0x100; l.segment1_end=0x218;
    l.footer_offset=0x218; l.footer_size=0x48;
    put32(b,0x20,nicolai::kEdatTaggedRefMagic); put32(b,0x24,0x80);
    auto a=nicolai::resolve_tagged_ref_at(b,l,0x20);
    assert(a.valid && a.target.region==nicolai::EdatRegion::Segment0);
    assert(a.target.relative_offset==0x68);
    put32(b,0x120,nicolai::kEdatTaggedRefMagic); put32(b,0x124,0x180);
    auto c=nicolai::resolve_tagged_ref_at(b,l,0x120);
    assert(c.valid && c.target.region==nicolai::EdatRegion::Segment1);
    assert(c.target.relative_offset==0x68);
    put32(b,0x128,nicolai::kEdatTaggedRefMagic); put32(b,0x12c,0x9999);
    assert(!nicolai::resolve_tagged_ref_at(b,l,0x128).valid);
    std::cout << "address_space_test: PASSED\n";
}
