#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>

namespace localmixer::engine {

enum class RecordingState {
  idle,
  armed,
  recording,
  saved,
  failed,
  overrun,
};

enum class RecordingTap {
  dry,
  processed,
  master,
};

struct RecordingConfig {
  std::filesystem::path directory;
  std::string baseName = "take";
  std::uint32_t sampleRate = 48000;
  std::uint16_t channels = 2;
  RecordingTap tap = RecordingTap::master;
};

class WavFloatWriter {
 public:
  bool open(const std::filesystem::path& path, std::uint32_t sampleRate, std::uint16_t channels);
  bool writeInterleaved(std::span<const float> samples);
  bool finalize();
  std::uint64_t framesWritten() const;
  const std::filesystem::path& path() const;

 private:
  std::ofstream file_;
  std::filesystem::path path_;
  std::uint16_t channels_ = 0;
  std::uint64_t framesWritten_ = 0;
};

class RecordingSession {
 public:
  bool arm(RecordingConfig config);
  bool start();
  bool write(std::span<const float> samples);
  void markOverrun();
  bool stop();
  RecordingState state() const;
  std::uint64_t framesWritten() const;
  const std::filesystem::path& path() const;

 private:
  RecordingConfig config_;
  WavFloatWriter writer_;
  RecordingState state_ = RecordingState::idle;
};

std::filesystem::path makeCollisionSafeTakePath(
  const std::filesystem::path& directory, const std::string& baseName, const std::string& extension);

}  // namespace localmixer::engine
