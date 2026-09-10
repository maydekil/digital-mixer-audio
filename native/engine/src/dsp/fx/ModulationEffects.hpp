#pragma once

#include "dsp/fx/EffectProcessor.hpp"

#include <vector>

namespace localmixer::dsp::fx {

struct ModulationConfig {
  double sampleRate = 48000.0;
  float rateHz = 0.5f;
  float depth = 0.3f;
  float baseDelayMs = 8.0f;
  float feedback = 0.0f;
};

class ChorusEffect final : public EffectProcessor {
 public:
  explicit ChorusEffect(ModulationConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  ModulationConfig config_;
  std::vector<float> bufferLeft_;
  std::vector<float> bufferRight_;
  std::size_t writeIndex_ = 0;
  double phase_ = 0.0;
};

class FlangerEffect final : public EffectProcessor {
 public:
  explicit FlangerEffect(ModulationConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  ModulationConfig config_;
  std::vector<float> bufferLeft_;
  std::vector<float> bufferRight_;
  std::size_t writeIndex_ = 0;
  double phase_ = 0.0;
};

class PhaserEffect final : public EffectProcessor {
 public:
  explicit PhaserEffect(ModulationConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  ModulationConfig config_;
  float z1Left_ = 0.0f;
  float z1Right_ = 0.0f;
  double phase_ = 0.0;
};

}  // namespace localmixer::dsp::fx
