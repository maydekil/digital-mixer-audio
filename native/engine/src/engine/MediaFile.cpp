#include "engine/MediaFile.hpp"

#include <algorithm>
#include <array>
#include <cstring>

namespace localmixer::engine {
namespace {

constexpr std::uint16_t kPcm = 1;
constexpr std::uint16_t kFloat = 3;

bool readExact(std::ifstream& file, char* data, std::streamsize bytes) {
  file.read(data, bytes);
  return file.gcount() == bytes;
}

bool readU16(std::ifstream& file, std::uint16_t& value) {
  std::array<unsigned char, 2> bytes{};
  if (!readExact(file, reinterpret_cast<char*>(bytes.data()), 2)) return false;
  value = static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
  return true;
}

bool readU32(std::ifstream& file, std::uint32_t& value) {
  std::array<unsigned char, 4> bytes{};
  if (!readExact(file, reinterpret_cast<char*>(bytes.data()), 4)) return false;
  value = static_cast<std::uint32_t>(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
  return true;
}

bool tagEquals(const char* tag, const char* expected) {
  return std::memcmp(tag, expected, 4) == 0;
}

float pcmToFloat(const unsigned char* bytes, std::uint16_t bits) {
  if (bits == 16) {
    const auto value = static_cast<std::int16_t>(bytes[0] | (bytes[1] << 8));
    return std::clamp(static_cast<float>(value) / 32768.0f, -1.0f, 1.0f);
  }
  if (bits == 24) {
    std::int32_t value = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16);
    if (value & 0x00800000) value |= static_cast<std::int32_t>(0xFF000000);
    return std::clamp(static_cast<float>(value) / 8388608.0f, -1.0f, 1.0f);
  }
  if (bits == 32) {
    std::int32_t value = static_cast<std::int32_t>(
      bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24)
    );
    return std::clamp(static_cast<float>(value) / 2147483648.0f, -1.0f, 1.0f);
  }
  return 0.0f;
}

}  // namespace

const char* mediaFileErrorName(MediaFileError error) {
  switch (error) {
    case MediaFileError::none: return "NONE";
    case MediaFileError::openFailed: return "OPEN_FAILED";
    case MediaFileError::unsupportedContainer: return "UNSUPPORTED_CONTAINER";
    case MediaFileError::unsupportedEncoding: return "UNSUPPORTED_ENCODING";
    case MediaFileError::malformedFile: return "MALFORMED_FILE";
    case MediaFileError::readFailed: return "READ_FAILED";
  }
  return "UNKNOWN";
}

MediaFileError WavStreamReader::open(const std::filesystem::path& path) {
  file_ = std::ifstream(path, std::ios::binary);
  info_ = {};
  dataOffset_ = 0;
  dataBytes_ = 0;
  formatTag_ = 0;
  if (!file_) return MediaFileError::openFailed;

  char riff[4]{};
  std::uint32_t riffSize = 0;
  char wave[4]{};
  if (!readExact(file_, riff, 4) || !readU32(file_, riffSize) || !readExact(file_, wave, 4)) {
    return MediaFileError::malformedFile;
  }
  if (!tagEquals(riff, "RIFF") || !tagEquals(wave, "WAVE")) return MediaFileError::unsupportedContainer;

  bool foundFormat = false;
  bool foundData = false;
  while (file_) {
    char chunkId[4]{};
    std::uint32_t chunkSize = 0;
    if (!readExact(file_, chunkId, 4)) break;
    if (!readU32(file_, chunkSize)) return MediaFileError::malformedFile;
    const auto chunkStart = file_.tellg();

    if (tagEquals(chunkId, "fmt ")) {
      std::uint32_t byteRate = 0;
      std::uint16_t blockAlign = 0;
      if (chunkSize < 16 || !readU16(file_, formatTag_) || !readU16(file_, info_.channels) ||
          !readU32(file_, info_.sampleRate) || !readU32(file_, byteRate) ||
          !readU16(file_, blockAlign) || !readU16(file_, info_.bitsPerSample)) {
        return MediaFileError::malformedFile;
      }
      foundFormat = true;
    } else if (tagEquals(chunkId, "data")) {
      dataOffset_ = static_cast<std::uint64_t>(file_.tellg());
      dataBytes_ = chunkSize;
      foundData = true;
    }

    file_.seekg(chunkStart + static_cast<std::streamoff>(chunkSize + (chunkSize % 2)));
  }

  if (!foundFormat || !foundData || info_.channels == 0 || info_.sampleRate == 0) return MediaFileError::malformedFile;
  if (!((formatTag_ == kPcm && (info_.bitsPerSample == 16 || info_.bitsPerSample == 24 || info_.bitsPerSample == 32)) ||
        (formatTag_ == kFloat && info_.bitsPerSample == 32))) {
    return MediaFileError::unsupportedEncoding;
  }
  const auto frameBytes = bytesPerFrame();
  if (frameBytes == 0 || dataBytes_ % frameBytes != 0) return MediaFileError::malformedFile;
  info_.frameCount = dataBytes_ / frameBytes;
  return MediaFileError::none;
}

const MediaFileInfo& WavStreamReader::info() const {
  return info_;
}

MediaReadResult WavStreamReader::readFrames(std::uint64_t startFrame, std::uint32_t frameCount) {
  file_.clear();
  if (!file_.is_open() || bytesPerFrame() == 0) return {.error = MediaFileError::readFailed};
  if (startFrame >= info_.frameCount || frameCount == 0) return {};

  const auto frames = std::min<std::uint64_t>(frameCount, info_.frameCount - startFrame);
  const auto bytesPerSample = info_.bitsPerSample / 8;
  const auto frameBytes = bytesPerFrame();
  std::vector<unsigned char> raw(static_cast<std::size_t>(frames * frameBytes));
  file_.seekg(static_cast<std::streamoff>(dataOffset_ + startFrame * frameBytes));
  if (!readExact(file_, reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(raw.size()))) {
    return {.error = MediaFileError::readFailed};
  }

  MediaReadResult result{.framesRead = frames};
  result.samples.resize(static_cast<std::size_t>(frames * info_.channels));
  for (std::size_t index = 0; index < result.samples.size(); index += 1) {
    const auto* sample = raw.data() + index * bytesPerSample;
    if (formatTag_ == kFloat) {
      float value = 0.0f;
      std::memcpy(&value, sample, sizeof(float));
      result.samples[index] = std::clamp(value, -1.0f, 1.0f);
    } else {
      result.samples[index] = pcmToFloat(sample, info_.bitsPerSample);
    }
  }
  return result;
}

std::uint32_t WavStreamReader::bytesPerFrame() const {
  return static_cast<std::uint32_t>(info_.channels) * (info_.bitsPerSample / 8);
}

}  // namespace localmixer::engine
