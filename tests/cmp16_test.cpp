#include "nicolai/cmp16.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const char* manifest =
        "qmlt.txt qnf.txt qrms.txt hmlt.txt hrms.txt qva.txt qvb.txt noise.txt seq.txt "
        "1 2 3 4 5 6 7 8 9 10";
    const auto m = nicolai::parse_cmp16_manifest(manifest);
    assert(m.valid);
    assert(m.qmlt == "qmlt.txt");
    assert(m.qnf == "qnf.txt");
    assert(m.noise == "noise.txt");
    assert(m.sequence == "seq.txt");
    assert(m.scalars[0] == 1 && m.scalars[9] == 10);

    // Two rows. Row 0 has 3 centroids + 2 thresholds; row 1 has one centroid.
    const auto q = nicolai::parse_qmlt("2 3 0.5 -0.5 1.25 0.0 0.5 1 2.0");
    assert(q.valid);
    assert(q.rows.size() == 2);
    assert(q.rows[0].centroids_q13[0] == 4096);
    assert(q.rows[0].centroids_q13[1] == -4096);
    assert(q.rows[0].centroids_q13[2] == 10240);
    assert(q.rows[0].thresholds.size() == 2);
    assert(q.rows[1].centroids_q13[0] == 16384);

    const auto r = nicolai::parse_qrms("3 0 20 40");
    assert(r.valid);
    assert(r.reconstructed.size() == 3);
    assert(r.reconstructed[0] == 4);    // 4 * 10^(0/20)
    assert(r.reconstructed[1] == 40);   // 4 * 10^(20/20)
    assert(r.reconstructed[2] == 400);  // 4 * 10^(40/20)

    const auto& specs = nicolai::cmp16_resource_specs();
    assert(specs.size() == 9);
    assert(specs[0].legacy_parser_va == 0x1010AD90u);

    std::cout << "cmp16_test: PASSED\n";
    return 0;
}
