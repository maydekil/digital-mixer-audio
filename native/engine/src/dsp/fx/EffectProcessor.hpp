#pragma once

#include <cstdint>
#include <span>

namespace localmixer::dsp::fx {

using ParameterId = std::uint32_t;

struct ProcessSpec {
  double sampleRate = 48000.0;
  std::uint32_t maximumBlockFrames = 0;
  std::uint32_t channels = 1;
};

struct ProcessContext {
  double sampleRate = 48000.0;
  std::uint64_t absoluteFrame = 0;
  double tempoBpm = 120.0;
};

struct AudioBlockView {
  std::span<float> left;
  std::span<float> right;
};

class EffectProcessor {
 public:
  virtual ~EffectProcessor() = default;
  virtual void prepare(const ProcessSpec& spec) = 0;
  virtual void reset() noexcept = 0;
  virtual void process(AudioBlockView& block, const ProcessContext& context) noexcept = 0;
  virtual void applyRealtimeParameter(ParameterId parameter, float value) noexcept = 0;
  virtual std::uint32_t latencySamples() const noexcept = 0;
  virtual std::uint64_t maximumTailSamples() const noexcept = 0;
};

}  // namespace localmixer::dsp::fx
