#include "dsp/OutputProtection.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp {
namespace {

constexpr double kTwoPi = 6.28318530717958647692;

float clampSample(float sample, float ceiling) {
  return std::clamp(sample, -ceiling, ceiling);
}

}  // namespace

void applyOutputProtection(std::span<float> samples, const OutputProtection& settings) {
  if (settings.muted) {
    std::fill(samples.begin(), samples.end(), 0.0f);
    return;
  }

  const float gain = decibelsToLinear(settings.masterGainDb);
  const float ceiling = std::max(0.0f, std::min(settings.limitCeiling, 1.0f));
  for (auto& sample : samples) sample = clampSample(sample * gain, ceiling);
}

void generateTestTone(std::span<float> samples, double sampleRate, float frequencyHz, float levelDb) {
  const float amplitude = decibelsToLinear(levelDb);
  for (std::size_t index = 0; index < samples.size(); index += 1) {
    const double phase = kTwoPi * static_cast<double>(frequencyHz) * static_cast<double>(index) / sampleRate;
    samples[index] = static_cast<float>(std::sin(phase)) * amplitude;
  }
}

}  // namespace localmixer::dsp
