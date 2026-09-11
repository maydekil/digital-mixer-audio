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

  Compressor gentleRatio;
  gentleRatio.configure(CompressorConfig{.thresholdDb = -24.0f, .ratio = 2.0f, .attackMs = 0.0f});
  Compressor strongRatio;
  strongRatio.configure(CompressorConfig{.thresholdDb = -24.0f, .ratio = 8.0f, .attackMs = 0.0f});
  std::vector<float> gentleRatioSample(1, decibelsToLinear(-12.0f));
  std::vector<float> strongRatioSample(1, decibelsToLinear(-12.0f));
  gentleRatio.processMono(gentleRatioSample);
  strongRatio.processMono(strongRatioSample);
  if (linearToDecibels(strongRatioSample[0]) > linearToDecibels(gentleRatioSample[0]) - 3.0f) {
    std::cerr << "compressor ratio should change output gain\n";
    return 1;
  }

  Compressor fastAttack;
  fastAttack.configure(CompressorConfig{.thresholdDb = -30.0f, .ratio = 10.0f, .attackMs = 0.0f});
  Compressor slowAttack;
  slowAttack.configure(CompressorConfig{.thresholdDb = -30.0f, .ratio = 10.0f, .attackMs = 200.0f});
  std::vector<float> fastAttackSample(1, decibelsToLinear(-6.0f));
  std::vector<float> slowAttackSample(1, decibelsToLinear(-6.0f));
  fastAttack.processMono(fastAttackSample);
  slowAttack.processMono(slowAttackSample);
  if (linearToDecibels(fastAttackSample[0]) > linearToDecibels(slowAttackSample[0]) - 12.0f) {
    std::cerr << "compressor attack should change transient gain\n";
    return 1;
  }

  Compressor fastRelease;
  fastRelease.configure(CompressorConfig{.thresholdDb = -24.0f, .ratio = 8.0f, .attackMs = 0.0f, .releaseMs = 1.0f});
  Compressor slowRelease;
  slowRelease.configure(CompressorConfig{.thresholdDb = -24.0f, .ratio = 8.0f, .attackMs = 0.0f, .releaseMs = 1000.0f});
  std::vector<float> fastReleaseTrigger(8, decibelsToLinear(-6.0f));
  std::vector<float> slowReleaseTrigger(8, decibelsToLinear(-6.0f));
  fastRelease.processMono(fastReleaseTrigger);
  slowRelease.processMono(slowReleaseTrigger);
  std::vector<float> fastReleaseTail(480, decibelsToLinear(-30.0f));
  std::vector<float> slowReleaseTail(480, decibelsToLinear(-30.0f));
  fastRelease.processMono(fastReleaseTail);
  slowRelease.processMono(slowReleaseTail);
  if (linearToDecibels(fastReleaseTail.back()) < linearToDecibels(slowReleaseTail.back()) + 3.0f) {
    std::cerr << "compressor release should change recovery gain\n";
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

  NoiseGate hardGate;
  hardGate.configure(NoiseGateConfig{.thresholdDb = -6.0f, .hysteresisDb = 1.0f, .holdMs = 0.0f, .rangeDb = -96.0f, .attackMs = 0.0f, .releaseMs = 0.0f});
  std::vector<float> fanLike(16, decibelsToLinear(-12.0f));
  hardGate.processMono(fanLike);
  if (linearToDecibels(fanLike.back()) > -90.0f || hardGate.isOpen()) {
    std::cerr << "hard vocal noise gate should close on loud ambience below voice threshold\n";
    return 1;
  }

  hardGate.reset();
  std::vector<float> closeVoice(16, decibelsToLinear(-3.0f));
  hardGate.processMono(closeVoice);
  if (linearToDecibels(closeVoice.back()) < -4.0f || !hardGate.isOpen()) {
    std::cerr << "hard vocal noise gate should open for close voice level\n";
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
