#pragma once

#include "dsp/fx/EffectProcessor.hpp"

#include <vector>

namespace localmixer::dsp::fx {

struct SaturationConfig {
  float driveDb = 3.0f;
  float outputDb = -3.0f;
  float tone = 0.5f;
};

class SaturationEffect final : public EffectProcessor {
 public:
  explicit SaturationEffect(SaturationConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  float processSample(float sample, float& previousInput, float& previousOutput) const noexcept;

  SaturationConfig config_;
  float previousInputLeft_ = 0.0f;
  float previousInputRight_ = 0.0f;
  float previousOutputLeft_ = 0.0f;
  float previousOutputRight_ = 0.0f;
};

struct DoublerConfig {
  double sampleRate = 48000.0;
  float voice1DelayMs = 15.0f;
  float voice2DelayMs = 23.0f;
  float voiceLevelDb = -9.0f;
  float voice1Pan = -0.7f;
  float voice2Pan = 0.7f;
};

class DoublerEffect final : public EffectProcessor {
 public:
  explicit DoublerEffect(DoublerConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  float readVoice(float delaySamples) const noexcept;

  DoublerConfig config_;
  std::vector<float> buffer_;
  std::size_t writeIndex_ = 0;
};

}  // namespace localmixer::dsp::fx
