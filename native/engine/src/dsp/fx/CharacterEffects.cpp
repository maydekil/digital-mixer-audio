#include "dsp/fx/CharacterEffects.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp::fx {
namespace {

std::size_t msToSamples(double sampleRate, float ms) {
  return static_cast<std::size_t>(std::max(1.0, sampleRate * ms * 0.001));
}

float panGainLeft(float pan) {
  return std::clamp(1.0f - pan, 0.0f, 1.0f);
}

float panGainRight(float pan) {
  return std::clamp(1.0f + pan, 0.0f, 1.0f);
}

}  // namespace

SaturationEffect::SaturationEffect(SaturationConfig config) : config_(config) {}

void SaturationEffect::prepare(const ProcessSpec&) {
  reset();
}

void SaturationEffect::reset() noexcept {
  previousInputLeft_ = 0.0f;
  previousInputRight_ = 0.0f;
  previousOutputLeft_ = 0.0f;
  previousOutputRight_ = 0.0f;
}

void SaturationEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  for (std::size_t index = 0; index < frames; index += 1) {
    block.left[index] = processSample(block.left[index], previousInputLeft_, previousOutputLeft_);
    block.right[index] = processSample(block.right[index], previousInputRight_, previousOutputRight_);
  }
}

void SaturationEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.driveDb = std::clamp(value, 0.0f, 24.0f);
  if (parameter == 2) config_.outputDb = std::clamp(value, -24.0f, 6.0f);
}

std::uint32_t SaturationEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t SaturationEffect::maximumTailSamples() const noexcept {
  return 64;
}

float SaturationEffect::processSample(float sample, float& previousInput, float& previousOutput) const noexcept {
  const auto drive = decibelsToLinear(config_.driveDb);
  const auto output = decibelsToLinear(config_.outputDb);
  const auto shaped = std::tanh(sample * drive) / std::tanh(drive);
  const auto blocked = shaped - previousInput + 0.995f * previousOutput;
  previousInput = shaped;
  previousOutput = blocked;
  return std::clamp(blocked * output, -1.0f, 1.0f);
}

DoublerEffect::DoublerEffect(DoublerConfig config) : config_(config) {}

void DoublerEffect::prepare(const ProcessSpec& spec) {
  config_.sampleRate = spec.sampleRate;
  const auto maxDelay = std::max(config_.voice1DelayMs, config_.voice2DelayMs) + 4.0f;
  buffer_.assign(msToSamples(config_.sampleRate, maxDelay) + 2, 0.0f);
  writeIndex_ = 0;
}

void DoublerEffect::reset() noexcept {
  std::fill(buffer_.begin(), buffer_.end(), 0.0f);
  writeIndex_ = 0;
}

void DoublerEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  const auto level = decibelsToLinear(config_.voiceLevelDb);
  const auto delay1 = static_cast<float>(config_.sampleRate * config_.voice1DelayMs * 0.001);
  const auto delay2 = static_cast<float>(config_.sampleRate * config_.voice2DelayMs * 0.001);
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto mono = (block.left[index] + block.right[index]) * 0.5f;
    const auto voice1 = readVoice(delay1) * level;
    const auto voice2 = readVoice(delay2) * level;
    buffer_[writeIndex_] = mono;
    writeIndex_ = (writeIndex_ + 1) % buffer_.size();
    block.left[index] = voice1 * panGainLeft(config_.voice1Pan) + voice2 * panGainLeft(config_.voice2Pan);
    block.right[index] = voice1 * panGainRight(config_.voice1Pan) + voice2 * panGainRight(config_.voice2Pan);
  }
}

void DoublerEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.voiceLevelDb = std::clamp(value, -60.0f, 0.0f);
}

std::uint32_t DoublerEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t DoublerEffect::maximumTailSamples() const noexcept {
  return buffer_.size();
}

float DoublerEffect::readVoice(float delaySamples) const noexcept {
  const auto size = buffer_.size();
  const auto read = static_cast<float>(writeIndex_) - delaySamples + static_cast<float>(size);
  const auto base = static_cast<std::size_t>(read) % size;
  const auto next = (base + 1) % size;
  const auto frac = read - std::floor(read);
  return buffer_[base] * (1.0f - frac) + buffer_[next] * frac;
}

}  // namespace localmixer::dsp::fx
