#include "nicolai/edat.hpp"
#include "nicolai/static_materializer.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::vector<std::uint8_t> read_all(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

static const nicolai::StaticFilePatch* find_suffix(
    const std::vector<nicolai::StaticFilePatch>& ps,
    const std::string& suffix) {
    for (const auto& p : ps) {
        if (p.path.size() >= suffix.size() &&
            p.path.compare(p.path.size() - suffix.size(), suffix.size(), suffix) == 0) return &p;
    }
    return nullptr;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: nicolai_m6_probe nicolai16.dat\n";
        return 2;
    }
    auto b = read_all(argv[1]);
    auto layout = nicolai::parse_edat_layout(b);
    auto patches = nicolai::scan_static_file_patches(b, layout);

    std::cout << "Nicolai native-port M6 EDAT materializer probe\n\n";
    std::cout << "exact path patches: " << patches.size() << "\n";
    std::cout << "serialized size: 0x" << std::hex << nicolai::kStaticFilePatchSerializedSize
              << " runtime size: 0x" << nicolai::kStaticFileNodeRuntimeSize << std::dec << "\n";

    const auto* rsrc = find_suffix(patches, "16stbi\\rsrc.dsc");
    const auto* mode = find_suffix(patches, "nicolai\\16aci\\modeinfo.dsc");

    auto show = [&](const char* label, const nicolai::StaticFilePatch* p) {
        std::cout << "\n" << label << ":\n";
        if (!p) { std::cout << "  not found\n"; return; }
        std::cout << "  serialized:  0x" << std::hex << p->serialized_offset << "\n"
                  << "  destination: 0x" << p->destination_offset << "\n"
                  << "  object_ref:  0x" << p->object_ref_offset << std::dec << "\n"
                  << "  field_104:   " << p->field_104 << "\n"
                  << "  field_108:   " << p->field_108 << "\n"
                  << "  state:       " << p->state << "\n"
                  << "  path:        " << p->path << "\n";

        if (p->destination_offset + nicolai::kStaticFileNodeRuntimeSize <= b.size()) {
            std::string before;
            for (std::size_t i=0;i<16;++i) {
                char tmp[4]; std::snprintf(tmp,sizeof(tmp),"%02X",unsigned(b[p->destination_offset+i]));
                if(i) before += ' '; before += tmp;
            }
            auto m = nicolai::materialize_static_file_patch_offsets(*p);
            std::string after;
            for (std::size_t i=0;i<16;++i) {
                char tmp[4]; std::snprintf(tmp,sizeof(tmp),"%02X",unsigned(m.bytes[i]));
                if(i) after += ' '; after += tmp;
            }
            std::cout << "  before[0:16]: " << before << "\n"
                      << "  after [0:16]: " << after << "\n";
        }
    };

    show("cmp16 rsrc patch", rsrc);
    show("Nicolai modeinfo patch", mode);

    // Evidence that M5's 21-byte run was pre-materialization backing memory,
    // not an rsrc-specific table: report whether its start lies before rsrc's
    // destination and whether it spans across that destination.
    const std::vector<std::uint8_t> marker{0xAD,0xAE,0x3C,0xAB,0xEC};
    std::vector<std::uint64_t> hits;
    if (mode && rsrc) {
        const auto lo = static_cast<std::size_t>(mode->destination_offset);
        const auto hi = std::min<std::size_t>(b.size(), static_cast<std::size_t>(rsrc->destination_offset) + 0x1000);
        for (std::size_t i=lo; i+marker.size()<=hi; ++i) {
            if (std::equal(marker.begin(), marker.end(), b.begin()+static_cast<std::ptrdiff_t>(i))) hits.push_back(i);
        }
        std::size_t stride21 = 0;
        for (std::size_t i=1;i<hits.size();++i) if (hits[i]-hits[i-1]==21) ++stride21;
        std::cout << "\npre-materialization 21-byte pattern:\n"
                  << "  hits in modeinfo..rsrc+0x1000: " << hits.size() << "\n";
        if (!hits.empty()) {
            std::cout << "  first: 0x" << std::hex << hits.front() << " last: 0x" << hits.back() << std::dec << "\n"
                      << "  adjacent stride-21 pairs: " << stride21 << "\n"
                      << "  crosses rsrc destination: "
                      << ((hits.front() < rsrc->destination_offset && hits.back() > rsrc->destination_offset) ? "yes" : "no") << "\n";
        }
    }

    auto image = b;
    const bool applied = nicolai::apply_static_file_patches_offset_view(image, layout, patches);
    std::cout << "\noffset-view materialization: " << (applied ? "ok" : "FAILED") << "\n";
    if (applied && rsrc) {
        std::cout << "  materialized rsrc path at Segment0 destination: "
                  << reinterpret_cast<const char*>(image.data() + rsrc->destination_offset) << "\n";
    }
    std::cout << "\nstatus: exact 0x120 -> 0x114 path-node patch materializer implemented\n";
    return applied ? 0 : 1;
}
