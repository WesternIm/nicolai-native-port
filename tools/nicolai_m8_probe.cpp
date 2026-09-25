#include "nicolai/address_space.hpp"
#include "nicolai/edat.hpp"
#include "nicolai/resource_registry.hpp"
#include "nicolai/static_files.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static std::vector<std::uint8_t> read_all(const char* p) {
    std::ifstream f(p,std::ios::binary); if(!f) return {};
    return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};
}
static std::string preview_ascii(const std::vector<std::uint8_t>& b,std::uint64_t o,std::size_t n=96){
    std::string s;
    for(std::size_t i=0;i<n && o+i<b.size();++i){
        const auto c=b[static_cast<std::size_t>(o+i)];
        if(c==0) break;
        s.push_back((c>=0x20 && c<=0x7e)?static_cast<char>(c):'.');
    }
    return s;
}
static void show_resolved(const char* label,const nicolai::ResolvedStaticRef& r,const std::vector<std::uint8_t>& b){
    std::cout<<label<<": ";
    if(!r.valid){std::cout<<"unresolved\n";return;}
    std::cout<<nicolai::to_string(r.duration)<<" logical=0x"<<std::hex<<r.logical_offset
             <<" -> file=0x"<<r.file_offset<<std::dec;
    const auto p=preview_ascii(b,r.file_offset);
    if(!p.empty()) std::cout<<"  ["<<p<<"]";
    std::cout<<"\n";
}
int main(int argc,char**argv){
    if(argc!=2){std::cerr<<"usage: nicolai_m8_probe nicolai16.dat\n";return 2;}
    const auto b=read_all(argv[1]); const auto l=nicolai::parse_edat_layout(b);
    if(!l.valid){std::cerr<<"invalid EDAT\n";return 1;}
    const auto range=nicolai::make_edat_range_descriptor(l);
    const std::vector<nicolai::StaticRangeDescriptor> ranges{range};

    std::cout<<"Nicolai native-port M8 legacy static-address probe\n\n";
    std::cout<<"segment0(init): file=0x"<<std::hex<<l.segment0_offset<<" size=0x"<<l.segment0_size<<"\n"
             <<"segment1(exec): file=0x"<<l.segment1_offset<<" size=0x"<<l.segment1_size<<std::dec<<"\n";
    std::cout<<"resolver formula: runtime/file base[duration] + logical_offset\n\n";

    show_resolved("segment0 root",nicolai::resolve_edat_static_ref_at(b,l,l.segment0_offset,nicolai::StaticDuration::Initialization),b);
    show_resolved("segment1 root",nicolai::resolve_edat_static_ref_at(b,l,l.segment1_offset,nicolai::StaticDuration::Execution),b);

    const auto refs=nicolai::scan_serialized_static_refs(b,l);
    std::size_t init_ok=0,exec_ok=0,both=0,neither=0;
    for(const auto&r:refs){
        const auto a=nicolai::resolve_static_ref(ranges,nicolai::StaticDuration::Initialization,r);
        const auto e=nicolai::resolve_static_ref(ranges,nicolai::StaticDuration::Execution,r);
        if(a.valid)++init_ok; if(e.valid)++exec_ok;
        if(a.valid&&e.valid)++both; else if(!a.valid&&!e.valid)++neither;
    }
    std::cout<<"\nserialized tagged refs (header excluded): "<<refs.size()<<"\n"
             <<"  fit init duration: "<<init_ok<<"\n"
             <<"  fit exec duration: "<<exec_ok<<"\n"
             <<"  fit both:          "<<both<<"\n"
             <<"  fit neither:       "<<neither<<"\n";

    const auto recs=nicolai::scan_static_file_records(b,l);
    std::cout<<"\npath-bearing metadata records: "<<recs.size()<<"\n";
    const auto* rsrc=nicolai::find_static_file_record(recs,"16stbi\\rsrc.dsc");
    const auto* mode=nicolai::find_static_file_record(recs,"nicolai\\16aci\\modeinfo.dsc");
    auto dump=[&](const char*label,const nicolai::StaticFileRecord* r){
        std::cout<<"\n"<<label<<":\n";
        if(!r){std::cout<<"  not found\n";return;}
        std::cout<<"  record file offset: 0x"<<std::hex<<r->tagged_ref_offset<<std::dec<<"\n"
                 <<"  path: "<<r->path<<"\n";
        const auto primary=nicolai::decode_static_ref_at(b,l,r->tagged_ref_offset);
        if(primary.valid){
            show_resolved("  primary/init",nicolai::resolve_static_ref(ranges,nicolai::StaticDuration::Initialization,primary),b);
            show_resolved("  primary/exec",nicolai::resolve_static_ref(ranges,nicolai::StaticDuration::Execution,primary),b);
        }
        if(r->has_field_10c_ref){
            const auto secondary=nicolai::decode_static_ref_at(b,l,r->path_offset+0x10c);
            show_resolved("  secondary/exec",nicolai::resolve_static_ref(ranges,nicolai::StaticDuration::Execution,secondary),b);
        }
    };
    dump("rsrc.dsc",rsrc); dump("modeinfo.dsc",mode);

    if(rsrc){
        const auto reg=nicolai::parse_exec_resource_registry(b,l,*rsrc);
        std::cout<<"\nresource registry:\n"
                 <<"  valid: "<<(reg.valid?"yes":"no")<<"\n"
                 <<"  terminated: "<<(reg.terminated?"yes":"no")<<"\n"
                 <<"  base_dir: "<<reg.header.base_dir<<"\n"
                 <<"  entries: "<<reg.entries.size()<<"\n";
        for(std::size_t i=0;i<std::min<std::size_t>(5,reg.entries.size());++i)
            std::cout<<"    ["<<i<<"] "<<reg.entries[i].name<<" -> "<<reg.entries[i].path<<"\n";
        for(const char* name:{"Syc.VoxDsc","Syc.CanauxDsc","Syc.Russkij.DataDir","Syc.Russkij.Abrev.Abreviations"}){
            if(const auto* e=nicolai::find_resource(reg,name))
                std::cout<<"  "<<name<<" -> "<<e->path<<"\n";
        }
        if(!reg.entries.empty())
            std::cout<<"  last: "<<reg.entries.back().name<<" -> "<<reg.entries.back().path<<"\n";
        if(!reg.valid) return 1;
    } else return 1;

    std::cout<<"\nM8 model: {tag, logical_offset} + explicit static duration -> range base + logical offset.\n"
             <<"Header tag/file-offset pairs are descriptors, not serialized runtime references.\n";
    return 0;
}
