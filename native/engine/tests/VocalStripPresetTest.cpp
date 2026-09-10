#include "engine/VocalStripPreset.hpp"

#include <cmath>
#include <iostream>

namespace {

using localmixer::engine::VocalStripPresetId;
using localmixer::engine::makeVocalStripPreset;
using localmixer::engine::vocalStripPresetIds;

bool near(float actual, float expected, float tolerance = 0.001f) {
  return std::fabs(actual - expected) <= tolerance;
}

bool near(double actual, double expected, double tolerance = 0.001) {
  return std::fabs(actual - expected) <= tolerance;
}

}  // namespace

int main() {
  const auto ids = vocalStripPresetIds();
  if (ids.size() != 4) {
    std::cerr << "vocal strip preset count mismatch\n";
    return 1;
  }

  const auto clean = makeVocalStripPreset(VocalStripPresetId::voiceClean, 48000.0);
  if (clean.name != "Voice Clean" || !near(clean.hpfHz, 80.0f) || clean.gateEnabled ||
      clean.noise.enabled || !near(clean.compressor.thresholdDb, -18.0f) ||
      !near(clean.compressor.ratio, 3.0f) || !near(clean.compressor.attackMs, 10.0f) ||
      !near(clean.compressor.releaseMs, 120.0f) || !near(clean.compressor.makeupGainDb, 0.0f) ||
      !near(clean.deEsser.detectorFrequencyHz, 6000.0f) || !near(clean.deEsser.maxReductionDb, 6.0f)) {
    std::cerr << "Voice Clean preset should match the phase 09 starting values\n";
    return 1;
  }

  for (const auto& band : clean.eqBands) {
    if (!near(band.sampleRate, 48000.0) || !near(band.gainDb, 0.0)) {
      std::cerr << "Voice Clean EQ should start flat at the project sample rate\n";
      return 1;
    }
  }

  const auto music = makeVocalStripPreset(VocalStripPresetId::musicFlat, 44100.0);
  if (music.name != "Music Flat" || !near(music.hpfHz, 20.0f) ||
      music.compressor.enabled || music.noise.enabled || music.deEsser.enabled) {
    std::cerr << "Music Flat preset should keep vocal processors bypassed\n";
    return 1;
  }

  for (const auto& band : music.eqBands) {
    if (!near(band.sampleRate, 44100.0) || !near(band.gainDb, 0.0)) {
      std::cerr << "Music Flat EQ should remain flat at the requested sample rate\n";
      return 1;
    }
  }

  std::cout << "local-mixer-vocal-strip-preset-tests ok\n";
  return 0;
}
