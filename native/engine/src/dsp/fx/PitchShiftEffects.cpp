#include "dsp/fx/PitchShiftEffects.hpp"

#include <algorithm>

namespace localmixer::dsp::fx {

PitchShiftEffect::PitchShiftEffect(PitchShiftConfig config) : config_(config) {}

void PitchShiftEffect::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  backend_ = makeRubberBandPitchBackend();
  if (!backend_ || !backend_->prepare(PitchBackendSpec{
                     .sampleRate = spec.sampleRate,
                     .channels = std::max<std::uint32_t>(1, std::min<std::uint32_t>(2, spec.channels)),
                     .preserveFormants = config_.preserveFormants,
                   })) {
    blockSize_ = 0;
    return;
  }
  blockSize_ = backend_->blockSize();
  const auto channels = std::max<std::uint32_t>(1, std::min<std::uint32_t>(2, spec_.channels));
  interleavedInput_.assign(static_cast<std::size_t>(blockSize_) * channels, 0.0f);
  interleavedOutput_.assign(static_cast<std::size_t>(blockSize_) * channels, 0.0f);
  configureBackend();
}

void PitchShiftEffect::reset() noexcept {
  if (backend_) backend_->reset();
}

void PitchShiftEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  if (!backend_ || blockSize_ == 0 || frames == 0) return;
  std::size_t offset = 0;
  while (offset + blockSize_ <= frames) {
    if (!processChunk(block, offset, blockSize_)) return;
    offset += blockSize_;
  }
}

void PitchShiftEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 0) config_.semitones = value;
  if (parameter == 1) config_.formantSemitones = value;
  if (parameter == 2) config_.mix = std::clamp(value, 0.0f, 1.0f);
  configureBackend();
}

std::uint32_t PitchShiftEffect::latencySamples() const noexcept {
  return backend_ ? backend_->startDelaySamples() : 0;
}

std::uint64_t PitchShiftEffect::maximumTailSamples() const noexcept {
  return latencySamples();
}

void PitchShiftEffect::configureBackend() noexcept {
  if (!backend_) return;
  backend_->setPitchSemitones(config_.semitones);
  backend_->setFormantSemitones(config_.formantSemitones);
}

bool PitchShiftEffect::processChunk(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept {
  const auto channels = std::max<std::uint32_t>(1, std::min<std::uint32_t>(2, spec_.channels));
  if (frames != blockSize_ || interleavedInput_.size() < frames * channels ||
      interleavedOutput_.size() < frames * channels) {
    return false;
  }
  for (std::size_t frame = 0; frame < frames; frame += 1) {
    interleavedInput_[frame * channels] = block.left[offset + frame];
    if (channels > 1) interleavedInput_[frame * channels + 1] = block.right[offset + frame];
  }
  if (!backend_->processBlock(
        std::span<const float>(interleavedInput_.data(), frames * channels),
        std::span<float>(interleavedOutput_.data(), frames * channels))) {
    return false;
  }
  const auto mix = std::clamp(config_.mix, 0.0f, 1.0f);
  for (std::size_t frame = 0; frame < frames; frame += 1) {
    const auto wetLeft = interleavedOutput_[frame * channels];
    const auto wetRight = channels > 1 ? interleavedOutput_[frame * channels + 1] : wetLeft;
    block.left[offset + frame] = block.left[offset + frame] * (1.0f - mix) + wetLeft * mix;
    block.right[offset + frame] = block.right[offset + frame] * (1.0f - mix) + wetRight * mix;
  }
  return true;
}

FormantShiftEffect::FormantShiftEffect(float formantSemitones)
    : shifter_(PitchShiftConfig{.formantSemitones = formantSemitones, .mix = 1.0f, .preserveFormants = true}) {}

void FormantShiftEffect::prepare(const ProcessSpec& spec) {
  shifter_.prepare(spec);
}

void FormantShiftEffect::reset() noexcept {
  shifter_.reset();
}

void FormantShiftEffect::process(AudioBlockView& block, const ProcessContext& context) noexcept {
  shifter_.process(block, context);
}

void FormantShiftEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  shifter_.applyRealtimeParameter(parameter == 0 ? 1 : parameter, value);
}

std::uint32_t FormantShiftEffect::latencySamples() const noexcept {
  return shifter_.latencySamples();
}

std::uint64_t FormantShiftEffect::maximumTailSamples() const noexcept {
  return shifter_.maximumTailSamples();
}

}  // namespace localmixer::dsp::fx
