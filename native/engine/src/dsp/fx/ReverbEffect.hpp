#pragma once

#include "dsp/Fx.hpp"
#include "dsp/fx/EffectProcessor.hpp"

#include <vector>

namespace localmixer::dsp::fx {

class ReverbEffect final : public EffectProcessor {
 public:
  explicit ReverbEffect(ReverbConfig config = {});
  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  ReverbConfig config_;
  SimpleReverb left_;
  SimpleReverb right_;
  std::vector<float> scratchLeft_;
  std::vector<float> scratchRight_;
};

}  // namespace localmixer::dsp::fx
