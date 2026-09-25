#include "nicolai/g711.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    using nicolai::decode_g711_alaw_byte;
    assert(decode_g711_alaw_byte(0xD5) == 8);
    assert(decode_g711_alaw_byte(0x55) == -8);
    assert(decode_g711_alaw_byte(0x80) == 5504);
    assert(decode_g711_alaw_byte(0x00) == -5504);
    assert(decode_g711_alaw_byte(0xFF) == 848);
    assert(decode_g711_alaw_byte(0x7F) == -848);

    const std::vector<std::uint8_t> encoded{0x55, 0xD5, 0x80, 0x00};
    const auto pcm = nicolai::decode_g711_alaw_pcm(encoded, 16000);
    assert(pcm.sample_rate == 16000);
    assert(pcm.samples.size() == encoded.size());
    assert(pcm.samples[0] == -8);
    assert(pcm.samples[1] == 8);
    assert(pcm.samples[2] == 5504);
    assert(pcm.samples[3] == -5504);
    return 0;
}
