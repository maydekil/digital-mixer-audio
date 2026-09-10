#pragma once

#include "dsp/fx/EffectRack.hpp"

#include <span>
#include <vector>

namespace localmixer::engine {

class VocalFxRackRuntime {
 public:
  void prepare(double sampleRate, std::uint32_t maximumFrames);
  bool configure(std::vector<dsp::fx::RackSlotState> slots);
  void processMonoToStereo(std::span<const float> input, std::span<float> left, std::span<float> right);
  bool active() const noexcept;

 private:
  dsp::fx::EffectRack rack_;
  std::vector<float> left_;
  std::vector<float> right_;
  bool active_ = false;
  double sampleRate_ = 48000.0;
  std::uint32_t maximumFrames_ = 0;
};

}  // namespace localmixer::engine
