#pragma once

#include "engine/MediaFile.hpp"
#include "engine/WaveformPyramid.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace localmixer::engine {

enum class MediaImportState {
  queued,
  running,
  completed,
  canceled,
  error,
};

struct MediaImportStatus {
  std::string jobId;
  MediaImportState state = MediaImportState::queued;
  MediaFileError error = MediaFileError::none;
  MediaFileInfo info;
  std::uint64_t processedFrames = 0;
  std::uint64_t totalFrames = 0;
  std::uint32_t waveformPoints = 0;
  double progress = 0.0;
};

class MediaImportJobManager {
 public:
  MediaImportJobManager();
  ~MediaImportJobManager();

  MediaImportStatus start(const std::filesystem::path& path, std::uint32_t framesPerPoint);
  std::optional<MediaImportStatus> status(const std::string& jobId);
  std::optional<MediaImportStatus> cancel(const std::string& jobId);

 private:
  struct Job;

  std::uint64_t nextJobNumber_ = 1;
  std::unordered_map<std::string, std::unique_ptr<Job>> jobs_;
};

const char* mediaImportStateName(MediaImportState state);

}  // namespace localmixer::engine
