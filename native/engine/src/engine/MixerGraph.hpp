#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace localmixer::engine {

constexpr std::size_t kMaxMixerStrips = 32;

enum class SourceAssignment {
  mono,
  stereo,
};

struct StripId {
  std::uint32_t value = 0;
  friend bool operator==(StripId left, StripId right) { return left.value == right.value; }
};

struct StripConfig {
  StripId id;
  std::string name;
  std::string color;
  SourceAssignment assignment = SourceAssignment::mono;
  std::uint32_t inputChannel = 0;
  bool stereoLinked = false;
  bool enabled = true;
  bool mute = false;
  bool solo = false;
  bool inputMonitoring = false;
  float trimDb = 0.0f;
  float faderDb = 0.0f;
  float pan = 0.0f;
};

struct StripMeters {
  float inputPeak = 0.0f;
  float outputPeakLeft = 0.0f;
  float outputPeakRight = 0.0f;
};

struct SourceBuffer {
  StripId stripId;
  std::span<const float> samples;
  std::uint32_t channels = 1;
};

struct StereoOutput {
  std::span<float> left;
  std::span<float> right;
};

enum class MixerError {
  none,
  graphFull,
  staleStripId,
  invalidSource,
  invalidBuffer,
};

struct CreateStripResult {
  MixerError error = MixerError::none;
  StripId id;
};

class MixerGraph {
 public:
  CreateStripResult createStrip(std::string name, std::string color);
  MixerError removeStrip(StripId id);
  MixerError renameStrip(StripId id, std::string name);
  MixerError setColor(StripId id, std::string color);
  MixerError setAssignment(StripId id, SourceAssignment assignment, std::uint32_t inputChannel, bool stereoLinked);
  MixerError setLevel(StripId id, float trimDb, float faderDb, float pan);
  MixerError setMute(StripId id, bool mute);
  MixerError setSolo(StripId id, bool solo);
  MixerError setEnabled(StripId id, bool enabled);
  MixerError setInputMonitoring(StripId id, bool enabled);
  MixerError process(std::span<const SourceBuffer> sources, StereoOutput output);

  std::optional<StripConfig> strip(StripId id) const;
  std::optional<StripMeters> meters(StripId id) const;
  std::size_t stripCount() const;

 private:
  std::optional<std::size_t> indexOf(StripId id) const;
  bool anySolo() const;

  std::uint32_t nextId_ = 1;
  std::vector<StripConfig> strips_;
  std::vector<StripMeters> meters_;
};

const char* mixerErrorName(MixerError error);

}  // namespace localmixer::engine
