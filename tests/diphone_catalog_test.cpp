#include "nicolai/diphone_catalog.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static void p32(std::vector<std::uint8_t>& b, std::size_t o, std::uint32_t v) {
    b[o]=static_cast<std::uint8_t>(v); b[o+1]=static_cast<std::uint8_t>(v>>8);
    b[o+2]=static_cast<std::uint8_t>(v>>16); b[o+3]=static_cast<std::uint8_t>(v>>24);
}
static void p16(std::vector<std::uint8_t>& b, std::size_t o, std::uint16_t v) {
    b[o]=static_cast<std::uint8_t>(v); b[o+1]=static_cast<std::uint8_t>(v>>8);
}
static void ps(std::vector<std::uint8_t>& b, std::size_t o, const std::string& s) {
    for(std::size_t i=0;i<s.size();++i)b[o+i]=static_cast<std::uint8_t>(s[i]);
    b[o+s.size()]=0;
}
static void file_obj(std::vector<std::uint8_t>&b,std::size_t o,const std::string&path,std::uint32_t data){
    ps(b,o,path);p32(b,o+0x104,2);p32(b,o+0x108,2);p32(b,o+0x10c,nicolai::kEdatTaggedRefMagic);p32(b,o+0x110,data);p32(b,o+0x114,1);
}
int main(){
    constexpr std::size_t base=0x18, segsz=0x3000;
    std::vector<std::uint8_t>b(base+segsz+0x100,0);
    nicolai::EdatLayout l; l.valid=true;l.tag0=l.tag1=nicolai::kEdatTaggedRefMagic;
    l.segment0_offset=base;l.segment0_size=segsz;l.segment0_end=base+segsz;
    l.segment1_offset=l.segment0_end;l.segment1_size=0;l.segment1_end=l.segment1_offset;l.footer_offset=l.segment1_end;

    file_obj(b,base+0x100,"c:\\voice\\nbr16aci.dsc.pho",0x800);
    file_obj(b,base+0x220,"c:\\voice\\nbr16aci.axm",0x900);
    file_obj(b,base+0x340,"c:\\voice\\nbr.rgl",0x954);
    file_obj(b,base+0x460,"c:\\voice\\nbr16aci.seg",0xA00);
    file_obj(b,base+0x580,"c:\\voice\\nbr16aci.ana",0xA20);

    // PHO: three symbols (#, p, b).
    p32(b,base+0x800,0);p32(b,base+0x804,3);ps(b,base+0x808,"#pb");

    // AXM: 3x3 u32 matrix followed by two 0x18-byte records.
    const auto ax=base+0x900; const std::uint32_t matrix_bytes=9*4;
    p32(b,ax+(0*3+1)*4,matrix_bytes);
    p32(b,ax+(1*3+2)*4,matrix_bytes+0x18);
    auto rec=[&](std::size_t o,std::uint8_t r,std::uint8_t c,std::uint32_t so){
        p32(b,o,0);p32(b,o+4,0);b[o+8]=1;b[o+9]=2;b[o+10]=r;b[o+11]=c;p32(b,o+12,so);p32(b,o+16,16);p32(b,o+20,0);
    };
    rec(ax+matrix_bytes,0,1,0);rec(ax+matrix_bytes+0x18,1,2,16);

    // SEG slices contain compressed ANA boundaries + four int16 metadata values.
    const auto sg=base+0xA00;
    p32(b,sg,0);p32(b,sg+4,static_cast<std::uint32_t>(-4));p16(b,sg+8,0xfffe);p16(b,sg+10,10);p16(b,sg+12,6);p16(b,sg+14,0xfff6);
    p32(b,sg+16,4);p32(b,sg+20,8);p16(b,sg+24,0xfffe);p16(b,sg+26,11);p16(b,sg+28,6);p16(b,sg+30,0xfff5);

    const auto an=base+0xA20; for(std::size_t i=0;i<10;++i)b[an+i]=static_cast<std::uint8_t>(0x50+i);
    // Standalone serialized ref supplies the next initialization target, delimiting ANA.
    p32(b,base+0x700,nicolai::kEdatTaggedRefMagic);p32(b,base+0x704,0xA2A);

    const auto cat=nicolai::parse_nicolai_diphone_catalog(b,l);
    assert(cat.valid);assert(cat.matrix_side==3);assert(cat.occupied_entries==2);assert(cat.units.size()==2);
    assert(cat.axm_serialized_size==84);assert(cat.seg_serialized_size==32);assert(cat.ana_serialized_size==10);
    assert(cat.ana_indexed_bytes==8);assert(cat.ana_trailing_bytes==2);
    const auto*u=nicolai::find_diphone(cat,"#","p");assert(u&&u->seg_offset==0&&u->compressed_end==4);
    const auto bytes=nicolai::extract_diphone_compressed_bytes(b,cat,*u);assert(bytes.size()==4&&bytes[0]==0x50&&bytes[3]==0x53);
    std::cout<<"diphone_catalog_test: PASSED\n";
}
