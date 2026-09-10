#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace localmixer::engine {

struct ResampleRequest {
  std::span<const float> samples;
  std::uint16_t channels = 1;
  double sourceSampleRate = 48000.0;
  double targetSampleRate = 48000.0;
};

std::vector<float> resampleLinear(const ResampleRequest& request);
std::uint64_t resampledFrameCount(std::uint64_t sourceFrames, double sourceSampleRate, double targetSampleRate);

}  // namespace localmixer::engine
