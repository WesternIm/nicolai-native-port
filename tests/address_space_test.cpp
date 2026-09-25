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
    l.tag0=l.tag1=nicolai::kEdatTaggedRefMagic;
    l.segment0_offset=0x18; l.segment0_size=0x100; l.segment0_end=0x118;
    l.segment1_offset=0x118; l.segment1_size=0x100; l.segment1_end=0x218;
    l.footer_offset=0x218; l.footer_size=0x48;

    // The serialized payload is a logical offset. Duration selects the base.
    put32(b,0x20,nicolai::kEdatTaggedRefMagic); put32(b,0x24,0x20);
    const auto raw=nicolai::decode_static_ref_at(b,l,0x20);
    assert(raw.valid && raw.logical_offset==0x20);
    const auto range=nicolai::make_edat_range_descriptor(l);
    const auto init=nicolai::resolve_static_ref({range},nicolai::StaticDuration::Initialization,raw);
    const auto exec=nicolai::resolve_static_ref({range},nicolai::StaticDuration::Execution,raw);
    assert(init.valid && init.file_offset==0x38);
    assert(exec.valid && exec.file_offset==0x138);

    const auto enc=nicolai::encode_static_address({range},nicolai::StaticDuration::Execution,0x138);
    assert(enc.valid && enc.tag==nicolai::kEdatTaggedRefMagic && enc.logical_offset==0x20);

    const auto bad=nicolai::resolve_static_ref({range},nicolai::StaticDuration::Execution,
                                                nicolai::kEdatTaggedRefMagic,0x101);
    assert(!bad.valid);

    // Header tag/offset fields are descriptors, not serialized SycStadPntr refs.
    put32(b,0x08,nicolai::kEdatTaggedRefMagic); put32(b,0x0c,0x18);
    const auto refs=nicolai::scan_serialized_static_refs(b,l);
    assert(refs.size()==1 && refs.front().source_offset==0x20);
    std::cout << "address_space_test: PASSED\n";
}
