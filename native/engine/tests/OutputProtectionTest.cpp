#include "dsp/OutputProtection.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  std::array<float, 4> samples{2.0f, -2.0f, 0.25f, -0.25f};
  localmixer::dsp::applyOutputProtection(samples, {.masterGainDb = 0.0f, .muted = false, .limitCeiling = 0.5f});
  if (!near(samples[0], 0.5f) || !near(samples[1], -0.5f)) {
    std::cerr << "limiter ceiling failed\n";
    return 1;
  }

  localmixer::dsp::applyOutputProtection(samples, {.masterGainDb = 0.0f, .muted = true, .limitCeiling = 0.5f});
  for (const auto sample : samples) {
    if (!near(sample, 0.0f)) {
      std::cerr << "mute failed\n";
      return 1;
    }
  }

  std::array<float, 480> tone{};
  localmixer::dsp::generateTestTone(tone, 48'000.0, 1'000.0f, -30.0f);
  float peak = 0.0f;
  for (const auto sample : tone) peak = std::max(peak, std::fabs(sample));
  if (peak > 0.032f || peak < 0.030f) {
    std::cerr << "test tone level failed\n";
    return 1;
  }

  std::cout << "output-protection-tests ok\n";
  return 0;
}
