#include "engine/ExportCommandHandlers.hpp"

#include "engine/Export.hpp"
#include "engine/JsonProtocol.hpp"
#include "engine/MediaFile.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>

namespace localmixer::engine {
namespace {

using protocol::escapeJson;
using protocol::readJsonBoolField;
using protocol::readJsonNumberField;
using protocol::readJsonStringField;

std::string exportRenderFailureFields(
  const std::string& error,
  const std::string& outputPath,
  std::uint32_t liveSourceCount
) {
  return "\"rendered\":false,\"canceled\":false,\"error\":\"" + escapeJson(error) +
    "\",\"path\":\"" + escapeJson(outputPath) +
    "\",\"framesWritten\":0,\"ignoredLiveSources\":" + std::to_string(liveSourceCount);
}

}  // namespace

std::string exportPlanJsonFields(const std::string& line) {
  const auto outputPath = readJsonStringField(line, "outputPath");
  const auto liveSourceCount = static_cast<std::uint32_t>(readJsonNumberField(line, "liveSourceCount").value_or(0.0));
  const auto plan = buildExportStemPlan(ExportStemRequest{
    .master = readJsonBoolField(line, "master").value_or(true),
    .fxAReturn = readJsonBoolField(line, "fxAReturn").value_or(false),
    .fxBReturn = readJsonBoolField(line, "fxBReturn").value_or(false),
    .includeMonitorVolume = readJsonBoolField(line, "includeMonitorVolume").value_or(false),
  });
  const bool exportable = !outputPath.empty() && liveSourceCount == 0 && plan.stemCount > 0;
  const std::string error = outputPath.empty()
    ? "INVALID_EXPORT_OUTPUT"
    : (liveSourceCount > 0 ? "LIVE_SOURCES_UNAVAILABLE_FOR_OFFLINE_EXPORT" : "");

  return "\"exportable\":" + std::string(exportable ? "true" : "false") +
    ",\"error\":\"" + escapeJson(error) + "\"" +
    ",\"outputPath\":\"" + escapeJson(outputPath) + "\"" +
    ",\"sampleRate\":" + std::to_string(readJsonNumberField(line, "sampleRate").value_or(48000.0)) +
    ",\"master\":" + std::string(plan.master ? "true" : "false") +
    ",\"fxAReturn\":" + std::string(plan.fxAReturn ? "true" : "false") +
    ",\"fxBReturn\":" + std::string(plan.fxBReturn ? "true" : "false") +
    ",\"stemCount\":" + std::to_string(plan.stemCount) +
    ",\"liveSourceCount\":" + std::to_string(liveSourceCount) +
    ",\"monitorVolumePrinted\":" + std::string(plan.monitorVolumePrinted ? "true" : "false");
}

std::string exportRenderJsonFields(const std::string& line) {
  const auto outputPath = readJsonStringField(line, "outputPath");
  const auto liveSourceCount = static_cast<std::uint32_t>(readJsonNumberField(line, "liveSourceCount").value_or(0.0));
  const auto sampleRate = static_cast<std::uint32_t>(readJsonNumberField(line, "sampleRate").value_or(48000.0));
  const auto durationFrames = static_cast<std::uint64_t>(readJsonNumberField(line, "durationFrames").value_or(0.0));
  const auto blockFrames = static_cast<std::uint32_t>(readJsonNumberField(line, "blockFrames").value_or(512.0));
  const auto tailFrames = static_cast<std::uint32_t>(readJsonNumberField(line, "tailFrames").value_or(0.0));
  const auto cancelAfter = readJsonNumberField(line, "cancelAfterFrames");
  const auto mediaPath = readJsonStringField(line, "mediaPath");

  TimelineScheduler timeline;
  if (!mediaPath.empty()) {
    WavStreamReader reader;
    const auto openError = reader.open(mediaPath);
    if (openError != MediaFileError::none) {
      return exportRenderFailureFields(mediaFileErrorName(openError), outputPath, liveSourceCount);
    }
    if (reader.info().sampleRate != sampleRate) {
      return exportRenderFailureFields("MEDIA_SAMPLE_RATE_MISMATCH", outputPath, liveSourceCount);
    }

    const auto framesToRead = static_cast<std::uint32_t>(std::min<std::uint64_t>(
      durationFrames == 0 ? reader.info().frameCount : durationFrames,
      std::min<std::uint64_t>(reader.info().frameCount, 0xffffffffu)
    ));
    const auto read = reader.readFrames(0, framesToRead);
    if (read.error != MediaFileError::none || read.framesRead == 0 ||
        !timeline.addMedia("media", TimelineMedia{.samples = read.samples, .channels = reader.info().channels})) {
      return exportRenderFailureFields("MEDIA_READ_FAILED", outputPath, liveSourceCount);
    }
    timeline.setClips({TimelineClip{.mediaId = "media", .durationFrames = read.framesRead}});
  }

  const auto result = exportTimelineToWav(timeline, ExportRequest{
    .outputPath = outputPath,
    .sampleRate = sampleRate,
    .durationFrames = durationFrames,
    .blockFrames = blockFrames,
    .tailFrames = tailFrames,
    .liveSourceCount = liveSourceCount,
    .cancelAfterFrames = cancelAfter.has_value()
      ? std::optional<std::uint64_t>(static_cast<std::uint64_t>(*cancelAfter))
      : std::nullopt,
  });
  return "\"rendered\":" + std::string(result.success ? "true" : "false") +
    ",\"canceled\":" + std::string(result.canceled ? "true" : "false") +
    ",\"error\":\"" + escapeJson(result.error) + "\"" +
    ",\"path\":\"" + escapeJson(result.path.string()) + "\"" +
    ",\"framesWritten\":" + std::to_string(result.framesWritten) +
    ",\"ignoredLiveSources\":" + std::to_string(result.ignoredLiveSources);
}

}  // namespace localmixer::engine
