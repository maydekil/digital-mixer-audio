#include "engine/VocalStripPreset.hpp"

namespace localmixer::engine {
namespace {

std::array<dsp::EqBandConfig, 4> flatEq(double sampleRate) {
  return {
    dsp::EqBandConfig{.type = dsp::EqFilterType::lowShelf, .sampleRate = sampleRate, .frequencyHz = 100.0, .gainDb = 0.0},
    dsp::EqBandConfig{.type = dsp::EqFilterType::peaking, .sampleRate = sampleRate, .frequencyHz = 350.0, .gainDb = 0.0, .q = 1.2},
    dsp::EqBandConfig{.type = dsp::EqFilterType::peaking, .sampleRate = sampleRate, .frequencyHz = 2500.0, .gainDb = 0.0, .q = 1.0},
    dsp::EqBandConfig{.type = dsp::EqFilterType::highShelf, .sampleRate = sampleRate, .frequencyHz = 10000.0, .gainDb = 0.0},
  };
}

dsp::CompressorConfig voiceCompressor(double sampleRate, float thresholdDb, float ratio) {
  return dsp::CompressorConfig{
    .sampleRate = sampleRate,
    .thresholdDb = thresholdDb,
    .ratio = ratio,
    .attackMs = 10.0f,
    .releaseMs = 120.0f,
    .makeupGainDb = 0.0f,
  };
}

dsp::DeEsserConfig voiceDeEsser(double sampleRate, float thresholdDb) {
  return dsp::DeEsserConfig{
    .sampleRate = sampleRate,
    .detectorFrequencyHz = 6000.0f,
    .thresholdDb = thresholdDb,
    .maxReductionDb = 6.0f,
  };
}

}  // namespace

std::array<VocalStripPresetId, 4> vocalStripPresetIds() {
  return {
    VocalStripPresetId::voiceClean,
    VocalStripPresetId::voiceWarm,
    VocalStripPresetId::voiceBroadcast,
    VocalStripPresetId::musicFlat,
  };
}

VocalStripPreset makeVocalStripPreset(VocalStripPresetId id, double sampleRate) {
  VocalStripPreset preset;
  preset.id = id;
  preset.eqBands = flatEq(sampleRate);
  preset.compressor = voiceCompressor(sampleRate, -18.0f, 3.0f);
  preset.noise = dsp::NoiseGateConfig{.enabled = false, .sampleRate = sampleRate};
  preset.deEsser = voiceDeEsser(sampleRate, -24.0f);

  switch (id) {
    case VocalStripPresetId::voiceClean:
      preset.name = "Voice Clean";
      preset.hpfHz = 80.0f;
      return preset;
    case VocalStripPresetId::voiceWarm:
      preset.name = "Voice Warm";
      preset.hpfHz = 70.0f;
      preset.eqBands[0].gainDb = 1.5;
      preset.eqBands[2].gainDb = -1.0;
      preset.compressor = voiceCompressor(sampleRate, -20.0f, 2.5f);
      return preset;
    case VocalStripPresetId::voiceBroadcast:
      preset.name = "Voice Broadcast";
      preset.hpfHz = 90.0f;
      preset.eqBands[1].gainDb = -1.5;
      preset.eqBands[2].gainDb = 2.0;
      preset.eqBands[3].gainDb = 1.0;
      preset.compressor = voiceCompressor(sampleRate, -22.0f, 4.0f);
      preset.deEsser = voiceDeEsser(sampleRate, -28.0f);
      return preset;
    case VocalStripPresetId::musicFlat:
      preset.name = "Music Flat";
      preset.hpfHz = 20.0f;
      preset.compressor.enabled = false;
      preset.deEsser.enabled = false;
      return preset;
  }

  return preset;
}

}  // namespace localmixer::engine
