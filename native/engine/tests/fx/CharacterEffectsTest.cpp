#include "dsp/Gain.hpp"
#include "dsp/fx/CharacterEffects.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::decibelsToLinear;
using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::DoublerConfig;
using localmixer::dsp::fx::DoublerEffect;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::SaturationConfig;
using localmixer::dsp::fx::SaturationEffect;

bool near(float actual, float expected, float tolerance = 0.001f) {
  return std::fabs(actual - expected) <= tolerance;
}

float mean(const std::vector<float>& samples) {
  float sum = 0.0f;
  for (const auto sample : samples) sum += sample;
  return sum / static_cast<float>(samples.size());
}

float peak(const std::vector<float>& samples) {
  float value = 0.0f;
  for (const auto sample : samples) value = std::max(value, std::fabs(sample));
  return value;
}

}  // namespace

int main() {
  const ProcessSpec spec{.sampleRate = 48000.0, .maximumBlockFrames = 4096, .channels = 2};

  std::vector<float> left(4096);
  std::vector<float> right(4096);
  for (std::size_t index = 0; index < left.size(); index += 1) {
    const auto sample = std::sin(2.0 * 3.14159265358979323846 * 1000.0 * static_cast<double>(index) / 48000.0);
    left[index] = static_cast<float>(sample) * 0.8f;
    right[index] = left[index];
  }

  SaturationEffect saturation(SaturationConfig{.driveDb = 12.0f, .outputDb = -3.0f});
  saturation.prepare(spec);
  AudioBlockView block{.left = left, .right = right};
  saturation.process(block, ProcessContext{});
  if (peak(left) > 1.0f || std::fabs(mean(left)) > 0.05f) {
    std::cerr << "saturation should keep output bounded and suppress DC offset\n";
    return 1;
  }

  const auto drivenSample = left[6];
  left.assign(4096, 0.0f);
  right.assign(4096, 0.0f);
  for (std::size_t index = 0; index < left.size(); index += 1) {
    left[index] = std::sin(2.0 * 3.14159265358979323846 * 1000.0 * static_cast<double>(index) / 48000.0) * 0.8f;
    right[index] = left[index];
  }
  SaturationEffect unity(SaturationConfig{.driveDb = 0.0f, .outputDb = 0.0f});
  unity.prepare(spec);
  block = AudioBlockView{.left = left, .right = right};
  unity.process(block, ProcessContext{});
  if (near(drivenSample, left[6], 0.0001f)) {
    std::cerr << "saturation drive should change waveform shape versus unity drive\n";
    return 1;
  }

  left.assign(4096, 0.0f);
  right.assign(4096, 0.0f);
  left[0] = 1.0f;
  right[0] = 1.0f;
  DoublerEffect doubler(DoublerConfig{
    .voice1DelayMs = 5.0f,
    .voice2DelayMs = 9.0f,
    .voiceLevelDb = -6.0f,
    .voice1Pan = -1.0f,
    .voice2Pan = 1.0f,
  });
  doubler.prepare(spec);
  block = AudioBlockView{.left = left, .right = right};
  doubler.process(block, ProcessContext{});
  if (!near(left[0], 0.0f) || !near(right[0], 0.0f)) {
    std::cerr << "doubler wet output should not include direct dry voice\n";
    return 1;
  }
  if (left[240] <= 0.0f || right[432] <= 0.0f || peak(left) > decibelsToLinear(-5.5f) * 2.1f) {
    std::cerr << "doubler voices should appear at independent bounded delays\n";
    return 1;
  }

  std::cout << "local-mixer-character-effects-tests ok\n";
  return 0;
}
