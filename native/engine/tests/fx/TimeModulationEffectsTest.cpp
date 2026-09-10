#include "dsp/fx/DelayEffect.hpp"
#include "dsp/fx/ModulationEffects.hpp"
#include "dsp/fx/ReverbEffect.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::DelayConfig;
using localmixer::dsp::ReverbConfig;
using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::ChorusEffect;
using localmixer::dsp::fx::DelayEffect;
using localmixer::dsp::fx::FlangerEffect;
using localmixer::dsp::fx::ModulationConfig;
using localmixer::dsp::fx::PhaserEffect;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::ReverbEffect;

bool near(float actual, float expected, float tolerance = 0.001f) {
  return std::fabs(actual - expected) <= tolerance;
}

float peak(const std::vector<float>& samples) {
  float value = 0.0f;
  for (const auto sample : samples) value = std::max(value, std::fabs(sample));
  return value;
}

bool finite(const std::vector<float>& samples) {
  for (const auto sample : samples) {
    if (!std::isfinite(sample)) return false;
  }
  return true;
}

}  // namespace

int main() {
  const ProcessSpec spec{.sampleRate = 48000.0, .maximumBlockFrames = 4096, .channels = 2};
  std::vector<float> left(4096, 0.0f);
  std::vector<float> right(4096, 0.0f);
  left[0] = 1.0f;
  right[0] = 1.0f;

  DelayEffect delay(DelayConfig{.delayMs = 1.0f, .feedback = 0.5f, .wetGain = 1.0f, .tone = 1.0f});
  delay.prepare(spec);
  AudioBlockView block{.left = left, .right = right};
  delay.process(block, ProcessContext{});
  if (!near(left[0], 0.0f) || !near(left[48], 1.0f) || !near(left[96], 0.5f) || delay.maximumTailSamples() == 0) {
    std::cerr << "DelayEffect should produce wet-only repeats with bounded tail\n";
    return 1;
  }

  left.assign(4096, 0.0f);
  right.assign(4096, 0.0f);
  left[0] = 1.0f;
  right[0] = 1.0f;
  ReverbEffect reverb(ReverbConfig{.preDelayMs = 1.0f, .decay = 0.5f, .wetGain = 1.0f});
  reverb.prepare(spec);
  reverb.process(block, ProcessContext{});
  if (peak(left) <= 0.0f || reverb.maximumTailSamples() < 48000) {
    std::cerr << "ReverbEffect should produce a wet tail with reported maximum tail\n";
    return 1;
  }

  left.assign(4096, 0.25f);
  right.assign(4096, 0.25f);
  ChorusEffect chorus(ModulationConfig{.rateHz = 0.6f, .depth = 0.5f, .baseDelayMs = 7.0f});
  chorus.prepare(spec);
  chorus.process(block, ProcessContext{});
  if (!finite(left) || peak(left) <= 0.0f) {
    std::cerr << "ChorusEffect should produce finite modulated delay output\n";
    return 1;
  }

  left.assign(4096, 0.0f);
  right.assign(4096, 0.0f);
  left[0] = 1.0f;
  right[0] = 1.0f;
  FlangerEffect flanger(ModulationConfig{.rateHz = 0.25f, .depth = 0.8f, .baseDelayMs = 1.0f, .feedback = 0.8f});
  flanger.prepare(spec);
  flanger.process(block, ProcessContext{});
  if (!finite(left) || peak(left) > 1.0f || peak(left) <= 0.0f) {
    std::cerr << "FlangerEffect should remain bounded and finite\n";
    return 1;
  }

  left.assign(512, 0.25f);
  right.assign(512, 0.25f);
  PhaserEffect phaser(ModulationConfig{.rateHz = 0.4f, .depth = 0.7f});
  phaser.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = 512, .channels = 2});
  block = AudioBlockView{.left = left, .right = right};
  phaser.process(block, ProcessContext{});
  if (!finite(left) || near(left[0], 0.25f)) {
    std::cerr << "PhaserEffect should alter phase response without producing invalid samples\n";
    return 1;
  }

  std::cout << "local-mixer-time-modulation-effects-tests ok\n";
  return 0;
}
