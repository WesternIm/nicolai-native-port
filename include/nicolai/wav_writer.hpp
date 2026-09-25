#pragma once

#include "nicolai/engine.hpp"
#include <filesystem>

namespace nicolai {

void write_wav_pcm16_mono(const std::filesystem::path& path, const Pcm16Mono& pcm);

} // namespace nicolai
