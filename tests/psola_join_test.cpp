#include "nicolai/psola_join.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <stdexcept>

int main() try {
    using namespace nicolai;
    DiphoneUnit u;
    u.metadata = {2, 5, 2, 5, 150, 152, 154, 156, 158};
    const auto h = infer_pitch_period_hint(u);
    assert(h.valid);
    assert(h.left_period == 152);
    assert(h.right_period == 156);

    Pcm16Mono a, b;
    a.sample_rate = b.sample_rate = 16000;
    for (int i = 0; i < 800; ++i) {
        a.samples.push_back(static_cast<std::int16_t>(8000 * std::sin(2.0 * 3.141592653589793 * i / 160.0)));
        b.samples.push_back(static_cast<std::int16_t>(8000 * std::sin(2.0 * 3.141592653589793 * (i + 17) / 160.0)));
    }
    OlaJoinDiagnostics d;
    auto c = hann_ola_join(a, b, 160, 160, &d);
    assert(d.valid);
    assert(d.overlap_samples == 160);
    assert(!c.samples.empty());
    assert(c.samples.size() < a.samples.size() + b.samples.size());
    assert(d.normalized_correlation > 0.8);
    OlaJoinDiagnostics fixed;
    auto fixed_pcm = hann_ola_join(a,b,160,160,&fixed,false);
    assert(fixed.valid && fixed.left_trim==0 && fixed.right_trim==0);
    assert(fixed_pcm.samples.size()==a.samples.size()+b.samples.size()-fixed.overlap_samples);
    assert(hann_ola_join(a,b,160,160,nullptr,true).samples==c.samples);
    auto require = [](bool ok) { if (!ok) throw std::runtime_error("M41 join preservation contract failed"); };
    require(hann_ola_join(a,b,160,160,nullptr,true,true).samples==c.samples);
    // Correlation can discard a short unvoiced release before an otherwise
    // perfectly matching noise block. Exercise the actual PCM, not just flags.
    Pcm16Mono noise_left, noise_right;
    noise_left.sample_rate = noise_right.sample_rate = 16000;
    noise_left.samples.resize(320, 0);
    noise_right.samples.resize(320, 0);
    std::uint32_t rng = 1;
    for (std::size_t i=0;i<80;++i) {
        rng = rng * 1664525u + 1013904223u;
        noise_left.samples[240+i] = static_cast<std::int16_t>(static_cast<int>(rng % 4001u)-2000);
        noise_right.samples[20+i] = noise_left.samples[240+i];
    }
    noise_right.samples[10] = 20000;
    OlaJoinDiagnostics old_noise, protected_noise;
    const auto old_pcm = hann_ola_join(noise_left,noise_right,0,0,&old_noise);
    const auto protected_pcm = hann_ola_join(noise_left,noise_right,0,0,&protected_noise,true,true);
    require(old_noise.right_trim > 10);
    require(*std::max_element(old_pcm.samples.begin(),old_pcm.samples.end()) < 10000);
    require(protected_noise.left_trim==0 && protected_noise.right_trim==0);
    require(protected_noise.protected_transient_m41);
    require(protected_noise.overlap_samples==16);
    require(*std::max_element(protected_pcm.samples.begin(),protected_pcm.samples.end()) > 12000);
    require(hann_ola_join(noise_left,noise_right,160,0,nullptr,true,true).samples==protected_pcm.samples);
    // The same deletion can occur on the left tail. Mirror the fixture so
    // the release is in the part discarded by left_trim.
    auto mirrored_left=noise_right, mirrored_right=noise_left;
    std::reverse(mirrored_left.samples.begin(),mirrored_left.samples.end());
    std::reverse(mirrored_right.samples.begin(),mirrored_right.samples.end());
    OlaJoinDiagnostics old_left, protected_left;
    const auto old_left_pcm=hann_ola_join(mirrored_left,mirrored_right,0,0,&old_left);
    const auto kept_left_pcm=hann_ola_join(mirrored_left,mirrored_right,0,0,&protected_left,true,true);
    require(old_left.left_trim>10);
    require(*std::max_element(old_left_pcm.samples.begin(),old_left_pcm.samples.end())<10000);
    require(protected_left.protected_transient_m41);
    require(*std::max_element(kept_left_pcm.samples.begin(),kept_left_pcm.samples.end())>12000);
    // Do not shorten the overlap for ordinary stationary fricative noise.
    for(std::size_t i=0;i<noise_left.samples.size();++i) {
        rng=rng*1664525u+1013904223u;
        noise_left.samples[i]=static_cast<std::int16_t>(static_cast<int>(rng%4001u)-2000);
        rng=rng*1664525u+1013904223u;
        noise_right.samples[i]=static_cast<std::int16_t>(static_cast<int>(rng%4001u)-2000);
    }
    OlaJoinDiagnostics stationary;
    require(hann_ola_join(noise_left,noise_right,0,0,&stationary,true,true).samples==
        hann_ola_join(noise_left,noise_right,0,0).samples);
    require(!stationary.protected_transient_m41);
    std::cout << "psola_join_test: PASSED corr=" << d.normalized_correlation << "\n";
} catch(const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
