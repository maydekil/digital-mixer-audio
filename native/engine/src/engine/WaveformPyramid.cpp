#include "engine/WaveformPyramid.hpp"

#include <algorithm>
#include <limits>

namespace localmixer::engine {

WaveformPyramid buildWaveformPyramid(WavStreamReader& reader, std::uint32_t framesPerPoint) {
  if (framesPerPoint == 0) return {.error = MediaFileError::malformedFile};

  const auto& info = reader.info();
  WaveformPyramid pyramid{
    .framesPerPoint = framesPerPoint,
    .sourceFrames = info.frameCount,
  };
  if (info.channels == 0 || info.frameCount == 0) return pyramid;

  constexpr std::uint32_t kReadFrames = 4096;
  float pointMin = std::numeric_limits<float>::max();
  float pointMax = std::numeric_limits<float>::lowest();
  std::uint32_t framesInPoint = 0;

  for (std::uint64_t cursor = 0; cursor < info.frameCount;) {
    const auto wanted = static_cast<std::uint32_t>(std::min<std::uint64_t>(kReadFrames, info.frameCount - cursor));
    const auto chunk = reader.readFrames(cursor, wanted);
    if (chunk.error != MediaFileError::none) return {.error = chunk.error};

    for (std::uint64_t frame = 0; frame < chunk.framesRead; frame += 1) {
      float mono = 0.0f;
      for (std::uint16_t channel = 0; channel < info.channels; channel += 1) {
        mono += chunk.samples[static_cast<std::size_t>(frame * info.channels + channel)];
      }
      mono /= static_cast<float>(info.channels);
      pointMin = std::min(pointMin, mono);
      pointMax = std::max(pointMax, mono);
      framesInPoint += 1;

      if (framesInPoint == framesPerPoint) {
        pyramid.points.push_back({.min = pointMin, .max = pointMax});
        pointMin = std::numeric_limits<float>::max();
        pointMax = std::numeric_limits<float>::lowest();
        framesInPoint = 0;
      }
    }
    cursor += chunk.framesRead;
  }

  if (framesInPoint > 0) pyramid.points.push_back({.min = pointMin, .max = pointMax});
  return pyramid;
}

}  // namespace localmixer::engine
