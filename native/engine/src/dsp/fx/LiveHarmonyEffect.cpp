#include "dsp/fx/LiveHarmonyEffect.hpp"

#include "dsp/Gain.hpp"
#include "dsp/fx/HarmonyEffect.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp::fx {
namespace {

constexpr auto kVoiceCount = std::size_t{2};
constexpr auto kVoicedOpenRms = 0.012f;
constexpr auto kVoicedCloseRms = 0.006f;
constexpr auto kMaxVoicedZeroCrossRate = 0.18f;
constexpr auto kBackingVoiceToneHz = 4200.0f;
constexpr auto kPi = 3.14159265358979323846f;

float voiceLevel(float levelDb, float voiceDb) noexcept {
  return decibelsToLinear(std::clamp(levelDb + voiceDb, -60.0f, 6.0f));
}

}  // namespace

LiveHarmonyEffect::LiveHarmonyEffect(LiveHarmonyConfig config) : config_(config) {}

void LiveHarmonyEffect::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  blockSize_ = 0;
  for (auto& voice : voices_) {
    voice.backend = makeRubberBandPitchBackend();
    if (!voice.backend || !voice.backend->prepare(PitchBackendSpec{
                           .sampleRate = spec.sampleRate,
                           .channels = 1,
                           .preserveFormants = config_.preserveFormants,
                         })) {
      continue;
    }
    blockSize_ = std::max<std::size_t>(blockSize_, voice.backend->blockSize());
  }
  if (blockSize_ == 0) return;
  inputBlock_.assign(blockSize_, 0.0f);
  for (auto index = std::size_t{0}; index < kVoiceCount; index += 1) {
    shiftedBlocks_[index].assign(blockSize_, 0.0f);
    voices_[index].outputRing.assign(blockSize_ * 4, 0.0f);
  }
  configureVoices();
  reset();
}

void LiveHarmonyEffect::reset() noexcept {
  std::fill(inputBlock_.begin(), inputBlock_.end(), 0.0f);
  for (auto index = std::size_t{0}; index < kVoiceCount; index += 1) {
    std::fill(shiftedBlocks_[index].begin(), shiftedBlocks_[index].end(), 0.0f);
    auto& voice = voices_[index];
    if (voice.backend) voice.backend->reset();
    std::fill(voice.outputRing.begin(), voice.outputRing.end(), 0.0f);
    voice.read = 0;
    voice.write = 0;
    voice.fill = 0;
    voice.toneState = 0.0f;
  }
  inputFill_ = 0;
  voiceGate_ = 0.0f;
}

void LiveHarmonyEffect::process(AudioBlockView& block, const ProcessContext&) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  if (blockSize_ == 0 || inputBlock_.size() != blockSize_) return;
  const std::array<LiveHarmonyVoiceConfig, 2> configs{config_.voice1, config_.voice2};
  for (auto frame = std::size_t{0}; frame < frames; frame += 1) {
    inputBlock_[inputFill_] = (block.left[frame] + block.right[frame]) * 0.5f;
    inputFill_ += 1;
    if (inputFill_ == blockSize_) processReadyBlock();

    for (auto voiceIndex = std::size_t{0}; voiceIndex < kVoiceCount; voiceIndex += 1) {
      if (!configs[voiceIndex].enabled) continue;
      const auto sample = popVoiceOutput(voices_[voiceIndex]);
      const auto level = voiceLevel(config_.levelDb, configs[voiceIndex].levelDb);
      block.left[frame] += sample * panLeft(configs[voiceIndex].pan) * level;
      block.right[frame] += sample * panRight(configs[voiceIndex].pan) * level;
    }
  }
}

void LiveHarmonyEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.voice1.levelDb = std::clamp(value, -60.0f, 0.0f);
  if (parameter == 2) config_.voice2.levelDb = std::clamp(value, -60.0f, 0.0f);
  if (parameter == 3) config_.voice1.semitones = std::clamp(value, -12.0f, 12.0f);
  if (parameter == 4) config_.voice2.semitones = std::clamp(value, -12.0f, 12.0f);
  if (parameter == 5) config_.levelDb = std::clamp(value, -30.0f, 6.0f);
  configureVoices();
}

