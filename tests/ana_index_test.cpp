#include "nicolai/ana_index.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {
void put_i32(std::vector<std::uint8_t>& v, std::int32_t x) {
    const auto u = static_cast<std::uint32_t>(x);
    for (int i = 0; i < 4; ++i) v.push_back(static_cast<std::uint8_t>((u >> (8*i)) & 0xff));
}
void put_i16(std::vector<std::uint8_t>& v, std::int16_t x) {
    const auto u = static_cast<std::uint16_t>(x);
    v.push_back(static_cast<std::uint8_t>(u & 0xff));
    v.push_back(static_cast<std::uint8_t>((u >> 8) & 0xff));
}
}

int main() {
    std::vector<std::uint8_t> ana;
    put_i32(ana, 0); put_i32(ana, -10); put_i16(ana, -2); put_i16(ana, 3);
    put_i32(ana, 10); put_i32(ana, 25); put_i16(ana, 2); put_i16(ana, 4); put_i16(ana, 99);
    put_i32(ana, 25); put_i32(ana, 40); put_i16(ana, 2); // terminal

    const auto idx = nicolai::parse_ana_unit_index(ana, 42, 64);
    if (!idx.valid) {
        std::cerr << idx.error << "\n";
        return 1;
    }
    if (idx.records.size() != 3 || idx.covered_bytes != 40 || idx.trailing_payload_bytes != 2) return 2;
    if (idx.records[0].record_size != 12 || idx.records[1].record_size != 14 || idx.records[2].record_size != 10) return 3;
    if (idx.records[0].signed_end != -10 || idx.records[1].compressed_start != 10) return 4;

    std::vector<std::uint8_t> voice(42);
    for (std::size_t i = 0; i < voice.size(); ++i) voice[i] = static_cast<std::uint8_t>(i);
    const auto unit1 = nicolai::extract_compressed_unit(voice, idx.records[1]);
    if (unit1.size() != 15 || unit1.front() != 10 || unit1.back() != 24) return 5;

    std::cout << "ana_index_test: PASSED\n";
    return 0;
}
