#include "nicolai/legacy_prosody.hpp"
#include <cassert>
#include <iostream>
int main(){
    nicolai::LegacyPhoneDurationProfile p;
    assert(!p.valid);
    nicolai::LegacyWordProsodyProfile w;
    assert(!w.valid);

    nicolai::LegacyWordProsodyProfile framing;
    framing.valid = true;
    framing.values.assign(35, 0.0f);
    framing.values[34] = 150.0f;
    assert(nicolai::legacy_pc_terminal_silence_samples(framing, 16000) == 4800);
    nicolai::Pcm16Mono pcm;
    pcm.sample_rate = 16000;
    pcm.samples = {1, -2, 3};
    nicolai::append_legacy_pc_terminal_silence(pcm, framing, 16000);
    assert(pcm.samples.size() == 4803);
    assert(pcm.samples[0] == 1 && pcm.samples[1] == -2 && pcm.samples[2] == 3);
    assert(pcm.samples.back() == 0);

    assert(nicolai::legacy_recovered_symbol("i0") == nicolai::LegacyRecoveredSymbol::I);
    assert(nicolai::legacy_recovered_symbol("j") == nicolai::LegacyRecoveredSymbol::J);
    assert(nicolai::legacy_recovered_symbol("r'") == nicolai::LegacyRecoveredSymbol::R);
    assert(nicolai::legacy_recovered_symbol("U4") == nicolai::LegacyRecoveredSymbol::U);
    const std::vector<std::string> uju={"#","u0","j","u0","#"};
    const auto hits=nicolai::analyze_legacy_recovered_neighbor_rules(uju);
    assert(hits.u_j_u_left_u==1);
    assert(hits.u_j_u_center_j==1);
    assert(hits.u_j_u_right_u==1);

    nicolai::LegacyWordProsodyProfile pitch_wordstr;
    pitch_wordstr.valid = true;
    pitch_wordstr.values.assign(35, 1.0f);
    pitch_wordstr.values[28] = 0.95f;
    nicolai::LegacyTimingPolicy pitch_policy;
    const double target = nicolai::legacy_pitch_target_f0_m23(100.0, &pitch_wordstr, pitch_policy);
    // 83 + (100-83) * (5/19) * 0.95 = 87.25 Hz.
    assert(target > 87.249 && target < 87.251);
    pitch_policy.legacy_pitch_strength = 0.0;
    assert(nicolai::legacy_pitch_target_f0_m23(100.0, &pitch_wordstr, pitch_policy) == 100.0);

    nicolai::LegacyPhysicalProsodyRecord phys{};
    phys.values[18]=10;
    const int g0[7]={15,15,10,15,-10,-15,-20};
    const int g1[7]={15,13,10,13,-10,-15,-20};
    const int g2[7]={15,10,10,10,-10,-15,-20};
    for(int i=0;i<7;++i){phys.values[21+i]=g0[i];phys.values[28+i]=g1[i];phys.values[35+i]=g2[i];}
    const auto t0=nicolai::legacy_physical_pitch_triplet(phys,0.0,18);
    assert(t0[0]==25.0 && t0[1]==25.0 && t0[2]==25.0);
    const auto t1=nicolai::legacy_physical_pitch_triplet(phys,1.0,18);
    assert(t1[0]==-10.0 && t1[1]==-10.0 && t1[2]==-10.0);
    const auto tm=nicolai::legacy_physical_pitch_triplet(phys,0.5,18);
    assert(tm[0]==25.0 && tm[1]==23.0 && tm[2]==20.0);

    // M27 exact 0x10215d30/0x10215d90 arithmetic.
    assert(nicolai::legacy_physical_interp_add_pc(15, 10, -20, 0, 4) == 25);
    assert(nicolai::legacy_physical_interp_add_pc(15, 10, -20, 3, 4) == -5);
    assert(nicolai::legacy_physical_interp_add_pc(-5, -10, 20, 1, 3) == 0);
    assert(nicolai::legacy_physical_interp_pc(10, -20, 2, 4) == -10);

    // M28 exact t-writer empty-marker branches and base-slot threshold.
    phys.values[19]=20;
    phys.values[20]=-20;
    // First non-final <> uses k0 with the selected base directly.
    const auto em0=nicolai::legacy_physical_empty_marker_triplet_pc(phys,true,false,18,1,6);
    assert(em0[0]==25.0 && em0[1]==25.0 && em0[2]==25.0);
    // Subsequent <> uses k2 and 0x10215d30 interpolation.
    const auto em2=nicolai::legacy_physical_empty_marker_triplet_pc(phys,false,false,18,3,6);
    assert(em2[0]==2.0 && em2[1]==2.0 && em2[2]==2.0);
    // Final global <> uses rec[20] + k4, independent of base 18/19.
    const auto em4=nicolai::legacy_physical_empty_marker_triplet_pc(phys,false,true,19,5,6);
    assert(em4[0]==-30.0 && em4[1]==-30.0 && em4[2]==-30.0);
    assert(nicolai::legacy_physical_base_slot_pc(1)==18);
    assert(nicolai::legacy_physical_base_slot_pc(2)==18);
    assert(nicolai::legacy_physical_base_slot_pc(3)==19);

    // M29 exact ordinary single-< path: k1 is live; k3 is unreachable in
    // this Nicolai build.  For class 1 with wordstr[33]=20, <=2 marked words
    // gives start=-10+record[18]=0 and end=record[20]=-20.
    const auto sm0=nicolai::legacy_physical_single_marker_triplet_pc(phys,1,0,4,2.0,20.0);
    assert(sm0[0]==15.0 && sm0[1]==13.0 && sm0[2]==10.0);
    const auto sm3=nicolai::legacy_physical_single_marker_triplet_pc(phys,1,3,4,2.0,20.0);
    assert(sm3[0]==-5.0 && sm3[1]==-7.0 && sm3[2]==-10.0);
    // >2 marker-bearing words selects byte 19: start=-10+20=10.
    const auto sm_many=nicolai::legacy_physical_single_marker_triplet_pc(phys,3,0,4,2.0,20.0);
    assert(sm_many[0]==25.0 && sm_many[1]==23.0 && sm_many[2]==20.0);



    // M31 exact [l%d] / [e%d] physical.int authoring arithmetic.
    // Class-1 values from the original Nicolai physical.int image.
    const int lp0[5]={-30,0,20,30,10};
    const int lp1[5]={-8,0,17,26,8};
    const int ep[5]={70,80,90,100,30};
    for(int i=0;i<5;++i){
        phys.values[3+i]=lp0[i];
        phys.values[8+i]=lp1[i];
        phys.values[13+i]=ep[i];
    }
    // <=wordstr[24] selects bytes 3..7; final marker also adds profile[4].
    assert(nicolai::legacy_physical_length_percent_pc(phys,2,1,false,false,false)==30);
    assert(nicolai::legacy_physical_length_percent_pc(phys,2,2,false,false,false)==40);
    // >2 selects bytes 8..12.
    assert(nicolai::legacy_physical_length_percent_pc(phys,3,1,false,false,false)==26);
    assert(nicolai::legacy_physical_length_percent_pc(phys,3,3,false,false,false)==34);
    assert(nicolai::legacy_physical_length_percent_pc(phys,3,3,true,false,false)==0);
    assert(nicolai::legacy_physical_length_percent_pc(phys,3,1,false,true,false)==17);
    assert(nicolai::legacy_physical_length_percent_pc(phys,3,1,false,true,true)==0);

    // Energy q3=100 for an ordinary marker and declines multiplicatively to
    // terminal q4=30 across a three-marker span: 100,65,30.
    assert(nicolai::legacy_physical_energy_percent_pc(phys,1,1,false,false,false)==100);
    assert(nicolai::legacy_physical_energy_percent_pc(phys,3,1,false,false,false)==100);
    assert(nicolai::legacy_physical_energy_percent_pc(phys,3,2,false,false,false)==65);
    assert(nicolai::legacy_physical_energy_percent_pc(phys,3,3,false,false,false)==30);
    assert(nicolai::legacy_physical_energy_percent_pc(phys,3,2,true,false,false)==46);

    // M31 lower-level dynamic authoring splitter recovered from 0x10216c70.
    std::vector<nicolai::LegacyAuthoringSplitItemM31> split_items(7);
    for(auto& x:split_items){x.raw_separator=' ';x.code_empty=true;}
    const auto minor=nicolai::legacy_authoring_split_decision_m31(
        split_items,0,6,' ',5,false);
    assert(minor.split && minor.index==3 && minor.effective_count==7);
    for(auto& x:split_items) x.raw_separator='_';
    const auto hard=nicolai::legacy_authoring_split_decision_m31(
        split_items,0,6,'_',6,true);
    assert(hard.split && hard.index==3 && hard.effective_count==7);
    split_items[3].code_empty=false;
    const auto near=nicolai::legacy_authoring_split_decision_m31(
        split_items,0,6,'_',6,true);
    assert(near.split && near.index==2);

    // M27 recovered 0x10215de0 class lattice.
    const auto q=nicolai::russian_text_to_nicolai_phones("что ты делаешь?");
    assert(q.valid);
    const auto qc=nicolai::legacy_physical_class_lattice_m27(q);
    assert(qc.size()==3 && qc[0]==0 && qc[1]==0 && qc[2]==3);
    const auto ex=nicolai::russian_text_to_nicolai_phones("привет, Николай!");
    assert(ex.valid);
    const auto exc=nicolai::legacy_physical_class_lattice_m27(ex);
    assert(exc.size()==2 && exc[0]==12 && exc[1]==4);
    const auto dec=nicolai::russian_text_to_nicolai_phones("я пришёл домой.");
    assert(dec.valid);
    const auto decc=nicolai::legacy_physical_class_lattice_m27(dec);
    assert(decc.size()==3 && decc[0]==0 && decc[1]==0 && decc[2]==1);

    // M30: 0x10214f40 authors one physical.int row per legacy "(/)" span,
    // selected from the span-ending class.  Internal words do not start fresh
    // physical rows.  The existing one-sentence probes therefore form one
    // authoring span even when punctuation such as a comma occurs inside it.
    const auto qsp=nicolai::legacy_authoring_spans_m30(q);
    assert(qsp.size()==1 && qsp[0].first_word==0 && qsp[0].last_word==2);
    assert(qsp[0].physical_class==3 && qsp[0].marker_word_count==3 && qsp[0].vowel_count>=3);
    const auto exsp=nicolai::legacy_authoring_spans_m30(ex);
    assert(exsp.size()==1 && exsp[0].first_word==0 && exsp[0].last_word==1);
    assert(exsp[0].physical_class==4 && exsp[0].marker_word_count==2);
    const auto two=nicolai::russian_text_to_nicolai_phones("мама. папа.");
    assert(two.valid);
    const auto twosp=nicolai::legacy_authoring_spans_m30(two);
    assert(twosp.size()==2 && twosp[0].first_word==0 && twosp[0].last_word==0);
    assert(twosp[1].first_word==1 && twosp[1].last_word==1);

    std::cout<<"legacy_prosody_test: PASSED\n";
    return 0;
}