std::uint32_t LiveHarmonyEffect::latencySamples() const noexcept {
  const auto backendDelay = voices_[0].backend ? voices_[0].backend->startDelaySamples() : 0;
  return static_cast<std::uint32_t>(blockSize_) + backendDelay;
}

std::uint64_t LiveHarmonyEffect::maximumTailSamples() const noexcept {
  return latencySamples() + static_cast<std::uint32_t>(blockSize_);
}

void LiveHarmonyEffect::configureVoices() noexcept {
  if (voices_[0].backend) voices_[0].backend->setPitchSemitones(config_.voice1.semitones);
  if (voices_[1].backend) voices_[1].backend->setPitchSemitones(config_.voice2.semitones);
}

float LiveHarmonyEffect::analyzeVoicedGate() const noexcept {
  if (inputBlock_.empty()) return 0.0f;
  auto crossings = std::uint32_t{0};
  auto previous = inputBlock_.front();
  double energy = 0.0;
  for (const auto sample : inputBlock_) {
    energy += static_cast<double>(sample) * static_cast<double>(sample);
    if ((sample >= 0.0f && previous < 0.0f) || (sample < 0.0f && previous >= 0.0f)) crossings += 1;
    previous = sample;
  }
  const auto rms = static_cast<float>(std::sqrt(energy / static_cast<double>(inputBlock_.size())));
  const auto zeroCrossRate = static_cast<float>(crossings) / static_cast<float>(inputBlock_.size());
  if (rms < kVoicedCloseRms) return 0.0f;
  if (rms < kVoicedOpenRms && voiceGate_ <= 0.0f) return 0.0f;
  return zeroCrossRate <= kMaxVoicedZeroCrossRate ? 1.0f : 0.0f;
}

float LiveHarmonyEffect::smoothVoiceTone(VoiceState& voice, float sample) const noexcept {
  const auto sampleRate = static_cast<float>(std::max(1.0, spec_.sampleRate));
  const auto alpha = 1.0f - std::exp((-2.0f * kPi * kBackingVoiceToneHz) / sampleRate);
  voice.toneState += alpha * (sample - voice.toneState);
  return voice.toneState;
}

void LiveHarmonyEffect::processReadyBlock() noexcept {
  const auto targetGate = analyzeVoicedGate();
  voiceGate_ += std::clamp(targetGate - voiceGate_, -0.35f, 0.18f);
  for (auto voiceIndex = std::size_t{0}; voiceIndex < kVoiceCount; voiceIndex += 1) {
    auto& voice = voices_[voiceIndex];
    if (!voice.backend) continue;
    if (!voice.backend->processBlock(inputBlock_, shiftedBlocks_[voiceIndex])) continue;
    for (auto& sample : shiftedBlocks_[voiceIndex]) sample = smoothVoiceTone(voice, sample) * voiceGate_;
    pushVoiceOutput(voice, shiftedBlocks_[voiceIndex]);
  }
  std::fill(inputBlock_.begin(), inputBlock_.end(), 0.0f);
  inputFill_ = 0;
}

void LiveHarmonyEffect::pushVoiceOutput(VoiceState& voice, std::span<const float> samples) noexcept {
  if (voice.outputRing.empty()) return;
  for (const auto sample : samples) {
    if (voice.fill == voice.outputRing.size()) {
      voice.read = (voice.read + 1) % voice.outputRing.size();
      voice.fill -= 1;
    }
    voice.outputRing[voice.write] = sample;
    voice.write = (voice.write + 1) % voice.outputRing.size();
    voice.fill += 1;
  }
}

float LiveHarmonyEffect::popVoiceOutput(VoiceState& voice) noexcept {
  if (voice.outputRing.empty() || voice.fill == 0) return 0.0f;
  const auto sample = voice.outputRing[voice.read];
  voice.read = (voice.read + 1) % voice.outputRing.size();
  voice.fill -= 1;
  return sample;
}

}  // namespace localmixer::dsp::fx
