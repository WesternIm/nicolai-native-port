#include "nicolai/address_space.hpp"
#include "nicolai/edat.hpp"
#include "nicolai/static_files.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

static std::vector<std::uint8_t> read_all(const char* p) {
    std::ifstream f(p,std::ios::binary); if(!f) return {};
    return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};
}
static void show(const char* label,const nicolai::ResolvedTaggedRef& r) {
    std::cout<<label<<": ";
    if(!r.valid){std::cout<<"unresolved\n";return;}
    std::cout<<"source=0x"<<std::hex<<r.source_offset<<" -> target=0x"<<r.target_offset
             <<" ["<<nicolai::to_string(r.target.region)<<"+0x"<<r.target.relative_offset<<"]"<<std::dec<<"\n";
}
int main(int argc,char**argv){
    if(argc!=2){std::cerr<<"usage: nicolai_m7_probe nicolai16.dat\n";return 2;}
    const auto b=read_all(argv[1]); const auto l=nicolai::parse_edat_layout(b);
    if(!l.valid){std::cerr<<"invalid EDAT\n";return 1;}
    const auto refs=nicolai::scan_resolved_tagged_refs(b,l);
    std::map<nicolai::EdatRegion,std::size_t> sc,tc;
    std::map<std::pair<nicolai::EdatRegion,nicolai::EdatRegion>,std::size_t> pc;
    std::size_t bad=0;
    for(const auto&r:refs){++sc[r.source_region];if(!r.valid){++bad;continue;}++tc[r.target.region];++pc[{r.source_region,r.target.region}];}
    std::cout<<"Nicolai native-port M7 EDAT address-space probe\n\n"
             <<"segment0: [0x"<<std::hex<<l.segment0_offset<<",0x"<<l.segment0_end<<")\n"
             <<"segment1: [0x"<<l.segment1_offset<<",0x"<<l.segment1_end<<")\n"
             <<"footer:   [0x"<<l.footer_offset<<",0x"<<b.size()<<")\n"<<std::dec
             <<"tagged refs: "<<refs.size()<<" unresolved: "<<bad<<"\n";
    for(auto r:{nicolai::EdatRegion::Header,nicolai::EdatRegion::Segment0,nicolai::EdatRegion::Segment1,nicolai::EdatRegion::Footer})
        std::cout<<"  source "<<nicolai::to_string(r)<<": "<<sc[r]<<"  target "<<nicolai::to_string(r)<<": "<<tc[r]<<"\n";
    std::cout<<"\nsource -> target:\n";
    for(const auto&kv:pc) std::cout<<"  "<<nicolai::to_string(kv.first.first)<<" -> "<<nicolai::to_string(kv.first.second)<<": "<<kv.second<<"\n";
    show("segment0 root",nicolai::resolve_tagged_ref_at(b,l,l.segment0_offset));
    show("segment1 root",nicolai::resolve_tagged_ref_at(b,l,l.segment1_offset));
    const auto recs=nicolai::scan_static_file_records(b,l);
    const auto run=nicolai::longest_static_record_lockstep_run(recs);
    std::cout<<"\npath-bearing metadata records: "<<recs.size()<<"\n";
    if(run.count){
      const auto& first=recs[run.begin_index];
      const auto& last=recs[run.begin_index+run.count-1];
      std::cout<<"longest 0x120 lockstep allocation pattern: "<<run.count<<" records\n"
               <<"  serialized: 0x"<<std::hex<<first.tagged_ref_offset<<" .. 0x"<<last.tagged_ref_offset<<"\n"
               <<"  primary:    0x"<<first.primary_ref_offset<<" .. 0x"<<last.primary_ref_offset<<std::dec<<"\n"
               <<"  first path: "<<first.path<<"\n"
               <<"  last path:  "<<last.path<<"\n";
    }
    auto dump=[&](const char*label,const nicolai::StaticFileRecord*r){
      std::cout<<"\n"<<label<<":\n"; if(!r){std::cout<<"  not found\n";return;}
      show("  primary ref",nicolai::resolve_tagged_ref_at(b,l,r->tagged_ref_offset));
      if(r->has_field_10c_ref) show("  secondary ref",nicolai::resolve_tagged_ref_at(b,l,r->path_offset+0x10c));
      std::cout<<"  fields: "<<r->field_104<<"/"<<r->field_108<<"\n  path: "<<r->path<<"\n";
    };
    dump("rsrc allocation metadata",nicolai::find_static_file_record(recs,"16stbi\\rsrc.dsc"));
    dump("modeinfo allocation metadata",nicolai::find_static_file_record(recs,"nicolai\\16aci\\modeinfo.dsc"));
    std::cout<<"\nmodel: tagged refs are absolute EDAT offsets resolved through loaded segment ranges.\n"
             <<"M6 write-patch materialization is deprecated; M7 performs no synthetic Segment0 overwrite.\n";
    return bad?1:0;
}
