#include "engine/Recording.hpp"

#include <algorithm>
#include <system_error>
#include <utility>

namespace localmixer::engine {
namespace {

void writeU16(std::ofstream& file, std::uint16_t value) {
  const char bytes[2] = {
    static_cast<char>(value & 0xff),
    static_cast<char>((value >> 8) & 0xff),
  };
  file.write(bytes, 2);
}

void writeU32(std::ofstream& file, std::uint32_t value) {
  const char bytes[4] = {
    static_cast<char>(value & 0xff),
    static_cast<char>((value >> 8) & 0xff),
    static_cast<char>((value >> 16) & 0xff),
    static_cast<char>((value >> 24) & 0xff),
  };
  file.write(bytes, 4);
}

std::string takeName(const std::string& baseName, int suffix, const std::string& extension) {
  if (suffix == 0) return baseName + extension;
  auto number = std::to_string(suffix);
  while (number.size() < 3) number = "0" + number;
  return baseName + "-" + number + extension;
}

}  // namespace

bool WavFloatWriter::open(const std::filesystem::path& path, std::uint32_t sampleRate, std::uint16_t channels) {
  if (channels == 0 || sampleRate == 0) return false;
  path_ = path;
  channels_ = channels;
  framesWritten_ = 0;
  file_ = std::ofstream(path_, std::ios::binary | std::ios::trunc);
  if (!file_) return false;

  file_.write("RIFF", 4);
  writeU32(file_, 0);
  file_.write("WAVE", 4);
  file_.write("fmt ", 4);
  writeU32(file_, 16);
  writeU16(file_, 3);
  writeU16(file_, channels_);
  writeU32(file_, sampleRate);
  writeU32(file_, sampleRate * channels_ * static_cast<std::uint32_t>(sizeof(float)));
  writeU16(file_, static_cast<std::uint16_t>(channels_ * sizeof(float)));
  writeU16(file_, 32);
  file_.write("data", 4);
  writeU32(file_, 0);
  return static_cast<bool>(file_);
}

bool WavFloatWriter::writeInterleaved(std::span<const float> samples) {
  if (!file_ || channels_ == 0 || samples.size() % channels_ != 0) return false;
  for (const auto sample : samples) {
    const auto clamped = std::clamp(sample, -1.0f, 1.0f);
    file_.write(reinterpret_cast<const char*>(&clamped), sizeof(float));
  }
  framesWritten_ += samples.size() / channels_;
  return static_cast<bool>(file_);
}

bool WavFloatWriter::finalize() {
  if (!file_) return false;
  const auto dataBytes = framesWritten_ * channels_ * sizeof(float);
  if (dataBytes > 0xffffffffu) return false;
  file_.seekp(4);
  writeU32(file_, static_cast<std::uint32_t>(36 + dataBytes));
  file_.seekp(40);
  writeU32(file_, static_cast<std::uint32_t>(dataBytes));
  file_.close();
  return true;
}

std::uint64_t WavFloatWriter::framesWritten() const {
  return framesWritten_;
}

const std::filesystem::path& WavFloatWriter::path() const {
  return path_;
}

bool RecordingSession::arm(RecordingConfig config) {
  if (state_ == RecordingState::recording || config.channels == 0 || config.sampleRate == 0) return false;
  std::error_code error;
  std::filesystem::create_directories(config.directory, error);
  if (error) {
    state_ = RecordingState::failed;
    return false;
  }
  config_ = std::move(config);
  state_ = RecordingState::armed;
  return true;
}

bool RecordingSession::start() {
  if (state_ != RecordingState::armed) return false;
  const auto takePath = makeCollisionSafeTakePath(config_.directory, config_.baseName, ".wav");
  if (!writer_.open(takePath, config_.sampleRate, config_.channels)) {
    state_ = RecordingState::failed;
    return false;
  }
  state_ = RecordingState::recording;
  return true;
}

bool RecordingSession::write(std::span<const float> samples) {
  if (state_ != RecordingState::recording && state_ != RecordingState::overrun) return false;
  return writer_.writeInterleaved(samples);
}

void RecordingSession::markOverrun() {
  if (state_ == RecordingState::recording) state_ = RecordingState::overrun;
}

bool RecordingSession::stop() {
  if (state_ != RecordingState::recording && state_ != RecordingState::overrun) return false;
  const auto wasOverrun = state_ == RecordingState::overrun;
  if (!writer_.finalize()) {
    state_ = RecordingState::failed;
    return false;
  }
  state_ = wasOverrun ? RecordingState::overrun : RecordingState::saved;
  return true;
}

RecordingState RecordingSession::state() const {
  return state_;
}

std::uint64_t RecordingSession::framesWritten() const {
  return writer_.framesWritten();
}

const std::filesystem::path& RecordingSession::path() const {
  return writer_.path();
}

std::filesystem::path makeCollisionSafeTakePath(
  const std::filesystem::path& directory, const std::string& baseName, const std::string& extension) {
  for (int suffix = 0; suffix < 1000; suffix += 1) {
    const auto path = directory / takeName(baseName.empty() ? "take" : baseName, suffix, extension);
    std::error_code error;
    if (!std::filesystem::exists(path, error)) return path;
  }
  return directory / takeName(baseName.empty() ? "take" : baseName, 1000, extension);
}

}  // namespace localmixer::engine
