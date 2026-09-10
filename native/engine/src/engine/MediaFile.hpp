#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class MediaFileError {
  none,
  openFailed,
  unsupportedContainer,
  unsupportedEncoding,
  malformedFile,
  readFailed,
};

struct MediaFileInfo {
  std::uint16_t channels = 0;
  std::uint32_t sampleRate = 0;
  std::uint16_t bitsPerSample = 0;
  std::uint64_t frameCount = 0;
};

struct MediaReadResult {
  MediaFileError error = MediaFileError::none;
  std::vector<float> samples;
  std::uint64_t framesRead = 0;
};

const char* mediaFileErrorName(MediaFileError error);

class WavStreamReader {
 public:
  MediaFileError open(const std::filesystem::path& path);
  const MediaFileInfo& info() const;
  MediaReadResult readFrames(std::uint64_t startFrame, std::uint32_t frameCount);

 private:
  std::ifstream file_;
  MediaFileInfo info_;
  std::uint64_t dataOffset_ = 0;
  std::uint64_t dataBytes_ = 0;
  std::uint16_t formatTag_ = 0;

  std::uint32_t bytesPerFrame() const;
};

}  // namespace localmixer::engine
