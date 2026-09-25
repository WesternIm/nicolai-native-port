#include "nicolai/voice_catalog.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

static void p32(std::vector<std::uint8_t>&b,std::size_t o,std::uint32_t v){b[o]=v;b[o+1]=v>>8;b[o+2]=v>>16;b[o+3]=v>>24;}
static void p16(std::vector<std::uint8_t>&b,std::size_t o,std::uint16_t v){b[o]=v;b[o+1]=v>>8;}
static void ps(std::vector<std::uint8_t>&b,std::size_t o,const std::string&s){for(std::size_t i=0;i<s.size();++i)b[o+i]=static_cast<std::uint8_t>(s[i]);b[o+s.size()]=0;}
int main(){
  constexpr std::size_t s0=0x18, s0sz=0x1000, s1=s0+s0sz, s1sz=0x24000;
  std::vector<std::uint8_t>b(s1+s1sz+0x100,0);
  nicolai::EdatLayout l; l.valid=true;l.tag0=l.tag1=nicolai::kEdatTaggedRefMagic;
  l.segment0_offset=s0;l.segment0_size=s0sz;l.segment0_end=s1;l.segment1_offset=s1;l.segment1_size=s1sz;l.segment1_end=s1+s1sz;l.footer_offset=l.segment1_end;

  ps(b,s0+0x40,"Nicolai");
  const auto fo=s1+0x80; ps(b,fo,"c:\\voice\\voix.dsc");p32(b,fo+0x104,2);p32(b,fo+0x108,2);
  p32(b,fo+0x10c,nicolai::kEdatTaggedRefMagic);p32(b,fo+0x110,0x400);p32(b,fo+0x114,1);

  const auto vr=s1+0x400; p32(b,vr,nicolai::kEdatTaggedRefMagic);p32(b,vr+4,0x40);
  p32(b,vr+8,353);p32(b,vr+0xc,643);p32(b,vr+0x10,127);
  ps(b,vr+0x118,"c:\\front\\exc_rus.txt");
  ps(b,vr+0x340,"c:\\tempo-psola\\russian\\nicolai\\16aci\\nbr16aci.dsc");
  ps(b,vr+0x444,"c:\\tempo-psola\\russian\\nicolai\\16aci\\modeinfo.dsc");
  for(std::size_t i=0;i<255;++i)p16(b,vr+0x684+i*2,static_cast<std::uint16_t>(i));

  const auto objs=nicolai::scan_legacy_file_objects(b,l,nicolai::StaticDuration::Execution);
  const auto* voix=nicolai::find_legacy_file_object(objs,"voix.dsc");assert(voix);
  const auto cat=nicolai::parse_voice_catalog(b,l,*voix);assert(cat.valid&&cat.occupied==1&&cat.capacity==64);
  const auto*n=nicolai::find_voice(cat,"nicolai");assert(n&&n->index==0&&n->identity_symbol_map);
  assert(n->raw_08==353&&n->raw_0c==643&&n->raw_10==127);
  assert(n->acoustic_dsc_path.find("nbr16aci.dsc")!=std::string::npos);
  std::cout<<"voice_catalog_test: PASSED\n";
}
