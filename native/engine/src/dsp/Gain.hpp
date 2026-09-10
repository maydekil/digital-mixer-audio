#pragma once

#include <span>

namespace localmixer::dsp {

float decibelsToLinear(float db);
void applyGain(std::span<float> samples, float gainLinear);

}  // namespace localmixer::dsp
