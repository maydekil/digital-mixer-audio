#pragma once

#include <cstdint>
#include <string>

namespace localmixer::platform::macos {

struct TestToneRequest {
  std::string outputUid;
  double projectSampleRate = 48000.0;
  double frequencyHz = 440.0;
  float levelDb = -30.0f;
  float monitorGainDb = 0.0f;
  std::uint32_t durationMs = 250;
};

struct TestToneResult {
  bool ok = false;
  std::string error;
  std::uint32_t outputChannels = 0;
  double actualSampleRate = 0.0;
};

TestToneResult playProtectedTestTone(const TestToneRequest& request);

}  // namespace localmixer::platform::macos
