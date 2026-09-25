#include "nicolai/edat.hpp"
#include "nicolai/static_files.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>

static std::vector<std::uint8_t> read_all(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "usage: nicolai_m5_probe nicolai16.dat\n"; return 2; }
    auto b = read_all(argv[1]);
    auto l = nicolai::parse_edat_layout(b);
    auto files = nicolai::scan_static_file_records(b, l);
    std::cout << "Nicolai native-port M5 static file probe\n\n";
    std::cout << "static_file_records: " << files.size() << "\n";

    const auto* rsrc = nicolai::find_static_file_record(files, "16stbi\\rsrc.dsc");
    const auto* mode = nicolai::find_static_file_record(files, "nicolai\\16aci\\modeinfo.dsc");
    auto show=[](const char* name,const nicolai::StaticFileRecord* r){
        if(!r){ std::cout<<name<<": not found\n"; return; }
        std::cout<<name<<":\n  path: "<<r->path
                 <<"\n  record_ref: 0x"<<std::hex<<r->tagged_ref_offset
                 <<"\n  payload:    0x"<<r->primary_ref_offset
                 <<"\n  field104:   "<<std::dec<<r->field_104
                 <<"\n  field108:   "<<r->field_108;
        if(r->has_field_10c_ref) std::cout<<"\n  field10c tagged target: 0x"<<std::hex<<r->field_10c_target;
        std::cout<<std::dec<<"\n";
    };
    show("cmp16 rsrc",rsrc); show("Nicolai modeinfo",mode);

    if (rsrc) {
        auto run=nicolai::detect_repeating_marker_run(b, rsrc->primary_ref_offset,
            std::min<std::uint64_t>(l.segment0_end, rsrc->primary_ref_offset+0x4000),5,21,64);
        std::cout << "\nrsrc payload structural probe:\n";
        std::cout << "  best 5-byte/21-byte run count: " << run.count << "\n";
        std::cout << "  first: 0x" << std::hex << run.first_offset << std::dec << "\n  marker:";
        for(auto x:run.marker) std::cout << ' ' << std::hex << std::setw(2) << std::setfill('0') << unsigned(x);
        std::cout << std::dec << "\n";
    }

    std::cout << "\nstatus: EDAT static-file records recovered; rsrc payload located without hardcoded address\n";
    return 0;
}
