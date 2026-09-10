#pragma once

#include "dsp/fx/EffectProcessor.hpp"
#include "dsp/fx/PitchBackend.hpp"

#include <memory>
#include <vector>

namespace localmixer::dsp::fx {

struct PitchShiftConfig {
  float semitones = 0.0f;
  float formantSemitones = 0.0f;
  float mix = 1.0f;
  bool preserveFormants = true;
};

class PitchShiftEffect final : public EffectProcessor {
 public:
  explicit PitchShiftEffect(PitchShiftConfig config = {});

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  void configureBackend() noexcept;
  bool processChunk(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept;

  PitchShiftConfig config_;
  ProcessSpec spec_;
  std::unique_ptr<PitchBackend> backend_;
  std::uint32_t blockSize_ = 0;
  std::vector<float> interleavedInput_;
  std::vector<float> interleavedOutput_;
};

class FormantShiftEffect final : public EffectProcessor {
 public:
  explicit FormantShiftEffect(float formantSemitones = 0.0f);

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  PitchShiftEffect shifter_;
};

}  // namespace localmixer::dsp::fx
