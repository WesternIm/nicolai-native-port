#include "nicolai/wav_writer.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace nicolai {
namespace {
void u16(std::ofstream& o, std::uint16_t v) {
    const char b[2] = {static_cast<char>(v & 0xff), static_cast<char>((v >> 8) & 0xff)};
    o.write(b, 2);
}
void u32(std::ofstream& o, std::uint32_t v) {
    const char b[4] = {
        static_cast<char>(v & 0xff), static_cast<char>((v >> 8) & 0xff),
        static_cast<char>((v >> 16) & 0xff), static_cast<char>((v >> 24) & 0xff)};
    o.write(b, 4);
}
} // namespace

void write_wav_pcm16_mono(const std::filesystem::path& path, const Pcm16Mono& pcm) {
    if (pcm.sample_rate <= 0) throw std::runtime_error("invalid sample rate");
    const auto data_bytes = static_cast<std::uint32_t>(pcm.samples.size() * sizeof(std::int16_t));
    std::ofstream o(path, std::ios::binary);
    if (!o) throw std::runtime_error("cannot create wav: " + path.string());
    o.write("RIFF", 4); u32(o, 36u + data_bytes); o.write("WAVE", 4);
    o.write("fmt ", 4); u32(o, 16); u16(o, 1); u16(o, 1);
    u32(o, static_cast<std::uint32_t>(pcm.sample_rate));
    u32(o, static_cast<std::uint32_t>(pcm.sample_rate * 2));
    u16(o, 2); u16(o, 16);
    o.write("data", 4); u32(o, data_bytes);
    o.write(reinterpret_cast<const char*>(pcm.samples.data()), data_bytes);
}

} // namespace nicolai
