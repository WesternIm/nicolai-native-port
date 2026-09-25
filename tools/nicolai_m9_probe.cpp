#include "nicolai/ana_index.hpp"
#include "nicolai/assets.hpp"
#include "nicolai/edat.hpp"
#include "nicolai/voice_catalog.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>
static std::vector<std::uint8_t> rd(const char*p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void hx(std::uint64_t v){std::cout<<"0x"<<std::hex<<v<<std::dec;}
static void dump_obj(const char*label,const nicolai::LegacyFileObject*o){std::cout<<label<<":\n";if(!o){std::cout<<"  not found\n";return;}std::cout<<"  path: "<<o->path<<"\n  duration: "<<nicolai::to_string(o->duration)<<"\n  file: ";hx(o->file_offset);std::cout<<" logical: ";hx(o->logical_offset);std::cout<<"\n  data logical: ";hx(o->data_logical);std::cout<<"\n  fields: "<<o->field_104<<" / "<<o->field_108<<" state="<<o->state<<"\n";}
int main(int argc,char**argv){
 if(argc!=2){std::cerr<<"usage: nicolai_m9_probe nicolai16.dat\n";return 2;}
 const auto b=rd(argv[1]);const auto l=nicolai::parse_edat_layout(b);if(!l.valid){std::cerr<<"invalid EDAT\n";return 1;}
 std::cout<<"Nicolai native-port M9 voice/channel descriptor probe\n\n";
 const auto init=nicolai::scan_legacy_file_objects(b,l,nicolai::StaticDuration::Initialization);
 const auto exec=nicolai::scan_legacy_file_objects(b,l,nicolai::StaticDuration::Execution);
 std::cout<<"legacy file objects: init="<<init.size()<<" exec="<<exec.size()<<"\n\n";
 const auto*voix=nicolai::find_legacy_file_object(exec,"voix.dsc");
 const auto*canaux=nicolai::find_legacy_file_object(exec,"canaux.dsc");
 dump_obj("voix.dsc",voix);dump_obj("canaux.dsc",canaux);if(!voix||!canaux)return 1;

 const auto cat=nicolai::parse_voice_catalog(b,l,*voix);
 std::cout<<"\nvoice catalog:\n  valid: "<<(cat.valid?"yes":"no")<<"\n  records file: ";hx(cat.records_file_offset);std::cout<<"\n  capacity: "<<cat.capacity<<"\n  occupied: "<<cat.occupied<<"\n";
 const auto*n=nicolai::find_voice(cat,"Nicolai");if(!n)return 1;
 std::cout<<"\nNicolai voice record:\n  index: "<<n->index<<"\n  file: ";hx(n->file_offset);std::cout<<"\n  init-name logical: ";hx(n->name_logical);std::cout<<"\n  name: "<<n->name<<"\n  raw[08,0c,10]: "<<n->raw_08<<", "<<n->raw_0c<<", "<<n->raw_10<<"\n  exception: "<<n->exception_path<<"\n  acoustic dsc: "<<n->acoustic_dsc_path<<"\n  modeinfo: "<<n->modeinfo_path<<"\n  0..254 symbol map: "<<(n->identity_symbol_map?"identity":"non-identity")<<"\n";

 const auto ch=nicolai::parse_channel_catalog_summary(b,l,*canaux);
 std::cout<<"\nchannel catalog summary:\n  valid: "<<(ch.valid?"yes":"no")<<"\n  data file: ";hx(ch.data_file_offset);std::cout<<"\n  serialized size: 0x"<<std::hex<<ch.serialized_size<<std::dec<<"\n  slot capacity: "<<ch.slot_capacity<<"\n  nonzero slot modes: "<<ch.nonzero_slot_modes<<"\n";

 const auto*dsc_obj=nicolai::find_legacy_file_object(init,"nbr16aci.dsc");dump_obj("\nNicolai acoustic DSC object",dsc_obj);if(!dsc_obj)return 1;
 const auto d=nicolai::parse_acoustic_descriptor(b,l,*dsc_obj);
 std::cout<<"\nNicolai acoustic descriptor:\n  valid: "<<(d.valid?"yes":"no")<<"\n  data file: ";hx(d.data_file_offset);std::cout<<"\n  sample rate: "<<d.sample_rate<<" Hz\n  header raw: "<<d.raw_00<<", "<<d.raw_04<<", "<<d.raw_08<<", "<<d.raw_0c<<", "<<d.raw_10<<", "<<d.raw_14<<", "<<d.raw_18<<"\n  rgl: "<<d.rgl_path<<"\n  axm: "<<d.axm_path<<"\n  seg: "<<d.seg_path<<"\n  ana: "<<d.ana_path<<"\n";

 const auto assets=nicolai::locate_nicolai_voice_assets(b,l,nicolai::scan_tagged_refs(b,l));if(!assets.valid){std::cerr<<"asset map failed: "<<assets.error<<"\n";return 1;}
 std::cout<<"\nM9 corrected acoustic byte ranges:\n  ANA data: [";hx(assets.analysis_data_offset);std::cout<<", ";hx(assets.analysis_data_end);std::cout<<") bytes="<<assets.analysis_data_size<<"\n  cmp16:    [";hx(assets.compressed_voice_offset);std::cout<<", ";hx(assets.compressed_voice_end);std::cout<<") bytes="<<assets.compressed_voice_size<<"\n  cmp16 prefix:";
 for(std::size_t i=0;i<16;++i)std::cout<<" "<<std::hex<<std::setw(2)<<std::setfill('0')<<(unsigned)b[assets.compressed_voice_offset+i];std::cout<<std::dec<<"\n";
 std::vector<std::uint8_t> ana(b.begin()+assets.analysis_data_offset,b.begin()+assets.analysis_data_end);
 const auto ai=nicolai::parse_ana_unit_index(ana,static_cast<std::uint32_t>(assets.compressed_voice_size));
 std::cout<<"  ANA units: "<<ai.records.size()<<" valid="<<(ai.valid?"yes":"no")<<" covered="<<ai.covered_bytes<<" trailing cmp16="<<ai.trailing_payload_bytes<<"\n";
 if(!cat.valid||cat.occupied!=50||n->index!=46||!ch.valid||d.sample_rate!=16000||!ai.valid)return 1;
 if(assets.compressed_voice_offset+16>b.size()||!std::all_of(b.begin()+assets.compressed_voice_offset,b.begin()+assets.compressed_voice_offset+16,[](std::uint8_t x){return x==0x55;}))return 1;
 std::cout<<"\nM9 chain recovered: voix.dsc -> Nicolai[46] -> nbr16aci.dsc -> RGL/AXM/SEG/ANA -> corrected cmp16 stream.\n";
 return 0;
}
