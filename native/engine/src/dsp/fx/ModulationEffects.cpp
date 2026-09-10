#include "dsp/fx/ModulationEffects.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp::fx {
namespace {

constexpr double kTwoPi = 6.28318530717958647692;

std::size_t bufferSize(double sampleRate, float baseDelayMs, float depth) {
  return static_cast<std::size_t>(sampleRate * (baseDelayMs + baseDelayMs * depth + 4.0f) * 0.001f) + 2;
}

float readDelay(const std::vector<float>& buffer, std::size_t writeIndex, float delaySamples) {
  const auto size = buffer.size();
  const auto read = static_cast<float>(writeIndex) - delaySamples + static_cast<float>(size);
  const auto base = static_cast<std::size_t>(read) % size;
  const auto next = (base + 1) % size;
  const auto frac = read - std::floor(read);
  return buffer[base] * (1.0f - frac) + buffer[next] * frac;
}

void prepareBuffer(std::vector<float>& buffer, const ProcessSpec& spec, const ModulationConfig& config) {
  buffer.assign(bufferSize(spec.sampleRate, config.baseDelayMs, config.depth), 0.0f);
}

}  // namespace

ChorusEffect::ChorusEffect(ModulationConfig config) : config_(config) {}

void ChorusEffect::prepare(const ProcessSpec& spec) {
  config_.sampleRate = spec.sampleRate;
  prepareBuffer(bufferLeft_, spec, config_);
  prepareBuffer(bufferRight_, spec, config_);
  writeIndex_ = 0;
}

void ChorusEffect::reset() noexcept {
  std::fill(bufferLeft_.begin(), bufferLeft_.end(), 0.0f);
  std::fill(bufferRight_.begin(), bufferRight_.end(), 0.0f);
  writeIndex_ = 0;
  phase_ = 0.0;
}

void ChorusEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto lfo = 0.5f + 0.5f * static_cast<float>(std::sin(phase_));
    const auto delaySamples = config_.sampleRate * (config_.baseDelayMs * (1.0f + config_.depth * lfo)) * 0.001f;
    const auto delayedLeft = readDelay(bufferLeft_, writeIndex_, delaySamples);
    const auto delayedRight = readDelay(bufferRight_, writeIndex_, delaySamples * 1.07f);
    bufferLeft_[writeIndex_] = block.left[index] + delayedLeft * config_.feedback;
    bufferRight_[writeIndex_] = block.right[index] + delayedRight * config_.feedback;
    block.left[index] = delayedLeft;
    block.right[index] = delayedRight;
    writeIndex_ = (writeIndex_ + 1) % bufferLeft_.size();
    phase_ += kTwoPi * config_.rateHz / config_.sampleRate;
    if (phase_ >= kTwoPi) phase_ -= kTwoPi;
  }
}

void ChorusEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.rateHz = std::clamp(value, 0.05f, 5.0f);
}

std::uint32_t ChorusEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t ChorusEffect::maximumTailSamples() const noexcept {
  return bufferLeft_.size();
}

FlangerEffect::FlangerEffect(ModulationConfig config) : config_(config) {}

void FlangerEffect::prepare(const ProcessSpec& spec) {
  config_.sampleRate = spec.sampleRate;
  prepareBuffer(bufferLeft_, spec, config_);
  prepareBuffer(bufferRight_, spec, config_);
  writeIndex_ = 0;
}

void FlangerEffect::reset() noexcept {
  std::fill(bufferLeft_.begin(), bufferLeft_.end(), 0.0f);
  std::fill(bufferRight_.begin(), bufferRight_.end(), 0.0f);
  writeIndex_ = 0;
  phase_ = 0.0;
}

void FlangerEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  const auto feedback = std::clamp(config_.feedback, -0.9f, 0.9f);
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto lfo = 0.5f + 0.5f * static_cast<float>(std::sin(phase_));
    const auto delaySamples = config_.sampleRate * (config_.baseDelayMs * (1.0f + config_.depth * lfo)) * 0.001f;
    const auto delayedLeft = readDelay(bufferLeft_, writeIndex_, delaySamples);
    const auto delayedRight = readDelay(bufferRight_, writeIndex_, delaySamples);
    bufferLeft_[writeIndex_] = block.left[index] + delayedLeft * feedback;
    bufferRight_[writeIndex_] = block.right[index] + delayedRight * feedback;
    block.left[index] = block.left[index] * 0.5f + delayedLeft * 0.5f;
    block.right[index] = block.right[index] * 0.5f + delayedRight * 0.5f;
    writeIndex_ = (writeIndex_ + 1) % bufferLeft_.size();
    phase_ += kTwoPi * config_.rateHz / config_.sampleRate;
    if (phase_ >= kTwoPi) phase_ -= kTwoPi;
  }
}

void FlangerEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.feedback = std::clamp(value, -0.9f, 0.9f);
}

std::uint32_t FlangerEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t FlangerEffect::maximumTailSamples() const noexcept {
  return bufferLeft_.size();
}

PhaserEffect::PhaserEffect(ModulationConfig config) : config_(config) {}

void PhaserEffect::prepare(const ProcessSpec& spec) {
  config_.sampleRate = spec.sampleRate;
  reset();
}

void PhaserEffect::reset() noexcept {
  z1Left_ = 0.0f;
  z1Right_ = 0.0f;
  phase_ = 0.0;
}

void PhaserEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto lfo = 0.5f + 0.5f * static_cast<float>(std::sin(phase_));
    const auto coefficient = std::clamp(0.15f + config_.depth * lfo * 0.75f, 0.01f, 0.95f);
    const auto left = -coefficient * block.left[index] + z1Left_;
    z1Left_ = block.left[index] + coefficient * left;
    const auto right = -coefficient * block.right[index] + z1Right_;
    z1Right_ = block.right[index] + coefficient * right;
    block.left[index] = left;
    block.right[index] = right;
    phase_ += kTwoPi * config_.rateHz / config_.sampleRate;
    if (phase_ >= kTwoPi) phase_ -= kTwoPi;
  }
}

void PhaserEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.depth = std::clamp(value, 0.0f, 1.0f);
}

std::uint32_t PhaserEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t PhaserEffect::maximumTailSamples() const noexcept {
  return 256;
}

}  // namespace localmixer::dsp::fx
