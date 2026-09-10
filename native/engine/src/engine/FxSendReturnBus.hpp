#pragma once

#include "engine/MixerControlQueue.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace localmixer::engine {

enum class FxBusId {
  a,
  b,
};

struct FxSendState {
  bool enabled = false;
  float gainDb = -90.0f;
};

struct FxBusChannel {
  StripId stripId;
  bool enabled = true;
  bool mute = false;
  float faderDb = 0.0f;
  FxSendState sendA;
  FxSendState sendB;
};

struct FxUnitRuntime {
  bool enabled = false;
  bool mute = false;
  float returnDb = -12.0f;
};

struct FxBusMeters {
  float inputPeak = 0.0f;
  float returnPeakLeft = 0.0f;
  float returnPeakRight = 0.0f;
};

struct FxBusSource {
  StripId stripId;
  std::span<const float> samples;
};

struct FxBusFrame {
  std::span<const FxBusSource> sources;
  std::span<float> mainLeft;
  std::span<float> mainRight;
};

using FxWetProcessor = std::function<void(std::span<const float> input, std::span<float> left, std::span<float> right)>;

class FxSendReturnBus {
 public:
  void prepare(std::uint32_t maximumFrames);
  void setUnit(FxBusId bus, FxUnitRuntime unit) noexcept;
  void setChannels(std::vector<FxBusChannel> channels);
  void process(FxBusFrame frame, const FxWetProcessor& processorA, const FxWetProcessor& processorB) noexcept;

  FxBusMeters meters(FxBusId bus) const noexcept;

 private:
  void sumSend(FxBusId bus, std::span<const FxBusSource> sources, std::span<float> output) noexcept;
  void renderReturn(FxBusId bus, const FxWetProcessor& processor, std::span<float> mainLeft, std::span<float> mainRight) noexcept;

  std::vector<FxBusChannel> channels_;
  std::vector<float> sendA_;
  std::vector<float> sendB_;
  std::vector<float> wetLeft_;
  std::vector<float> wetRight_;
  FxUnitRuntime unitA_;
  FxUnitRuntime unitB_;
  FxBusMeters metersA_;
  FxBusMeters metersB_;
};

}  // namespace localmixer::engine
