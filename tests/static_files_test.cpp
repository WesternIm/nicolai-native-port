#include "nicolai/static_files.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

static void put32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) {
    b[o] = static_cast<std::uint8_t>(v);
    b[o+1] = static_cast<std::uint8_t>(v >> 8);
    b[o+2] = static_cast<std::uint8_t>(v >> 16);
    b[o+3] = static_cast<std::uint8_t>(v >> 24);
}

int main() {
    std::vector<std::uint8_t> b(0x400, 0);
    nicolai::EdatLayout l;
    l.valid = true; l.segment0_offset = 0x18; l.segment0_end = 0x180;
    l.segment1_offset = 0x180; l.segment1_end = 0x400;

    const std::size_t ref = 0x190;
    put32(b, ref, nicolai::kEdatTaggedRefMagic);
    put32(b, ref+4, 0x80);
    const char* path = "c:\\x\\rsrc.dsc";
    for (std::size_t i=0; path[i]; ++i) b[ref+8+i] = static_cast<std::uint8_t>(path[i]);
    put32(b, ref+8+0x104, 2);
    put32(b, ref+8+0x108, 2);
    put32(b, ref+8+0x10c, nicolai::kEdatTaggedRefMagic);
    put32(b, ref+8+0x110, 0x40);

    auto r = nicolai::scan_static_file_records(b, l);
    assert(r.size() == 1);
    assert(r[0].primary_ref_offset == 0x80);
    assert(r[0].field_104 == 2 && r[0].field_108 == 2);
    assert(r[0].has_field_10c_ref && r[0].field_10c_target == 0x40);
    assert(nicolai::find_static_file_record(r, "rsrc.dsc") != nullptr);

    // Geometry helper: three records can advance in lockstep without implying
    // any write-patch semantics.
    std::vector<nicolai::StaticFileRecord> geom(3);
    geom[0].tagged_ref_offset=0x400; geom[0].primary_ref_offset=0x80;
    geom[1].tagged_ref_offset=0x520; geom[1].primary_ref_offset=0x1A0;
    geom[2].tagged_ref_offset=0x640; geom[2].primary_ref_offset=0x2C0;
    const auto gr=nicolai::longest_static_record_lockstep_run(geom);
    assert(gr.begin_index==0 && gr.count==3);

    std::vector<std::uint8_t> p(200, 0);
    const std::vector<std::uint8_t> m{1,2,3,4,5};
    for (int i=0;i<7;++i) std::copy(m.begin(),m.end(),p.begin()+10+i*21);
    auto run=nicolai::detect_repeating_marker_run(p,0,p.size(),5,21,32);
    assert(run.count==7 && run.first_offset==10);
    std::cout << "static_files_test: PASSED\n";
}
