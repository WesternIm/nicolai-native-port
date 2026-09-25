#include "nicolai/resource_registry.hpp"
#include <algorithm>
#include <cctype>
#include <set>

namespace nicolai {
namespace {
std::uint32_t le32(const std::vector<std::uint8_t>& b, std::size_t o) {
    if (o + 4 > b.size()) return 0;
    return static_cast<std::uint32_t>(b[o]) |
           (static_cast<std::uint32_t>(b[o+1]) << 8) |
           (static_cast<std::uint32_t>(b[o+2]) << 16) |
           (static_cast<std::uint32_t>(b[o+3]) << 24);
}
struct Ptr { bool valid=false; bool is_null=false; std::uint32_t logical=0; };
Ptr ptr_at(const std::vector<std::uint8_t>& b, std::size_t file_off) {
    Ptr p;
    if (file_off + 8 > b.size()) return p;
    const auto a=le32(b,file_off), x=le32(b,file_off+4);
    if (a==0 && x==0) { p.valid=true; p.is_null=true; return p; }
    if (a!=kEdatTaggedRefMagic) return p;
    p.valid=true; p.logical=x; return p;
}
std::string ascii_at_exec(const std::vector<std::uint8_t>& b,
                          const EdatLayout& l,
                          std::uint32_t logical,
                          std::size_t max_len=1024) {
    if (logical > l.segment1_size) return {};
    const auto o=static_cast<std::size_t>(l.segment1_offset)+logical;
    const auto end=std::min<std::size_t>(b.size(), std::min<std::uint64_t>(l.segment1_end,o+max_len));
    std::string s;
    for (std::size_t i=o;i<end && b[i]!=0;++i) {
        if (b[i]<0x20 || b[i]>0x7e) return {};
        s.push_back(static_cast<char>(b[i]));
    }
    return s;
}
std::string lower(std::string s) {
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return s;
}
}

ResourceRegistry parse_exec_resource_registry(const std::vector<std::uint8_t>& bytes,
                                              const EdatLayout& layout,
                                              const StaticFileRecord& rsrc,
                                              std::size_t max_entries) {
    ResourceRegistry out;
    if (!layout.valid || !rsrc.has_field_10c_ref) return out;
    const auto range=make_edat_range_descriptor(layout);
    const auto root=resolve_static_ref({range}, StaticDuration::Execution,
                                       kEdatTaggedRefMagic, rsrc.field_10c_target);
    if (!root.valid || root.file_offset + 24 > bytes.size()) return out;
    out.header.logical_offset=rsrc.field_10c_target;
    const auto p0=ptr_at(bytes,static_cast<std::size_t>(root.file_offset));
    const auto p1=ptr_at(bytes,static_cast<std::size_t>(root.file_offset+8));
    const auto p2=ptr_at(bytes,static_cast<std::size_t>(root.file_offset+16));
    if (!p0.valid || p0.is_null || !p1.valid || p1.is_null || !p2.valid || p2.is_null) return out;
    out.header.aux_logical=p0.logical;
    out.header.base_dir_logical=p1.logical;
    out.header.head_logical=p2.logical;
    out.header.base_dir=ascii_at_exec(bytes,layout,p1.logical);
    out.header.valid=!out.header.base_dir.empty();
    if (!out.header.valid) return out;

    std::set<std::uint32_t> seen;
    auto cur=p2.logical;
    for (std::size_t n=0;n<max_entries;++n) {
        if (!seen.insert(cur).second) return out;
        if (cur > layout.segment1_size || cur + 24 > layout.segment1_size) return out;
        const auto fo=static_cast<std::size_t>(layout.segment1_offset)+cur;
        const auto pn=ptr_at(bytes,fo), pp=ptr_at(bytes,fo+8), px=ptr_at(bytes,fo+16);
        if (!pn.valid || pn.is_null || !pp.valid || pp.is_null || !px.valid) return out;
        ResourceRegistryEntry e;
        e.node_logical=cur; e.name_logical=pn.logical; e.path_logical=pp.logical;
        e.name=ascii_at_exec(bytes,layout,pn.logical);
        e.path=ascii_at_exec(bytes,layout,pp.logical);
        if (e.name.empty()) return out;
        if (px.is_null) {
            e.next_logical=0;
            out.entries.push_back(std::move(e));
            out.terminated=true;
            out.valid=true;
            return out;
        }
        e.next_logical=px.logical;
        out.entries.push_back(std::move(e));
        cur=px.logical;
    }
    return out;
}

const ResourceRegistryEntry* find_resource(const ResourceRegistry& r,
                                           const std::string& name) {
    const auto want=lower(name);
    for (const auto& e:r.entries) if (lower(e.name)==want) return &e;
    return nullptr;
}

} // namespace nicolai
