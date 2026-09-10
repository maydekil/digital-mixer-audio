#pragma once

#include "dsp/Fx.hpp"
#include "engine/FxProgramRegistry.hpp"
#include "engine/FxSendReturnBus.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace localmixer::engine {

class FxProgramWetProcessor {
 public:
  explicit FxProgramWetProcessor(double sampleRate = 48000.0);

  bool configure(std::uint32_t programId);
  void prepare(std::uint32_t maximumFrames);
  void reset();
  void process(std::span<const float> input, std::span<float> left, std::span<float> right);
  FxWetProcessor callback();
  std::uint32_t programId() const noexcept;

 private:
  double parameter(std::string_view id, double fallback) const;
  void processDelayPlate(std::span<const float> input, std::span<float> left, std::span<float> right);
  void processModulatedCopy(std::span<const float> input, std::span<float> left, std::span<float> right);

  double sampleRate_ = 48000.0;
  std::uint32_t programId_ = 0;
  std::optional<FxFactoryProgram> program_;
  dsp::DelayLine delay_;
  dsp::SimpleReverb reverb_;
  std::vector<float> scratchA_;
  std::vector<float> scratchB_;
  float phase_ = 0.0f;
  std::uint32_t maximumFrames_ = 0;
};

}  // namespace localmixer::engine
