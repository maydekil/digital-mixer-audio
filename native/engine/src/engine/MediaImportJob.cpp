#include "engine/MediaImportJob.hpp"

#include <algorithm>
#include <limits>

namespace localmixer::engine {
namespace {

constexpr std::uint32_t kFramesPerPoll = 4096;

}  // namespace

struct MediaImportJobManager::Job {
  std::string id;
  std::unique_ptr<WavStreamReader> reader;
  MediaImportStatus status;
  std::uint32_t framesPerPoint = 512;
  std::uint32_t framesInPoint = 0;
  float pointMin = std::numeric_limits<float>::max();
  float pointMax = std::numeric_limits<float>::lowest();
  std::vector<WaveformPoint> points;

  MediaImportStatus step() {
    if (status.state != MediaImportState::queued && status.state != MediaImportState::running) return status;
    status.state = MediaImportState::running;
    const auto remaining = status.totalFrames - status.processedFrames;
    const auto wanted = static_cast<std::uint32_t>(std::min<std::uint64_t>(kFramesPerPoll, remaining));
    const auto chunk = reader->readFrames(status.processedFrames, wanted);
    if (chunk.error != MediaFileError::none) {
      status.state = MediaImportState::error;
      status.error = chunk.error;
      return status;
    }

    for (std::uint64_t frame = 0; frame < chunk.framesRead; frame += 1) {
      float mono = 0.0f;
      for (std::uint16_t channel = 0; channel < status.info.channels; channel += 1) {
        mono += chunk.samples[static_cast<std::size_t>(frame * status.info.channels + channel)];
      }
      mono /= static_cast<float>(status.info.channels);
      pointMin = std::min(pointMin, mono);
      pointMax = std::max(pointMax, mono);
      framesInPoint += 1;
      if (framesInPoint == framesPerPoint) {
        points.push_back({.min = pointMin, .max = pointMax});
        pointMin = std::numeric_limits<float>::max();
        pointMax = std::numeric_limits<float>::lowest();
        framesInPoint = 0;
      }
    }

    status.processedFrames += chunk.framesRead;
    status.progress = status.totalFrames == 0 ? 1.0 : static_cast<double>(status.processedFrames) / status.totalFrames;
    if (status.processedFrames >= status.totalFrames) {
      if (framesInPoint > 0) points.push_back({.min = pointMin, .max = pointMax});
      status.waveformPoints = static_cast<std::uint32_t>(points.size());
      status.state = MediaImportState::completed;
      status.progress = 1.0;
    }
    return status;
  }
};

const char* mediaImportStateName(MediaImportState state) {
  switch (state) {
    case MediaImportState::queued: return "queued";
    case MediaImportState::running: return "running";
    case MediaImportState::completed: return "completed";
    case MediaImportState::canceled: return "canceled";
    case MediaImportState::error: return "error";
  }
  return "unknown";
}

MediaImportJobManager::MediaImportJobManager() = default;

MediaImportJobManager::~MediaImportJobManager() = default;

MediaImportStatus MediaImportJobManager::start(const std::filesystem::path& path, std::uint32_t framesPerPoint) {
  const auto id = "media-job-" + std::to_string(nextJobNumber_++);
  auto reader = std::make_unique<WavStreamReader>();
  const auto error = reader->open(path);
  if (error != MediaFileError::none) {
    return {.jobId = id, .state = MediaImportState::error, .error = error};
  }

  auto job = std::make_unique<Job>();
  job->id = id;
  job->reader = std::move(reader);
  job->framesPerPoint = std::max<std::uint32_t>(1, framesPerPoint);
  job->status = MediaImportStatus{
    .jobId = id,
    .state = MediaImportState::queued,
    .info = job->reader->info(),
    .totalFrames = job->reader->info().frameCount,
  };
  const auto status = job->status;
  jobs_[id] = std::move(job);
  return status;
}

std::optional<MediaImportStatus> MediaImportJobManager::status(const std::string& jobId) {
  const auto found = jobs_.find(jobId);
  if (found == jobs_.end()) return std::nullopt;
  return found->second->step();
}

std::optional<MediaImportStatus> MediaImportJobManager::cancel(const std::string& jobId) {
  const auto found = jobs_.find(jobId);
  if (found == jobs_.end()) return std::nullopt;
  found->second->status.state = MediaImportState::canceled;
  found->second->status.progress = 0.0;
  return found->second->status;
}

}  // namespace localmixer::engine
