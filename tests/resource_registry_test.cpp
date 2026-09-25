#include "nicolai/resource_registry.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static void put32(std::vector<std::uint8_t>& b,std::size_t o,std::uint32_t v){
  b[o]=static_cast<std::uint8_t>(v); b[o+1]=static_cast<std::uint8_t>(v>>8);
  b[o+2]=static_cast<std::uint8_t>(v>>16); b[o+3]=static_cast<std::uint8_t>(v>>24);
}
static void putptr(std::vector<std::uint8_t>& b,std::size_t o,std::uint32_t logical){
  put32(b,o,nicolai::kEdatTaggedRefMagic); put32(b,o+4,logical);
}
static void putstr(std::vector<std::uint8_t>& b,std::size_t o,const std::string&s){
  for(std::size_t i=0;i<s.size();++i)b[o+i]=static_cast<std::uint8_t>(s[i]); b[o+s.size()]=0;
}
int main(){
  std::vector<std::uint8_t>b(0x500,0);
  nicolai::EdatLayout l; l.valid=true; l.tag0=l.tag1=nicolai::kEdatTaggedRefMagic;
  l.segment0_offset=0x18; l.segment0_size=0x180; l.segment0_end=0x198;
  l.segment1_offset=0x198; l.segment1_size=0x200; l.segment1_end=0x398;
  l.footer_offset=0x398; l.footer_size=0x168;
  const auto base=static_cast<std::size_t>(l.segment1_offset);

  // Registry header at execution logical 0x20: aux, base-dir, head.
  putptr(b,base+0x20,0x60); putptr(b,base+0x28,0x70); putptr(b,base+0x30,0xA0);
  putstr(b,base+0x70,"c:\\voice\\");
  // One node: name, path, null-next.
  putptr(b,base+0xA0,0xC0); putptr(b,base+0xA8,0xE0);
  put32(b,base+0xB0,0); put32(b,base+0xB4,0);
  putstr(b,base+0xC0,"Syc.Test"); putstr(b,base+0xE0,"c:\\voice\\test.dsc");

  nicolai::StaticFileRecord r; r.has_field_10c_ref=true; r.field_10c_target=0x20;
  const auto reg=nicolai::parse_exec_resource_registry(b,l,r);
  assert(reg.valid && reg.terminated && reg.header.valid);
  assert(reg.header.base_dir=="c:\\voice\\");
  assert(reg.entries.size()==1);
  assert(reg.entries[0].name=="Syc.Test");
  assert(reg.entries[0].path=="c:\\voice\\test.dsc");
  const auto* e=nicolai::find_resource(reg,"syc.test");
  assert(e && e->path=="c:\\voice\\test.dsc");
  std::cout<<"resource_registry_test: PASSED\n";
}
