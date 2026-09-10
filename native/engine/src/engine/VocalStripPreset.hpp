#pragma once

#include "dsp/DeEsser.hpp"
#include "dsp/Dynamics.hpp"
#include "dsp/Eq.hpp"

#include <array>
#include <string_view>

namespace localmixer::engine {

enum class VocalStripPresetId {
  voiceClean,
  voiceWarm,
  voiceBroadcast,
  musicFlat,
};

struct VocalStripPreset {
  VocalStripPresetId id = VocalStripPresetId::voiceClean;
  std::string_view name = "Voice Clean";
  float hpfHz = 80.0f;
  bool gateEnabled = false;
  std::array<dsp::EqBandConfig, 4> eqBands{};
  dsp::CompressorConfig compressor{};
  dsp::NoiseGateConfig noise{};
  dsp::DeEsserConfig deEsser{};
};

std::array<VocalStripPresetId, 4> vocalStripPresetIds();
VocalStripPreset makeVocalStripPreset(VocalStripPresetId id, double sampleRate);

}  // namespace localmixer::engine
