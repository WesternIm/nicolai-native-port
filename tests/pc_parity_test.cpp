#include "nicolai/pc_parity.hpp"
#include <cassert>
#include <iostream>

int main() {
    nicolai::Pcm16Mono p;
    p.sample_rate = 16000;
    p.samples = {0, 1000, -1000, 16000, -20000, 20000};
    nicolai::apply_pc_reference_output_gain(p);
    assert(p.samples[0] == 0);
    assert(p.samples[1] == 2000);
    assert(p.samples[2] == -2000);
    assert(p.samples[3] == 32000);
    assert(p.samples[4] == -32768);
    assert(p.samples[5] == 32767);
    std::cout << "pc_parity_test: PASSED\n";
}
