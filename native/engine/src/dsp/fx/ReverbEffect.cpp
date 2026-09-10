#include "dsp/fx/ReverbEffect.hpp"

#include <algorithm>

namespace localmixer::dsp::fx {

ReverbEffect::ReverbEffect(ReverbConfig config) : config_(config) {}

void ReverbEffect::prepare(const ProcessSpec& spec) {
  config_.sampleRate = spec.sampleRate;
  left_.configure(config_);
  right_.configure(config_);
  scratchLeft_.assign(spec.maximumBlockFrames, 0.0f);
  scratchRight_.assign(spec.maximumBlockFrames, 0.0f);
}

void ReverbEffect::reset() noexcept {
  left_.reset();
  right_.reset();
}

void ReverbEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min({block.left.size(), block.right.size(), scratchLeft_.size(), scratchRight_.size()});
  left_.processWet(std::span<const float>(block.left.data(), frames), std::span<float>(scratchLeft_.data(), frames));
  right_.processWet(std::span<const float>(block.right.data(), frames), std::span<float>(scratchRight_.data(), frames));
  std::copy(scratchLeft_.begin(), scratchLeft_.begin() + static_cast<std::ptrdiff_t>(frames), block.left.begin());
  std::copy(scratchRight_.begin(), scratchRight_.begin() + static_cast<std::ptrdiff_t>(frames), block.right.begin());
}

void ReverbEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.decay = std::clamp(value, 0.0f, 0.95f);
}

std::uint32_t ReverbEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t ReverbEffect::maximumTailSamples() const noexcept {
  return static_cast<std::uint64_t>(config_.sampleRate * 3.0);
}

}  // namespace localmixer::dsp::fx
