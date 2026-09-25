#pragma once

#include "nicolai/engine.hpp"
#include <cstdint>
#include <vector>

namespace nicolai {

// Legacy SpeechCube signal coding IDs recovered from mtsyc32.dll:
//   1 = G.711 mu-law ("loi mu")
//   2 = G.711 A-law  ("loi A")
//   3 = linear PCM   ("lineaire")
// Nicolai's nbr16aci.dsc stores type 2.
enum class LegacySignalCoding : std::uint32_t {
    MuLaw = 1,
    ALaw = 2,
    Linear = 3,
    Cmp16 = 0x50
};

const char* to_string(LegacySignalCoding coding) noexcept;

std::int16_t decode_g711_alaw_byte(std::uint8_t encoded) noexcept;
std::vector<std::int16_t> decode_g711_alaw(const std::vector<std::uint8_t>& encoded);
Pcm16Mono decode_g711_alaw_pcm(const std::vector<std::uint8_t>& encoded,
                               int sample_rate = 16000);

} // namespace nicolai
