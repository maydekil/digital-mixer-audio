#pragma once

#include <span>

namespace localmixer::dsp {

struct OutputProtection {
  float masterGainDb = 0.0f;
  bool muted = false;
  float limitCeiling = 0.98f;
};

void applyOutputProtection(std::span<float> samples, const OutputProtection& settings);
void generateTestTone(std::span<float> samples, double sampleRate, float frequencyHz, float levelDb);

}  // namespace localmixer::dsp
