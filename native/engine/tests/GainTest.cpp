#include "dsp/Gain.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  if (!near(localmixer::dsp::decibelsToLinear(0.0f), 1.0f)) {
    std::cerr << "0 dB should equal unity\n";
    return 1;
  }

  if (!near(localmixer::dsp::decibelsToLinear(-6.0f), 0.5011872f)) {
    std::cerr << "-6 dB conversion mismatch\n";
    return 1;
  }

  std::array<float, 3> samples{1.0f, -0.5f, 0.25f};
  localmixer::dsp::applyGain(samples, 0.5f);
  if (!near(samples[0], 0.5f) || !near(samples[1], -0.25f) || !near(samples[2], 0.125f)) {
    std::cerr << "applyGain mismatch\n";
    return 1;
  }

  std::cout << "local-mixer-dsp-tests ok\n";
  return 0;
}
