#include "nicolai/hybrid_psola.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    nicolai::DiphoneUnit u;
    u.metadata = {-2,17,10,-9,8,-104,105,149,171,172,198,200,220};
    const auto s = nicolai::parse_seg_schedule_m15(u);
    assert(s.valid);
    const auto l = nicolai::layout_seg_runs_m15(s, 2994);
    assert(l.valid);

    nicolai::Pcm16Mono p;
    p.sample_rate = 16000;
    p.samples.resize(2994);
    for (std::size_t i=0;i<p.samples.size();++i) {
        // deterministic unvoiced-ish first region, voiced-ish second region
        if (i < l.runs[0].source_end)
            p.samples[i] = static_cast<std::int16_t>(((i*1103515245u + 12345u)>>16) % 8000 - 4000);
        else
            p.samples[i] = static_cast<std::int16_t>(9000.0 * std::sin(2.0*3.141592653589793*i/170.0));
    }

    nicolai::M15UnitDiagnostics d;
    auto q = nicolai::resynthesize_seg_m15(p, s, l, {1.15,1.20}, &d);
    assert(!q.samples.empty());
    assert(d.runs.size() == 2);
    assert(d.runs[0].used_unvoiced_stretch);
    assert(d.runs[1].used_td_psola);
    assert(d.runs[1].signed_period_reset);
    assert(d.internal_joins.size() == 1);
    std::cout << "hybrid_psola_test: PASSED output=" << q.samples.size() << "\n";
}
