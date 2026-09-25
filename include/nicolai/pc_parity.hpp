#pragma once
#include "nicolai/engine.hpp"

namespace nicolai {

// The original 32-bit SAPI Nicolai reference captured at volume=100 is
// approximately 2x the portable pipeline peak level for ordinary lexical
// material. Keep this as a separate compatibility stage rather than baking the
// multiplier into G.711 or TD-PSOLA, where it would obscure codec/DSP parity.
constexpr double kPcReferenceOutputGain = 2.0;

void apply_pc_reference_output_gain(Pcm16Mono& pcm,
                                    double gain = kPcReferenceOutputGain) noexcept;

} // namespace nicolai
