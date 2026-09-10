#pragma once

#include "engine/Timeline.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace localmixer::engine {

struct ExportRequest {
  std::filesystem::path outputPath;
  std::uint32_t sampleRate = 48000;
  std::uint64_t durationFrames = 0;
  std::uint32_t blockFrames = 512;
  std::uint32_t tailFrames = 0;
  std::uint32_t liveSourceCount = 0;
  std::optional<std::uint64_t> cancelAfterFrames;
};

struct ExportResult {
  bool success = false;
  bool canceled = false;
  std::string error;
  std::filesystem::path path;
  std::uint32_t ignoredLiveSources = 0;
  std::uint64_t framesWritten = 0;
};

struct ExportStemRequest {
  bool master = true;
  bool fxAReturn = false;
  bool fxBReturn = false;
  bool includeMonitorVolume = false;
};

struct ExportStemPlan {
  bool master = true;
  bool fxAReturn = false;
  bool fxBReturn = false;
  bool monitorVolumePrinted = false;
  std::uint32_t stemCount = 1;
};

ExportResult exportTimelineToWav(const TimelineScheduler& timeline, const ExportRequest& request);
ExportStemPlan buildExportStemPlan(const ExportStemRequest& request);
std::filesystem::path partialExportPath(const std::filesystem::path& outputPath);

}  // namespace localmixer::engine
