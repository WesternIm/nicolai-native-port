#include "nicolai/diphone_catalog.hpp"
#include "nicolai/edat.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>
static std::vector<std::uint8_t> rd(const char*p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void hx(std::uint64_t v){std::cout<<"0x"<<std::hex<<v<<std::dec;}
int main(int argc,char**argv){
 if(argc!=2){std::cerr<<"usage: nicolai_m10_probe nicolai16.dat\n";return 2;}
 const auto b=rd(argv[1]);const auto l=nicolai::parse_edat_layout(b);if(!l.valid){std::cerr<<"invalid EDAT\n";return 1;}
 const auto c=nicolai::parse_nicolai_diphone_catalog(b,l);if(!c.valid){std::cerr<<"catalog failed: "<<c.error<<"\n";return 1;}
 std::cout<<"Nicolai native-port M10 deterministic diphone graph\n\n";
 std::cout<<"phones: "<<c.phone_inventory.phones.size()<<"\n";
 std::cout<<"AXM: matrix="<<c.matrix_side<<"x"<<c.matrix_side<<" entries="<<c.matrix_entries<<" occupied="<<c.occupied_entries<<" unique_records="<<c.unique_axm_records<<" size="<<c.axm_serialized_size<<"\n";
 std::cout<<"SEG: offset=";hx(c.seg_data_file_offset);std::cout<<" size="<<c.seg_serialized_size<<" fully_partitioned=yes\n";
 std::cout<<"ANA: offset=";hx(c.ana_data_file_offset);std::cout<<" end=";hx(c.ana_data_end_file_offset);std::cout<<" size="<<c.ana_serialized_size<<" indexed="<<c.ana_indexed_bytes<<" trailing="<<c.ana_trailing_bytes<<"\n\n";
 std::cout<<"phone inventory:\n";
 for(std::size_t i=0;i<c.phone_inventory.phones.size();++i){std::cout<<std::setw(2)<<i<<":"<<c.phone_inventory.phones[i]<<((i+1)%8?"  ":"\n");}
 std::cout<<"\nfirst diphone units:\n";
 for(std::size_t i=0;i<std::min<std::size_t>(12,c.units.size());++i){
   const auto&u=c.units[i];std::cout<<std::setw(4)<<u.index<<"  "<<u.left_phone<<" -> "<<u.right_phone<<"  axm_rel=";hx(u.axm_record_relative);std::cout<<"  seg=["<<u.seg_offset<<","<<u.seg_offset+u.seg_size<<")  ana=["<<u.compressed_start<<","<<u.compressed_end<<") bytes="<<(u.compressed_end-u.compressed_start)<<" signed_end="<<u.signed_end<<" meta="<<u.metadata.size()<<"\n";
 }
 const auto*first=nicolai::find_diphone(c,"#","p");
 if(first){const auto raw=nicolai::extract_diphone_compressed_bytes(b,c,*first);std::cout<<"\n# -> p compressed unit: "<<raw.size()<<" bytes; first 24:";for(std::size_t i=0;i<std::min<std::size_t>(24,raw.size());++i)std::cout<<" "<<std::hex<<std::setw(2)<<std::setfill('0')<<(unsigned)raw[i];std::cout<<std::dec<<"\n";}
 std::cout<<"\nM10 invariant: AXM matrix cell -> exact SEG record -> exact ANA compressed slice.\n";
 return 0;
}
