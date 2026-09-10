#include "engine/MediaResampler.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {

std::uint64_t resampledFrameCount(std::uint64_t sourceFrames, double sourceSampleRate, double targetSampleRate) {
  if (sourceFrames == 0 || sourceSampleRate <= 0.0 || targetSampleRate <= 0.0) return 0;
  return static_cast<std::uint64_t>(std::llround(static_cast<double>(sourceFrames) * targetSampleRate / sourceSampleRate));
}

std::vector<float> resampleLinear(const ResampleRequest& request) {
  if (request.channels == 0 || request.samples.empty() || request.sourceSampleRate <= 0.0 ||
      request.targetSampleRate <= 0.0 || request.samples.size() % request.channels != 0) {
    return {};
  }
  const auto sourceFrames = request.samples.size() / request.channels;
  const auto targetFrames = resampledFrameCount(sourceFrames, request.sourceSampleRate, request.targetSampleRate);
  std::vector<float> output(static_cast<std::size_t>(targetFrames * request.channels), 0.0f);
  if (targetFrames == 0) return output;

  const auto ratio = request.sourceSampleRate / request.targetSampleRate;
  for (std::uint64_t frame = 0; frame < targetFrames; frame += 1) {
    const auto sourcePosition = static_cast<double>(frame) * ratio;
    const auto sourceIndex = static_cast<std::uint64_t>(sourcePosition);
    const auto nextIndex = std::min<std::uint64_t>(sourceIndex + 1, sourceFrames - 1);
    const auto blend = static_cast<float>(sourcePosition - static_cast<double>(sourceIndex));
    for (std::uint16_t channel = 0; channel < request.channels; channel += 1) {
      const auto a = request.samples[static_cast<std::size_t>(sourceIndex * request.channels + channel)];
      const auto b = request.samples[static_cast<std::size_t>(nextIndex * request.channels + channel)];
      output[static_cast<std::size_t>(frame * request.channels + channel)] = a + (b - a) * blend;
    }
  }
  return output;
}

}  // namespace localmixer::engine
