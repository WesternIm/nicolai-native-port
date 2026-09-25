#include "nicolai/legacy_prosody.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/voice_db.hpp"
#include <iostream>
#include <string>
#include <vector>

static void print_case(const std::string& text){
    auto fr=nicolai::russian_text_to_nicolai_phones(text);
    std::cout << "TEXT\t" << text;
    if(!fr.valid){ std::cout << "\tERROR\t" << fr.error << "\n"; return; }
    auto c=nicolai::legacy_physical_class_lattice_m27(fr);
    std::cout << "\tCLASS";
    for(auto x:c) std::cout << "\t" << x;
    std::cout << "\n";
}

int main(int argc,char**argv){
    if(argc<2){std::cerr<<"usage: nicolai_m27_probe nicolai16.dat\n";return 2;}
    auto db=nicolai::VoiceDb::load(argv[1]);
    auto phys=nicolai::parse_legacy_russian_physical(db.bytes(),db.metadata().edat);
    std::cout << "PHYSICAL\tvalid=" << (phys.valid?1:0) << "\trecords=" << phys.records.size() << "\n";
    if(phys.valid){
        const auto&r=phys.records[1];
        std::cout << "T_LAYOUT_CLASS1\tstartA=" << int(r.values[18])
                  << "\tstartB=" << int(r.values[19]) << "\tend=" << int(r.values[20])
                  << "\tterminal=" << int(r.values[20]+r.values[25])
                  << "," << int(r.values[20]+r.values[32])
                  << "," << int(r.values[20]+r.values[39]) << "\n";
        std::cout << "INTERP_EXAMPLE\t"
                  << nicolai::legacy_physical_interp_add_pc(15,10,-20,0,4) << "\t"
                  << nicolai::legacy_physical_interp_add_pc(15,10,-20,3,4) << "\n";
    }
    print_case("что ты делаешь?");
    print_case("привет, Николай!");
    print_case("я пришёл домой.");
    print_case("когда домой?");
    print_case("ты дома?");
    return 0;
}
