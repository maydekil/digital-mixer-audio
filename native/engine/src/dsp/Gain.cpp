#include "dsp/Gain.hpp"

#include <cmath>

namespace localmixer::dsp {

float decibelsToLinear(float db) {
  if (db <= -120.0f) return 0.0f;
  return std::pow(10.0f, db / 20.0f);
}

void applyGain(std::span<float> samples, float gainLinear) {
  for (auto& sample : samples) sample *= gainLinear;
}

}  // namespace localmixer::dsp
