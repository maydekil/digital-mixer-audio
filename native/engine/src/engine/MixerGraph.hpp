#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "engine/ChannelProcessorChain.hpp"
#include "engine/MixerControlQueue.hpp"

namespace localmixer::engine {

constexpr std::size_t kMaxMixerStrips = 32;

enum class SourceAssignment {
  mono,
  stereo,
};

struct StripConfig {
  StripId id;
  std::string name;
  std::string color;
  std::string sourceUid;
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
  ChannelProcessorConfig processors{};
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

struct CreateStripResult {
  MixerError error = MixerError::none;
  StripId id;
};

class MixerGraph {
 public:
  MixerGraph();

  CreateStripResult createStrip(std::string name, std::string color);
  MixerError removeStrip(StripId id);
  MixerError renameStrip(StripId id, std::string name);
  MixerError setColor(StripId id, std::string color);
  MixerError setSourceUid(StripId id, std::string sourceUid);
  MixerError setAssignment(StripId id, SourceAssignment assignment, std::uint32_t inputChannel, bool stereoLinked);
  MixerError setLevel(StripId id, float trimDb, float faderDb, float pan);
  MixerError setMute(StripId id, bool mute);
  MixerError setSolo(StripId id, bool solo);
  MixerError setEnabled(StripId id, bool enabled);
  MixerError setInputMonitoring(StripId id, bool enabled);
  MixerError setProcessors(StripId id, ChannelProcessorConfig config);
  MixerError enqueueControl(MixerCommand command);
  MixerError applyQueuedControls(std::uint32_t rampFrames);
  MixerError process(std::span<const SourceBuffer> sources, StereoOutput output);

  std::optional<StripConfig> strip(StripId id) const;
  std::optional<StripMeters> meters(StripId id) const;
  std::size_t pendingControlCount() const;
  std::size_t stripCount() const;

 private:
  struct StripRuntime {
    float currentGain = 1.0f;
    float targetGain = 1.0f;
    float gainStep = 0.0f;
    float currentPan = 0.0f;
    float targetPan = 0.0f;
    float panStep = 0.0f;
    std::uint32_t remainingRampFrames = 0;
  };

  MixerError setLevelAt(std::size_t index, float trimDb, float faderDb, float pan, std::uint32_t rampFrames);
  std::optional<std::size_t> indexOf(StripId id) const;
  bool anySolo() const;
  void advanceRuntime(StripRuntime& runtime);

  std::uint32_t nextId_ = 1;
  MixerControlQueue controlQueue_;
  std::vector<StripConfig> strips_;
  std::vector<StripMeters> meters_;
  std::vector<StripRuntime> runtimes_;
  std::vector<ChannelProcessorChain> processors_;
};

const char* mixerErrorName(MixerError error);

}  // namespace localmixer::engine
