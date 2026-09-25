#include "nicolai/g711.hpp"

namespace nicolai {

const char* to_string(LegacySignalCoding coding) noexcept {
    switch (coding) {
        case LegacySignalCoding::MuLaw: return "g711_mu_law";
        case LegacySignalCoding::ALaw: return "g711_a_law";
        case LegacySignalCoding::Linear: return "linear_pcm";
        case LegacySignalCoding::Cmp16: return "cmp16";
    }
    return "unknown";
}

std::int16_t decode_g711_alaw_byte(std::uint8_t encoded) noexcept {
    // ITU-T G.711 A-law expansion. A-law bytes are XOR-masked with 0x55.
    const std::uint8_t a = static_cast<std::uint8_t>(encoded ^ 0x55u);
    int value = static_cast<int>(a & 0x0fu) << 4;
    const int segment = static_cast<int>((a & 0x70u) >> 4);

    if (segment == 0) {
        value += 8;
    } else if (segment == 1) {
        value += 0x108;
    } else {
        value += 0x108;
        value <<= (segment - 1);
    }

    return static_cast<std::int16_t>((a & 0x80u) ? value : -value);
}

std::vector<std::int16_t> decode_g711_alaw(const std::vector<std::uint8_t>& encoded) {
    std::vector<std::int16_t> out;
    out.reserve(encoded.size());
    for (const auto byte : encoded) out.push_back(decode_g711_alaw_byte(byte));
    return out;
}

Pcm16Mono decode_g711_alaw_pcm(const std::vector<std::uint8_t>& encoded,
                               int sample_rate) {
    Pcm16Mono pcm;
    pcm.sample_rate = sample_rate;
    pcm.samples = decode_g711_alaw(encoded);
    return pcm;
}

} // namespace nicolai
