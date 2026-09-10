#pragma once

#include <cstdint>
#include <span>

namespace localmixer::dsp {

enum class EqFilterType {
  highPass,
  lowPass,
  peaking,
  lowShelf,
  highShelf,
};

struct BiquadCoefficients {
  double b0 = 1.0;
  double b1 = 0.0;
  double b2 = 0.0;
  double a1 = 0.0;
  double a2 = 0.0;
};

struct EqBandConfig {
  EqFilterType type = EqFilterType::peaking;
  bool enabled = true;
  double sampleRate = 48000.0;
  double frequencyHz = 1000.0;
  double gainDb = 0.0;
  double q = 0.70710678;
};

BiquadCoefficients makeBiquad(const EqBandConfig& config);
double biquadMagnitudeDb(const BiquadCoefficients& coefficients, double sampleRate, double frequencyHz);

class BiquadFilter {
 public:
  void setCoefficients(BiquadCoefficients coefficients);
  void reset();
  void process(std::span<float> samples);

 private:
  BiquadCoefficients coefficients_;
  double x1_ = 0.0;
  double x2_ = 0.0;
  double y1_ = 0.0;
  double y2_ = 0.0;
};

}  // namespace localmixer::dsp
