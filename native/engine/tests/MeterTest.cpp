#include "dsp/Meter.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

using localmixer::dsp::PeakRmsMeter;
using localmixer::dsp::amplitudeToDb;

bool near(float value, float expected, float tolerance = 0.08f) {
  return std::fabs(value - expected) <= tolerance;
}

}  // namespace

int main() {
  if (!near(amplitudeToDb(0.5f), -6.0206f)) {
    std::cerr << "0.5 amplitude should be about -6.02 dBFS peak\n";
    return 1;
  }

  PeakRmsMeter meter(4);
  constexpr float sine = 0.35355339f;
  std::array<float, 4> sineSamples{sine, -sine, sine, -sine};
  const auto sineReading = meter.process(sineSamples);
  if (!near(sineReading.peakDb, -9.0309f) || !near(sineReading.rmsDb, -9.0309f)) {
    std::cerr << "constant sine-quadrature fixture should produce expected peak/rms\n";
    return 1;
  }

  std::array<float, 2> clipSamples{0.2f, 1.0f};
  const auto clipReading = meter.process(clipSamples);
  if (!clipReading.clipped || !meter.clipLatched()) {
    std::cerr << "meter should latch digital clip\n";
    return 1;
  }
  meter.resetClip();
  if (meter.clipLatched()) {
    std::cerr << "clip reset should clear latch\n";
    return 1;
  }

  std::cout << "local-mixer-meter-tests ok\n";
  return 0;
}
