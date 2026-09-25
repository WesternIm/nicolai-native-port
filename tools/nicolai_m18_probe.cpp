#include "nicolai/engine.hpp"
#include "nicolai/legacy_prosody.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"
#include "nicolai/wav_writer.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main(int argc,char**argv){
    if(argc<5){std::cerr<<"usage: nicolai_m18_probe nicolai16.dat exc_rus.txt abb_rus.txt out_dir\n";return 2;}
    const std::filesystem::path dbp=argv[1], excp=argv[2], abbp=argv[3], out=argv[4];
    std::filesystem::create_directories(out);
    auto db=nicolai::VoiceDb::load(dbp);
    auto stress=nicolai::load_exc_rus_cp1251(excp);
    auto exc=nicolai::load_exc_rus_replacements_cp1251(excp);
    auto abb=nicolai::load_abb_rus_cp1251(abbp);
    auto dur=nicolai::parse_legacy_russian_phone_durations(db.bytes(),db.metadata().edat);
    auto ws=nicolai::parse_legacy_russian_wordstr(db.bytes(),db.metadata().edat);

    std::cout<<"Nicolai native-port M18 PC-compat probe\n\n";
    std::cout<<"stress entries: "<<stress.entries<<"\n";
    std::cout<<"exception entries: "<<exc.entries<<" (multiword "<<exc.multiword_entries<<")\n";
    std::cout<<"abbreviation entries: "<<abb.entries<<"\n";
    std::cout<<"duration table: "<<(dur.valid?"yes":"no")<<" entries="<<dur.milliseconds.size()<<"\n";
    for(const char* p:{"a0","a1","a3","a4","m","p","sh","sc"}){
        auto it=dur.milliseconds.find(p);if(it!=dur.milliseconds.end())std::cout<<"  "<<p<<"="<<it->second<<" ms\n";
    }
    std::cout<<"wordstr: "<<(ws.valid?"yes":"no")<<" floats="<<ws.values.size()<<"\n";
    if(ws.valid){std::cout<<"  tail:";for(std::size_t i=30;i<ws.values.size();++i)std::cout<<" "<<ws.values[i];std::cout<<"\n";}

    nicolai::Engine eng(std::move(db),std::move(stress),std::move(abb),std::move(exc));
    const std::vector<std::pair<std::string,std::string>> cases={
        {"молоко","moloko"},
        {"привет","privet"},
        {"все будет хорошо","vse_budet_horosho"},
        {"USB 123","usb_123"},
        {"мама папа","mama_papa"}
    };
    for(const auto&c:cases){
        auto r=eng.synthesize(c.first);
        std::cout<<"text: "<<c.first<<" -> "<<nicolai::to_string(r.status)<<" samples="<<r.pcm.samples.size();
        if(r.status==nicolai::SynthStatus::Ok){auto fn=out/(c.second+".wav");nicolai::write_wav_pcm16_mono(fn,r.pcm);std::cout<<" wav="<<fn.string();}
        else std::cout<<" error="<<r.message;
        std::cout<<"\n";
    }
    return 0;
}
