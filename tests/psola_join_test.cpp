#include "nicolai/psola_join.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
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
    std::cout << "psola_join_test: PASSED corr=" << d.normalized_correlation << "\n";
}
