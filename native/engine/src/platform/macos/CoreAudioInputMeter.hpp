#pragma once

#include <cstdint>
#include <string>

namespace localmixer::platform::macos {

struct InputMeterRequest {
  std::string inputUid;
  double projectSampleRate = 48000.0;
  std::uint32_t durationMs = 500;
};

struct InputMeterResult {
  bool ok = false;
  std::string error;
  std::uint32_t inputChannels = 0;
  double actualSampleRate = 0.0;
  float peak = 0.0f;
};

InputMeterResult measureInputPeak(const InputMeterRequest& request);

}  // namespace localmixer::platform::macos
