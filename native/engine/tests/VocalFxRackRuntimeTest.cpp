#include "engine/VocalFxRackRuntime.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  localmixer::engine::VocalFxRackRuntime runtime;
  runtime.prepare(48000.0, 64);

  std::array<float, 64> input{};
  input.fill(0.2f);
  std::array<float, 64> left{};
  std::array<float, 64> right{};

  runtime.processMonoToStereo(input, left, right);
  if (!near(left[0], 0.2f) || !near(right[0], 0.2f) || runtime.active()) {
    std::cerr << "inactive Vocal FX runtime should pass mono through to stereo\n";
    return 1;
  }

  const auto configured = runtime.configure({
    localmixer::dsp::fx::RackSlotState{
      .instanceId = "drive",
      .effectType = "saturation",
      .mix = 1.0f,
    },
  });
  if (!configured || !runtime.active()) {
    std::cerr << "Vocal FX runtime should configure production rack slots\n";
    return 1;
  }

  runtime.processMonoToStereo(input, left, right);
  if (near(left[0], 0.2f) || near(right[0], 0.2f)) {
    std::cerr << "active Vocal FX runtime should process audio\n";
    return 1;
  }

  std::cout << "local-mixer-vocal-fx-rack-runtime-tests ok\n";
  return 0;
}
