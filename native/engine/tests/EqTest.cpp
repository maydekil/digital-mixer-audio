#include "dsp/Eq.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

using localmixer::dsp::BiquadFilter;
using localmixer::dsp::EqBandConfig;
using localmixer::dsp::EqFilterType;
using localmixer::dsp::biquadMagnitudeDb;
using localmixer::dsp::makeBiquad;

bool near(double value, double expected, double tolerance = 0.35) {
  return std::fabs(value - expected) <= tolerance;
}

}  // namespace

int main() {
  const auto flat = makeBiquad(EqBandConfig{.enabled = false});
  if (!near(biquadMagnitudeDb(flat, 48000.0, 1000.0), 0.0, 0.001)) {
    std::cerr << "bypassed EQ should be unity\n";
    return 1;
  }

  const auto peak = makeBiquad(EqBandConfig{
    .type = EqFilterType::peaking,
    .sampleRate = 48000.0,
    .frequencyHz = 1000.0,
    .gainDb = 6.0,
    .q = 1.0,
  });
  if (!near(biquadMagnitudeDb(peak, 48000.0, 1000.0), 6.0)) {
    std::cerr << "peaking EQ should match center gain\n";
    return 1;
  }

  const auto highPass = makeBiquad(EqBandConfig{
    .type = EqFilterType::highPass,
    .sampleRate = 48000.0,
    .frequencyHz = 1000.0,
  });
  if (biquadMagnitudeDb(highPass, 48000.0, 100.0) > -30.0 ||
      !near(biquadMagnitudeDb(highPass, 48000.0, 1000.0), -3.01, 0.35)) {
    std::cerr << "HPF response should attenuate low frequencies and sit near -3 dB at cutoff\n";
    return 1;
  }

  BiquadFilter filter;
  filter.setCoefficients(peak);
  std::array<float, 8> samples{1.0f, 0.0f, -1.0f, 0.5f, -0.5f, 0.25f, -0.25f, 0.0f};
  filter.process(samples);
  for (const auto sample : samples) {
    if (!std::isfinite(sample)) {
      std::cerr << "EQ output should remain finite\n";
      return 1;
    }
  }

  std::cout << "local-mixer-eq-tests ok\n";
  return 0;
}
