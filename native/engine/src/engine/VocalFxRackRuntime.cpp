#include "engine/VocalFxRackRuntime.hpp"

#include "dsp/fx/EffectProcessorFactory.hpp"

#include <algorithm>

namespace localmixer::engine {

void VocalFxRackRuntime::prepare(double sampleRate, std::uint32_t maximumFrames) {
  sampleRate_ = sampleRate;
  maximumFrames_ = std::max<std::uint32_t>(1, maximumFrames);
  left_.assign(maximumFrames_, 0.0f);
  right_.assign(maximumFrames_, 0.0f);
  rack_.prepare(dsp::fx::ProcessSpec{.sampleRate = sampleRate_, .maximumBlockFrames = maximumFrames_, .channels = 2});
}

bool VocalFxRackRuntime::configure(std::vector<dsp::fx::RackSlotState> slots) {
  const auto result = rack_.replaceAll(std::move(slots), rack_.revision(), dsp::fx::nativeEffectFactory());
  active_ = result.error == dsp::fx::FxError::none && !rack_.slots().empty();
  return result.error == dsp::fx::FxError::none;
}

void VocalFxRackRuntime::processMonoToStereo(std::span<const float> input, std::span<float> left, std::span<float> right) {
  const auto frames = std::min(input.size(), std::min(left.size(), right.size()));
  if (!active_ || frames == 0 || frames > left_.size() || frames > right_.size()) {
    for (std::size_t index = 0; index < frames; index += 1) {
      left[index] = input[index];
      right[index] = input[index];
    }
    return;
  }

  std::copy(input.begin(), input.begin() + static_cast<std::ptrdiff_t>(frames), left_.begin());
  std::copy(input.begin(), input.begin() + static_cast<std::ptrdiff_t>(frames), right_.begin());
  auto block = dsp::fx::AudioBlockView{
    .left = std::span<float>(left_.data(), frames),
    .right = std::span<float>(right_.data(), frames),
  };
  rack_.process(block, dsp::fx::ProcessContext{.sampleRate = sampleRate_});
  std::copy(left_.begin(), left_.begin() + static_cast<std::ptrdiff_t>(frames), left.begin());
  std::copy(right_.begin(), right_.begin() + static_cast<std::ptrdiff_t>(frames), right.begin());
}

bool VocalFxRackRuntime::active() const noexcept {
  return active_;
}

}  // namespace localmixer::engine
