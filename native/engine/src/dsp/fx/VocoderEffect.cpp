#include "dsp/fx/VocoderEffect.hpp"

#include "dsp/Gain.hpp"
#include "dsp/fx/PitchDetector.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp::fx {
namespace {

constexpr double kPi = 3.14159265358979323846;

VocoderEffect::BiquadState makeBandPass(double sampleRate, double frequencyHz, double q) {
  const auto omega = 2.0 * kPi * std::clamp(frequencyHz, 20.0, sampleRate * 0.45) / sampleRate;
  const auto alpha = std::sin(omega) / (2.0 * q);
  const auto cosw = std::cos(omega);
  const auto a0 = 1.0 + alpha;
  return {
    .b0 = alpha / a0,
    .b1 = 0.0,
    .b2 = -alpha / a0,
    .a1 = (-2.0 * cosw) / a0,
    .a2 = (1.0 - alpha) / a0,
  };
}

float envelopeCoefficient(double sampleRate, float ms) {
  const auto samples = std::max(1.0, sampleRate * static_cast<double>(ms) * 0.001);
  return static_cast<float>(std::exp(-1.0 / samples));
}

}  // namespace

VocoderEffect::VocoderEffect(VocoderConfig config) : config_(config) {}

void VocoderEffect::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  updateBands();
  reset();
}

void VocoderEffect::reset() noexcept {
  for (auto& band : bands_) {
    band.analysis.reset();
    band.synthesis.reset();
    band.envelope = 0.0f;
  }
  clearOscillators();
  noiseState_ = 0x12345678U;
}

void VocoderEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  const auto attack = envelopeCoefficient(spec_.sampleRate, config_.attackMs);
  const auto release = envelopeCoefficient(spec_.sampleRate, config_.releaseMs);
  const auto outputGain = localmixer::dsp::decibelsToLinear(config_.outputDb);
  const auto noiseMix = std::clamp(config_.unvoicedPct, 0.0f, 100.0f) * 0.01f;

  for (std::size_t index = 0; index < frames; index += 1) {
    const auto modulator = (block.left[index] + block.right[index]) * 0.5f;
    const auto carrier = nextCarrier() * (1.0f - noiseMix) + nextNoise() * noiseMix;
    float wet = 0.0f;
    for (auto& band : bands_) {
      const auto analyzed = band.analysis.process(modulator);
      const auto magnitude = std::fabs(analyzed);
      const auto coefficient = magnitude > band.envelope ? attack : release;
      band.envelope = coefficient * band.envelope + (1.0f - coefficient) * magnitude;
      wet += band.synthesis.process(carrier) * band.envelope;
    }
    wet = std::clamp(wet * outputGain * 2.0f, -1.0f, 1.0f);
    block.left[index] = wet;
    block.right[index] = wet;
  }
}

void VocoderEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.fixedMidiNote = std::clamp(static_cast<int>(std::lround(value)), 36, 84);
  if (parameter == 2) config_.attackMs = std::clamp(value, 1.0f, 100.0f);
  if (parameter == 3) config_.releaseMs = std::clamp(value, 10.0f, 500.0f);
  if (parameter == 4) config_.unvoicedPct = std::clamp(value, 0.0f, 100.0f);
  if (parameter == 5) config_.outputDb = std::clamp(value, -24.0f, 6.0f);
}

std::uint32_t VocoderEffect::latencySamples() const noexcept {
  return 0;
}

std::uint64_t VocoderEffect::maximumTailSamples() const noexcept {
  return static_cast<std::uint64_t>(spec_.sampleRate * config_.releaseMs * 0.001);
}

void VocoderEffect::noteOn(int midiNote) noexcept {
  if (config_.carrierMode != VocoderCarrierMode::midi) return;
  for (std::uint32_t index = 0; index < noteCount_; index += 1) {
    if (activeNotes_[index] == midiNote) return;
  }
  if (noteCount_ >= activeNotes_.size()) return;
  activeNotes_[noteCount_] = std::clamp(midiNote, 0, 127);
  phases_[noteCount_] = 0.0f;
  noteCount_ += 1;
}

void VocoderEffect::noteOff(int midiNote) noexcept {
  for (std::uint32_t index = 0; index < noteCount_; index += 1) {
    if (activeNotes_[index] != midiNote) continue;
    for (std::uint32_t move = index + 1; move < noteCount_; move += 1) {
      activeNotes_[move - 1] = activeNotes_[move];
      phases_[move - 1] = phases_[move];
    }
    noteCount_ -= 1;
    return;
  }
}

void VocoderEffect::allNotesOff() noexcept {
  noteCount_ = 0;
  for (auto& band : bands_) band.synthesis.reset();
}

void VocoderEffect::panic() noexcept {
  allNotesOff();
  for (auto& band : bands_) band.envelope = 0.0f;
}

float VocoderEffect::nextCarrier() noexcept {
  if (config_.carrierMode == VocoderCarrierMode::fixed && noteCount_ == 0) {
    activeNotes_[0] = std::clamp(config_.fixedMidiNote, 0, 127);
    noteCount_ = 1;
  }
  if (noteCount_ == 0 || spec_.sampleRate <= 0.0) return 0.0f;
  float sum = 0.0f;
  for (std::uint32_t index = 0; index < noteCount_; index += 1) {
    const auto hz = midiToFrequency(activeNotes_[index], 440.0f);
    phases_[index] += static_cast<float>(hz / spec_.sampleRate);
    if (phases_[index] >= 1.0f) phases_[index] -= 1.0f;
    sum += 2.0f * phases_[index] - 1.0f;
  }
  return sum / static_cast<float>(noteCount_);
}

float VocoderEffect::nextNoise() noexcept {
  noiseState_ = noiseState_ * 1664525U + 1013904223U;
  return (static_cast<float>((noiseState_ >> 8) & 0x00FFFFFFU) / 8388607.5f) - 1.0f;
}

void VocoderEffect::updateBands() {
  constexpr auto kMinHz = 100.0;
  constexpr auto kMaxHz = 8000.0;
  for (std::size_t index = 0; index < bands_.size(); index += 1) {
    const auto ratio = static_cast<double>(index) / static_cast<double>(bands_.size() - 1);
    const auto frequency = kMinHz * std::pow(kMaxHz / kMinHz, ratio);
    bands_[index].analysis = makeBandPass(spec_.sampleRate, frequency, 3.0);
    bands_[index].synthesis = makeBandPass(spec_.sampleRate, frequency, 3.0);
  }
}

void VocoderEffect::clearOscillators() noexcept {
  noteCount_ = 0;
  activeNotes_.fill(0);
  phases_.fill(0.0f);
}

float VocoderEffect::BiquadState::process(float sample) noexcept {
  const auto input = static_cast<double>(sample);
  const auto output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
  x2 = x1;
  x1 = input;
  y2 = y1;
  y1 = output;
  return static_cast<float>(output);
}

void VocoderEffect::BiquadState::reset() noexcept {
  x1 = 0.0;
  x2 = 0.0;
  y1 = 0.0;
  y2 = 0.0;
}

}  // namespace localmixer::dsp::fx
