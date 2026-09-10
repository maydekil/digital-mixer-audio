#include "dsp/Dynamics.hpp"
#include "dsp/Gain.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::Compressor;
using localmixer::dsp::CompressorConfig;
using localmixer::dsp::NoiseGate;
using localmixer::dsp::NoiseGateConfig;
using localmixer::dsp::NoiseMode;
using localmixer::dsp::decibelsToLinear;
using localmixer::dsp::linearToDecibels;

bool near(float actual, float expected, float tolerance = 0.35f) {
  return std::fabs(actual - expected) <= tolerance;
}

}  // namespace

int main() {
  Compressor compressor;
  compressor.configure(CompressorConfig{
    .sampleRate = 48000.0,
    .thresholdDb = -24.0f,
    .ratio = 4.0f,
    .attackMs = 1.0f,
    .releaseMs = 20.0f,
  });
  std::vector<float> steady(48000, decibelsToLinear(-12.0f));
  compressor.processMono(steady);
  if (!near(linearToDecibels(steady.back()), -21.0f, 0.2f)) {
    std::cerr << "compressor steady-state gain law mismatch\n";
    return 1;
  }
  if (!near(compressor.lastGainReductionDb(), 9.0f, 0.2f)) {
    std::cerr << "compressor gain-reduction meter mismatch\n";
    return 1;
  }

  Compressor bypassedCompressor;
  bypassedCompressor.configure(CompressorConfig{.enabled = false});
  std::vector<float> bypassed{0.25f, -0.5f};
  bypassedCompressor.processMono(bypassed);
  if (!near(bypassed[0], 0.25f, 0.0001f) || !near(bypassed[1], -0.5f, 0.0001f)) {
    std::cerr << "compressor bypass should preserve samples\n";
    return 1;
  }

  Compressor linkedCompressor;
  linkedCompressor.configure(CompressorConfig{.thresholdDb = -24.0f, .ratio = 4.0f, .attackMs = 0.0f});
  std::vector<float> stereo{decibelsToLinear(-12.0f), decibelsToLinear(-24.0f)};
  linkedCompressor.processInterleavedLinked(stereo, 2);
  if (!near(stereo[0] / stereo[1], decibelsToLinear(12.0f), 0.001f)) {
    std::cerr << "linked compressor should preserve stereo image\n";
    return 1;
  }

  NoiseGate gate;
  gate.configure(NoiseGateConfig{.thresholdDb = -50.0f, .hysteresisDb = 4.0f, .holdMs = 3.0f, .rangeDb = -80.0f});
  std::vector<float> silence(512, 0.0f);
  gate.processMono(silence);
  if (gate.isOpen() || gate.currentAttenuationDb() > -70.0f) {
    std::cerr << "silent gate should stay closed without flicker\n";
    return 1;
  }

  std::vector<float> voice(4096, decibelsToLinear(-18.0f));
  gate.processMono(voice);
  if (!gate.isOpen() || gate.currentAttenuationDb() < -1.0f) {
    std::cerr << "gate should open for voice-level input\n";
    return 1;
  }

  std::vector<float> hold(64, decibelsToLinear(-80.0f));
  gate.processMono(hold);
  if (!gate.isOpen()) {
    std::cerr << "gate hold should prevent immediate close\n";
    return 1;
  }

  NoiseGate expander;
  expander.configure(NoiseGateConfig{
    .mode = NoiseMode::expander,
    .thresholdDb = -40.0f,
    .rangeDb = -30.0f,
    .ratio = 2.0f,
    .attackMs = 0.0f,
    .releaseMs = 0.0f,
  });
  std::vector<float> ambience(1, decibelsToLinear(-50.0f));
  expander.processMono(ambience);
  if (!near(linearToDecibels(ambience[0]), -60.0f, 0.2f)) {
    std::cerr << "expander attenuation mismatch\n";
    return 1;
  }

  std::cout << "local-mixer-dynamics-tests ok\n";
  return 0;
}
