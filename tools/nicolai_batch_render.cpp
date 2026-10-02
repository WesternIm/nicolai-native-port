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
  nicolai::RussianLexiconM44 lexicon;
  const char* ending=std::getenv("NICOLAI_M48_LEXICON_STRESS");
  const char* newer=std::getenv("NICOLAI_M47_LEXICON_STRESS");
  const char* older=std::getenv("NICOLAI_M44_LEXICON_STRESS");
  const bool m47=newer && std::atoi(newer)!=0;
  const bool m48=ending && std::atoi(ending)!=0;
  if(m48 || m47 || (older && std::atoi(older)!=0)) {
      lexicon=nicolai::parse_russian_lexicon_m44(db.bytes(),db.metadata().edat);
      if(!lexicon.valid) throw std::runtime_error(lexicon.error);
      std::cerr<<"LEXICON\t"<<lexicon.blocks<<"\t"<<lexicon.records<<"\n";
  }
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
   if(lexicon.valid) fo.lexicon_m44=&lexicon;
   fo.enable_lexicon_stress_m47=m47;
   fo.enable_lexicon_stress_m48=m48;
   if(const char* v=std::getenv("NICOLAI_M43_FIXED_IKA_STRESS")) fo.enable_fixed_ika_stress_m43=std::atoi(v)!=0;
   auto fr=nicolai::russian_text_to_nicolai_phones(norm.normalized_utf8,fo);
   if(!fr.valid){std::cerr<<id<<" front "<<fr.error<<"\n";continue;}
   for(std::size_t wi=0;wi<fr.words.size();++wi) {
       const auto& w=fr.words[wi];
       std::cout<<"W\t"<<id<<"\t"<<wi<<"\t"<<w.stress_vowel_index<<"\t"<<w.stress_source<<"\n";
   }
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
   if(const char* v=std::getenv("NICOLAI_M40_BLEND_UNCOVERED")) policy.blend_uncovered_edges_m40=std::atoi(v)!=0;
   policy.audit_transients_m40=std::getenv("NICOLAI_AUDIT_TRANSIENTS")!=nullptr;
   if(const char* v=std::getenv("NICOLAI_M41_PRESERVE_RUNS")) policy.preserve_unvoiced_runs_m41=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_M41_PRESERVE_JOINS")) policy.preserve_unvoiced_joins_m41=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_M41_INTERPOLATE_UNCOVERED")) policy.interpolate_uncovered_m41=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_M42_JOIN_PERIOD_CONTINUITY")) policy.join_period_continuity_m42=std::atof(v);
   if(const char* v=std::getenv("NICOLAI_M42_LOCAL_JOIN_PITCH")) policy.local_join_pitch_m42=std::atoi(v)!=0;
   if(const char* v=std::getenv("NICOLAI_M43_WORD_RHYTHM")) policy.word_rhythm_strength_m43=std::atof(v);
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
   std::cout<<"M41\t"<<id<<"\t"<<a.protected_internal_m41<<"\t"<<a.protected_external_m41<<"\n";
   if(policy.join_period_continuity_m42>0.0) std::cout<<"M42\t"<<id<<"\t"<<a.reconciled_pitch_joins_m42<<"\n";
   for(const auto& w:a.word_budgets_m43) std::cout<<"WT\t"<<id<<"\t"<<w.first_phone<<"\t"<<w.last_phone
       <<"\t"<<w.baseline_samples<<"\t"<<w.trial_samples<<"\t"<<w.effective_strength<<"\n";
   nicolai::append_legacy_pc_terminal_silence(a.pcm,wordstr,16000);
   nicolai::write_wav_pcm16_mono(std::filesystem::path(argv[5])/(id+".wav"),a.pcm);
   std::cout<<"P\t"<<id<<"\t"<<a.pcm.samples.size()<<"\t"<<fr.phones.size(); for(const auto&ph:fr.phones) std::cout<<"\t"<<ph; std::cout<<"\n"; for(const auto&t:a.timings) std::cout<<"D\t"<<id<<"\t"<<t.label<<"\t"<<t.source_ms<<"\t"<<t.target_ms<<"\t"<<t.duration_scale<<"\t"<<t.left_duration_scale<<"\t"<<t.right_duration_scale<<"\t"<<t.left_energy_gain<<"\t"<<t.right_energy_gain<<"\n";
   for(const auto&j:a.joins) std::cout<<"J\t"<<id<<"\t"<<j.shared_phone_index<<"\t"<<j.shared_phone<<"\t"<<j.center_sample<<"\t"<<j.overlap_samples<<"\t"<<j.left_trim<<"\t"<<j.right_trim<<"\t"<<j.normalized_correlation<<"\n";
   if(std::getenv("NICOLAI_AUDIT_JOIN_PERIODS"))
       for(const auto&j:a.joins) std::cout<<"JP\t"<<id<<"\t"<<j.shared_phone_index<<"\t"<<j.shared_phone
           <<"\t"<<j.left_target_period_hint_m42<<"\t"<<j.right_target_period_hint_m42<<"\n";
   if(std::getenv("NICOLAI_AUDIT_TRANSIENTS"))
       for(const auto&u:a.unit_transients){
           std::cout<<"U\t"<<id<<"\t"<<u.label<<"\t"<<u.source_max_step<<"\t"<<u.source_max_step_at
                    <<"\t"<<u.rendered_max_step<<"\t"<<u.rendered_max_step_at<<"\t"<<u.internal_joins;
           for(std::size_t k=0;k<u.run_output_samples.size();++k)
               std::cout<<"\trun="<<u.run_output_samples[k]<<","<<(u.run_voiced[k]?"v":"u")
                        <<",uncovered="<<u.run_uncovered_samples[k]
                        <<",step="<<u.run_psola_max_steps[k]<<"@"<<u.run_psola_max_step_at[k]
                        <<",weights="<<u.run_psola_weight_before[k]<<"/"<<u.run_psola_weight_after[k];
           for(const auto n:u.internal_join_centers) std::cout<<"\tjoin="<<n;
           std::cout<<"\n";
       }
  }
  return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}
}
