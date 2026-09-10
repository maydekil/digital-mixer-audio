#include "engine/Export.hpp"

#include "engine/Recording.hpp"

#include <algorithm>
#include <span>
#include <vector>

namespace localmixer::engine {

ExportResult exportTimelineToWav(const TimelineScheduler& timeline, const ExportRequest& request) {
  ExportResult result{.path = request.outputPath, .ignoredLiveSources = request.liveSourceCount};
  if (request.liveSourceCount > 0) {
    result.error = "LIVE_SOURCES_UNAVAILABLE_FOR_OFFLINE_EXPORT";
    return result;
  }
  if (request.outputPath.empty() || request.sampleRate == 0 || request.durationFrames == 0 || request.blockFrames == 0) {
    result.error = "INVALID_EXPORT_REQUEST";
    return result;
  }

  const auto finalFrame = request.durationFrames + request.tailFrames;
  const auto outputPath = request.cancelAfterFrames.has_value() ? partialExportPath(request.outputPath) : request.outputPath;
  WavFloatWriter writer;
  if (!writer.open(outputPath, request.sampleRate, 2)) {
    result.error = "OPEN_FAILED";
    return result;
  }

  std::vector<float> left(request.blockFrames);
  std::vector<float> right(request.blockFrames);
  std::vector<float> interleaved(request.blockFrames * 2);
  for (std::uint64_t frame = 0; frame < finalFrame;) {
    if (request.cancelAfterFrames.has_value() && frame >= *request.cancelAfterFrames) {
      result.canceled = true;
      break;
    }

    const auto framesThisBlock = std::min<std::uint64_t>(request.blockFrames, finalFrame - frame);
    auto leftBlock = std::span<float>(left.data(), static_cast<std::size_t>(framesThisBlock));
    auto rightBlock = std::span<float>(right.data(), static_cast<std::size_t>(framesThisBlock));
    if (!timeline.render(frame, TimelineRenderBuffer{.left = leftBlock, .right = rightBlock})) {
      result.error = "TIMELINE_RENDER_FAILED";
      return result;
    }
    for (std::size_t index = 0; index < framesThisBlock; index += 1) {
      interleaved[index * 2] = left[index];
      interleaved[index * 2 + 1] = right[index];
    }
    if (!writer.writeInterleaved(std::span<const float>(interleaved.data(), static_cast<std::size_t>(framesThisBlock * 2)))) {
      result.error = "WRITE_FAILED";
      return result;
    }
    frame += framesThisBlock;
  }

  if (!writer.finalize()) {
    result.error = "FINALIZE_FAILED";
    return result;
  }
  result.success = !result.canceled;
  result.path = outputPath;
  result.framesWritten = writer.framesWritten();
  return result;
}

std::filesystem::path partialExportPath(const std::filesystem::path& outputPath) {
  auto path = outputPath;
  path += ".partial";
  return path;
}

}  // namespace localmixer::engine
