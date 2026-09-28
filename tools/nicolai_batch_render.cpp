#include "nicolai/voice_db.hpp"
#include "nicolai/diphone_catalog.hpp"
#include "nicolai/legacy_prosody.hpp"
#include "nicolai/russian_legacy.hpp"
#include "nicolai/russian_stress.hpp"
#include "nicolai/russian_frontend.hpp"
#include "nicolai/pc_parity.hpp"
#include "nicolai/wav_writer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc,char**argv){
 if(argc!=6){std::cerr<<"usage: nicolai_batch_render nicolai16.dat exc_rus.txt abb_rus.txt corpus.tsv outdir\n";return 2;}
 try{
  auto db=nicolai::VoiceDb::load(argv[1]);
  auto stress=nicolai::load_exc_rus_cp1251(argv[2]);
  auto exceptions=nicolai::load_exc_rus_replacements_cp1251(argv[2]);
  auto abbreviations=nicolai::load_abb_rus_cp1251(argv[3]);
  auto catalog=nicolai::parse_nicolai_diphone_catalog(db.bytes(),db.metadata().edat);
  auto duration=nicolai::parse_legacy_russian_phone_durations(db.bytes(),db.metadata().edat);
  auto wordstr=nicolai::parse_legacy_russian_wordstr(db.bytes(),db.metadata().edat);
  auto physical=nicolai::parse_legacy_russian_physical(db.bytes(),db.metadata().edat);
  if(wordstr.valid){ std::cerr<<"WORDSTR"; for(auto v:wordstr.values) std::cerr<<"\t"<<v; std::cerr<<"\n"; }
  if(physical.valid) std::cerr<<"PHYSICAL\t"<<physical.records.size()<<"x42\n";
  if(!catalog.valid||!duration.valid){std::cerr<<"db parse failed\n";return 1;}
  std::filesystem::create_directories(argv[5]);
  std::ifstream f(argv[4]); std::string line;
  while(std::getline(f,line)){
   auto tab=line.find('\t'); if(tab==std::string::npos) continue;
   auto id=line.substr(0,tab), text=line.substr(tab+1);
   auto norm=nicolai::normalize_russian_legacy_text(text,&abbreviations,&exceptions);
   if(!norm.valid){std::cerr<<id<<" norm "<<norm.error<<"\n";continue;}
   nicolai::RussianFrontendOptions fo;fo.stress_dictionary=&stress;
   auto fr=nicolai::russian_text_to_nicolai_phones(norm.normalized_utf8,fo);
   if(!fr.valid){std::cerr<<id<<" front "<<fr.error<<"\n";continue;}
   const auto rh=nicolai::analyze_legacy_recovered_neighbor_rules(fr.phones);
   std::cout<<"R\t"<<id<<"\tR="<<rh.r_adjacent<<"\tJI="<<rh.j_to_i
            <<"\tUJU_L="<<rh.u_j_u_left_u<<"\tUJU_J="<<rh.u_j_u_center_j
            <<"\tUJU_R="<<rh.u_j_u_right_u<<"\n";
   nicolai::LegacyTimingPolicy policy;
   if(const char* v=std::getenv("NICOLAI_STATEFUL_TDS")) policy.use_stateful_tds_m34=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_SHARED_PHONE_DURATION")) policy.shared_phone_duration_m34=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_M36_TRANSITION_EXECUTOR")) if(std::atoi(v)!=0){
       // The M36 A/B path deliberately reuses the established stateful caller
       // contract and shared-phone timing coefficients. resynthesize_stateful_m34
       // dispatches to the guarded M36 transition executor when this env flag is set.
       policy.use_stateful_tds_m34=true;
       policy.shared_phone_duration_m34=true;
   }
   if(const char* v=std::getenv("NICOLAI_M36_CHAIN_EXECUTOR")) if(std::atoi(v)!=0){
       policy.use_stateful_tds_m34=true;
       policy.shared_phone_duration_m34=true;
   }
   if(const char* v=std::getenv("NICOLAI_PC_SEG_TIMELINE")) policy.use_pc_seg_timeline=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_SEARCH_JOIN_PHASE")) policy.search_join_phase=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_PHONE_SCALE")) policy.phone_duration_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_UTTERANCE_BOUNDARY_SCALE")) policy.utterance_boundary_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_WORD_BOUNDARY_SCALE")) policy.word_boundary_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_M38_BOUNDARY_SPEECH_SHARE")) policy.word_boundary_speech_share_m38=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_COMMA_BOUNDARY_SCALE")) policy.comma_boundary_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_WORDSTR_CONTOUR_STRENGTH")) policy.wordstr_contour_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHONE_SIDE_STRENGTH")) policy.phone_side_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHONE_SIDE_RATIO_LIMIT")) policy.phone_side_ratio_limit=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_CC_SCALE")) policy.cc_duration_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_CV_SCALE")) policy.cv_duration_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_VC_SCALE")) policy.vc_duration_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_VV_SCALE")) policy.vv_duration_scale=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_LEGACY_PITCH_STRENGTH")) policy.legacy_pitch_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_LEGACY_PITCH_BASE_HZ")) policy.legacy_pitch_base_hz=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_LEGACY_PITCH_SOURCE_RESIDUAL")) policy.legacy_pitch_source_residual=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_DECLINATION_STRENGTH")) policy.pitch_declination_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_DECLINATION_START")) policy.pitch_declination_start=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_DECLINATION_MID")) policy.pitch_declination_mid=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_DECLINATION_END")) policy.pitch_declination_end=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_STRESSED_VOWEL_BOOST")) policy.pitch_stressed_vowel_boost=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_ANCHOR_STRENGTH")) policy.pitch_anchor_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_ANCHOR_START_PERCENT")) policy.pitch_anchor_start_percent=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_ANCHOR_END_PERCENT")) policy.pitch_anchor_end_percent=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_ANCHOR_STRESS_LIFT_PERCENT")) policy.pitch_anchor_stress_lift_percent=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PITCH_ANCHOR_LONG_START_BOOST_PERCENT")) policy.pitch_anchor_long_phrase_start_boost_percent=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_PITCH_STRENGTH")) policy.physical_pitch_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_TERMINAL_PITCH_STRENGTH")) policy.physical_terminal_pitch_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_EMPTY_MARKER_PITCH_STRENGTH")) policy.physical_empty_marker_pitch_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_SINGLE_MARKER_PITCH_STRENGTH")) policy.physical_single_marker_pitch_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_LENGTH_STRENGTH")) policy.physical_length_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_ENERGY_STRENGTH")) policy.physical_energy_strength=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_TERMINAL_CLASS")) policy.physical_terminal_class_override=std::atoi(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_INTERNAL_CLASS")) policy.physical_internal_class=std::atoi(v);
   if(const char* v=std::getenv("NICOLAI_PHYSICAL_NONWH_QUESTION_CLASS")) policy.physical_nonwh_question_class=std::atoi(v);
   if(const char* v=std::getenv("NICOLAI_DISABLE_PHYSICAL_PUNCT_CLASSES")) policy.physical_use_punctuation_classes=(std::atoi(v)==0);
   if(const char* v=std::getenv("NICOLAI_DISABLE_WORDSTR_CONTOUR")) policy.enable_wordstr_position_contour=(std::atoi(v)==0);
   double pitch_scale=1.0;
   if(const char* v=std::getenv("NICOLAI_PITCH_SCALE")) pitch_scale=std::atof(v);
   auto a=nicolai::synthesize_diphone_chain_legacy_duration(db.bytes(),catalog,fr.phones,duration,&wordstr,pitch_scale,16000,fr.boundaries,policy,physical.valid?&physical:nullptr,&fr);
   if(!a.valid){std::cerr<<id<<" synth "<<a.error<<"\n";continue;}
   if(policy.use_stateful_tds_m34)
       std::cout<<"TDS\t"<<id<<"\t"<<a.tds_intervals_m34<<"\t"<<a.tds_grains_m34<<"\t"<<a.tds_dropped_m34<<"\t"<<a.tds_final_carry_m34<<"\n";
   if(policy.use_stateful_tds_m34)
       std::cout<<"CLOCK\t"<<id<<"\t"<<a.pcm.samples.size()<<"\t"<<a.tds_target_samples_m35
           <<"\t"<<a.tds_budget_samples_m35<<"\t"<<a.tds_emitted_samples_m35
           <<"\t"<<a.tds_clamped_records_m35<<"\n";
   if(policy.use_stateful_tds_m34)
       std::cout<<"M36\t"<<id<<"\t"<<a.m36_initial_paths<<"\t"<<a.m36_cross_paths
           <<"\t"<<a.m36_terminal_flushes<<"\t"<<a.m36_fallbacks<<"\n";
   nicolai::apply_pc_reference_output_gain(a.pcm);
   nicolai::append_legacy_pc_terminal_silence(a.pcm,wordstr,16000);
   nicolai::write_wav_pcm16_mono(std::filesystem::path(argv[5])/(id+".wav"),a.pcm);
   std::cout<<"P\t"<<id<<"\t"<<a.pcm.samples.size()<<"\t"<<fr.phones.size(); for(const auto&ph:fr.phones) std::cout<<"\t"<<ph; std::cout<<"\n"; for(const auto&t:a.timings) std::cout<<"D\t"<<id<<"\t"<<t.label<<"\t"<<t.source_ms<<"\t"<<t.target_ms<<"\t"<<t.duration_scale<<"\t"<<t.left_duration_scale<<"\t"<<t.right_duration_scale<<"\t"<<t.left_energy_gain<<"\t"<<t.right_energy_gain<<"\n";
   for(const auto&j:a.joins) std::cout<<"J\t"<<id<<"\t"<<j.shared_phone_index<<"\t"<<j.shared_phone<<"\t"<<j.center_sample<<"\t"<<j.overlap_samples<<"\t"<<j.left_trim<<"\t"<<j.right_trim<<"\t"<<j.normalized_correlation<<"\n";
  }
  return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}
}
