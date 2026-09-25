#include "nicolai/embedded_resources.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
static void p32(std::vector<std::uint8_t>&b,std::size_t o,std::uint32_t v){b[o]=v;b[o+1]=v>>8;b[o+2]=v>>16;b[o+3]=v>>24;}
static void ps(std::vector<std::uint8_t>&b,std::size_t o,const std::string&s){for(std::size_t i=0;i<s.size();++i)b[o+i]=static_cast<std::uint8_t>(s[i]);b[o+s.size()]=0;}
int main(){
  std::vector<std::uint8_t>b(0x500,0); nicolai::EdatLayout l;l.valid=true;l.tag0=l.tag1=nicolai::kEdatTaggedRefMagic;l.segment0_offset=0x18;l.segment0_size=0x300;l.segment0_end=0x318;l.segment1_offset=0x318;l.segment1_size=0x100;l.segment1_end=0x418;l.footer_offset=0x418;
  const std::size_t r=0x80; const std::uint32_t payload=0x180,path=0x220;
  p32(b,r,2);p32(b,r+4,2);p32(b,r+8,nicolai::kEdatTaggedRefMagic);p32(b,r+0xc,payload);p32(b,r+0x10,1);p32(b,r+0x14,nicolai::kEdatTaggedRefMagic);p32(b,r+0x18,path);
  ps(b,l.segment0_offset+path,"c:\\voice\\nicolai\\16aci\\test.ana");
  const auto all=nicolai::scan_init_embedded_resources(b,l);assert(all.size()==1);
  const auto n=nicolai::nicolai_embedded_resources(b,l);assert(n.size()==1);
  assert(n[0].primary_file_offset==l.segment0_offset+payload);
  assert(nicolai::find_embedded_resource_suffix(n,"test.ana"));
  std::cout<<"embedded_resources_test: PASSED\n";
}
