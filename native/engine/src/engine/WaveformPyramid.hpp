#pragma once

#include "engine/MediaFile.hpp"

#include <cstdint>
#include <vector>

namespace localmixer::engine {

struct WaveformPoint {
  float min = 0.0f;
  float max = 0.0f;
};

struct WaveformPyramid {
  MediaFileError error = MediaFileError::none;
  std::uint32_t framesPerPoint = 0;
  std::uint64_t sourceFrames = 0;
  std::vector<WaveformPoint> points;
};

WaveformPyramid buildWaveformPyramid(WavStreamReader& reader, std::uint32_t framesPerPoint);

}  // namespace localmixer::engine
