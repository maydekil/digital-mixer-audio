#include "engine/MonitorFileSource.hpp"

#include "engine/MediaFile.hpp"
#include "engine/MediaResampler.hpp"

#include <cmath>
#include <limits>
#include <utility>

namespace localmixer::engine {

MonitorFileBuffer prepareMonitorFile(const std::filesystem::path& path, double outputSampleRate) {
  if (!std::isfinite(outputSampleRate) || outputSampleRate < 8000.0 || outputSampleRate > 192000.0) {
    return {.error = "OUTPUT_RATE_UNAVAILABLE"};
  }
  WavStreamReader reader;
  if (reader.open(path) != MediaFileError::none) return {.error = "MEDIA_FILE_OPEN_FAILED"};
  const auto& info = reader.info();
  if (info.channels > 2) return {.error = "MEDIA_CHANNELS_UNSUPPORTED"};
  if (info.frameCount == 0 || info.frameCount > std::numeric_limits<std::uint32_t>::max()) {
    return {.error = "MEDIA_FILE_FRAME_COUNT_UNSUPPORTED"};
  }
  auto read = reader.readFrames(0, static_cast<std::uint32_t>(info.frameCount));
  if (read.error != MediaFileError::none || read.framesRead != info.frameCount) {
    return {.error = "MEDIA_FILE_READ_FAILED"};
  }
  MonitorFileBuffer result{.channels = info.channels};
  if (std::fabs(static_cast<double>(info.sampleRate) - outputSampleRate) < 0.01) {
    result.samples = std::move(read.samples);
  } else {
    result.samples = resampleLinear({
      .samples = read.samples,
      .channels = info.channels,
      .sourceSampleRate = static_cast<double>(info.sampleRate),
      .targetSampleRate = outputSampleRate,
    });
  }
  result.frameCount = result.samples.size() / result.channels;
  if (result.frameCount == 0) result.error = "MEDIA_FILE_RESAMPLE_FAILED";
  return result;
}

}  // namespace localmixer::engine
