#include "dsp/fx/HarmonyEffect.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace localmixer::dsp::fx {
namespace {

constexpr std::array<int, 12> kChromaticDegrees{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
constexpr std::array<int, 7> kMajorDegrees{0, 2, 4, 5, 7, 9, 11};
constexpr std::array<int, 7> kMinorDegrees{0, 2, 3, 5, 7, 8, 10};

std::span<const int> degreesFor(ScaleType scale) {
  if (scale == ScaleType::major) return kMajorDegrees;
  if (scale == ScaleType::naturalMinor) return kMinorDegrees;
  return kChromaticDegrees;
}

int floorDiv(int value, int divisor) {
  const auto quotient = value / divisor;
  const auto remainder = value % divisor;
  return remainder < 0 ? quotient - 1 : quotient;
}

float semitonesBetweenMidi(int sourceMidi, int targetMidi) {
  return static_cast<float>(targetMidi - sourceMidi);
}

}  // namespace

HarmonyEffect::HarmonyEffect(HarmonyConfig config) : config_(config) {}

void HarmonyEffect::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  enabledTarget_ = config_.enabled ? 1.0f : 0.0f;
  enabledRamp_ = enabledTarget_;
  detector_.prepare(PitchDetectorConfig{.sampleRate = spec.sampleRate});
  for (auto& voice : voices_) {
    voice = makeRubberBandPitchBackend();
    voice->prepare(PitchBackendSpec{.sampleRate = spec.sampleRate, .channels = 1, .preserveFormants = config_.preserveFormants});
  }
  const auto backendFrames = voices_[0] ? voices_[0]->blockSize() : spec.maximumBlockFrames;
  analysisRing_.assign(detector_.windowFrames(), 0.0f);
  analysisWindow_.assign(detector_.windowFrames(), 0.0f);
  voiceInput_.assign(backendFrames, 0.0f);
  for (auto& output : voiceOutput_) output.assign(backendFrames, 0.0f);
  reset();
}

void HarmonyEffect::reset() noexcept {
  std::fill(analysisRing_.begin(), analysisRing_.end(), 0.0f);
  std::fill(analysisWindow_.begin(), analysisWindow_.end(), 0.0f);
  std::fill(voiceInput_.begin(), voiceInput_.end(), 0.0f);
  for (auto& output : voiceOutput_) std::fill(output.begin(), output.end(), 0.0f);
  for (auto& voice : voices_) {
    if (voice) voice->reset();
  }
  analysisWrite_ = 0;
  samplesSinceAnalysis_ = 0;
  filledAnalysis_ = 0;
  voiceGate_ = 0.0f;
  enabledTarget_ = config_.enabled ? 1.0f : 0.0f;
  enabledRamp_ = enabledTarget_;
  lastTarget_ = {};
}

void HarmonyEffect::process(AudioBlockView& block, const ProcessContext& context) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto mono = (block.left[index] + block.right[index]) * 0.5f;
    pushAnalysisSample(mono);
    analyzeIfReady(context.absoluteFrame + index);
  }

  const auto backendFrames = voices_[0] ? voices_[0]->blockSize() : 0;
  if (backendFrames == 0) return;
  for (std::size_t offset = 0; offset < frames; offset += backendFrames) {
    const auto count = std::min<std::size_t>(backendFrames, frames - offset);
    if (!processBackendBlock(block, offset, count)) return;
  }
}

void HarmonyEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.voice1.levelDb = std::clamp(value, -60.0f, 0.0f);
  if (parameter == 2) config_.voice2.levelDb = std::clamp(value, -60.0f, 0.0f);
  if (parameter == 3) config_.voice1.interval = std::clamp(static_cast<int>(std::lround(value)), -12, 12);
  if (parameter == 4) config_.voice2.interval = std::clamp(static_cast<int>(std::lround(value)), -12, 12);
  if (parameter == 5) setHarmonyLevelDb(value);
}

std::uint32_t HarmonyEffect::latencySamples() const noexcept {
  const auto backendDelay = voices_[0] ? voices_[0]->startDelaySamples() : 0;
  return backendDelay + detector_.windowFrames() / 2;
}

std::uint64_t HarmonyEffect::maximumTailSamples() const noexcept {
  return latencySamples() + detector_.windowFrames();
}

void HarmonyEffect::setEnabled(bool enabled) noexcept {
  config_.enabled = enabled;
  enabledTarget_ = enabled ? 1.0f : 0.0f;
}

void HarmonyEffect::setHarmonyLevelDb(float levelDb) noexcept {
  config_.harmonyLevelDb = std::clamp(levelDb, -30.0f, 6.0f);
}

void HarmonyEffect::pushAnalysisSample(float sample) noexcept {
  if (analysisRing_.empty()) return;
  analysisRing_[analysisWrite_] = sample;
  analysisWrite_ = (analysisWrite_ + 1) % analysisRing_.size();
  samplesSinceAnalysis_ += 1;
  filledAnalysis_ = std::min<std::uint32_t>(filledAnalysis_ + 1, static_cast<std::uint32_t>(analysisRing_.size()));
}

void HarmonyEffect::analyzeIfReady(std::uint64_t frame) noexcept {
  if (filledAnalysis_ < analysisRing_.size() || samplesSinceAnalysis_ < hopFrames_) return;
  samplesSinceAnalysis_ = 0;
  for (std::size_t index = 0; index < analysisRing_.size(); index += 1) {
    analysisWindow_[index] = analysisRing_[(analysisWrite_ + index) % analysisRing_.size()];
  }
  const auto estimate = detector_.analyze(analysisWindow_, frame - analysisRing_.size() + 1);
  if (!estimate.voiced || estimate.confidence < config_.confidenceThreshold) {
    voiceGate_ = std::max(0.0f, voiceGate_ - 0.02f);
    lastTarget_ = {};
    return;
  }

  const auto sourceMidi = static_cast<int>(std::lround(frequencyToMidi(estimate.frequencyHz, config_.a4Hz)));
  lastTarget_.active = true;
  lastTarget_.sourceMidi = sourceMidi;
  const std::array<HarmonyVoiceConfig, 2> configs{config_.voice1, config_.voice2};
  for (std::size_t voice = 0; voice < configs.size(); voice += 1) {
    const auto targetMidi = config_.mode == HarmonyMode::fixed
      ? sourceMidi + configs[voice].interval
      : mapDiatonicInterval(sourceMidi, config_.keySemitone, config_.scale, configs[voice].interval);
    lastTarget_.targetMidi[voice] = targetMidi;
    lastTarget_.shiftSemitones[voice] = semitonesBetweenMidi(sourceMidi, targetMidi);
  }
  voiceGate_ = std::min(1.0f, voiceGate_ + 0.08f);
}

bool HarmonyEffect::processBackendBlock(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept {
  if (frames != voiceInput_.size() || !lastTarget_.active) return true;
  for (std::size_t index = 0; index < frames; index += 1) {
    voiceInput_[index] = (block.left[offset + index] + block.right[offset + index]) * 0.5f;
  }

  const std::array<HarmonyVoiceConfig, 2> configs{config_.voice1, config_.voice2};
  for (std::size_t voice = 0; voice < voices_.size(); voice += 1) {
    if (!voices_[voice] || !configs[voice].enabled) continue;
    voices_[voice]->setPitchSemitones(lastTarget_.shiftSemitones[voice]);
    if (!voices_[voice]->processBlock(voiceInput_, voiceOutput_[voice])) return false;
    const auto level = decibelsToLinear(configs[voice].levelDb + config_.harmonyLevelDb) * voiceGate_;
    const auto leftGain = panLeft(configs[voice].pan) * level;
    const auto rightGain = panRight(configs[voice].pan) * level;
    for (std::size_t index = 0; index < frames; index += 1) {
      enabledRamp_ += std::clamp(enabledTarget_ - enabledRamp_, -1.0f / 1920.0f, 1.0f / 1920.0f);
      block.left[offset + index] += voiceOutput_[voice][index] * leftGain * enabledRamp_;
      block.right[offset + index] += voiceOutput_[voice][index] * rightGain * enabledRamp_;
    }
  }
  return true;
}

int mapDiatonicInterval(int sourceMidi, int keySemitone, ScaleType scale, int intervalSteps) noexcept {
  const auto degrees = degreesFor(scale);
  const auto relative = sourceMidi - keySemitone;
  const auto octave = floorDiv(relative, 12);
  const auto pitchClass = ((relative % 12) + 12) % 12;
  auto degreeIndex = 0;
  for (std::size_t index = 0; index < degrees.size(); index += 1) {
    if (std::abs(degrees[index] - pitchClass) < std::abs(degrees[static_cast<std::size_t>(degreeIndex)] - pitchClass)) {
      degreeIndex = static_cast<int>(index);
    }
  }
  const auto targetDegree = degreeIndex + intervalSteps;
  const auto targetOctave = octave + floorDiv(targetDegree, static_cast<int>(degrees.size()));
  const auto wrappedDegree = ((targetDegree % static_cast<int>(degrees.size())) + static_cast<int>(degrees.size())) %
    static_cast<int>(degrees.size());
  return keySemitone + targetOctave * 12 + degrees[static_cast<std::size_t>(wrappedDegree)];
}

float panLeft(float pan) noexcept {
  return std::clamp(1.0f - std::clamp(pan, -1.0f, 1.0f), 0.0f, 1.0f);
}

float panRight(float pan) noexcept {
  return std::clamp(1.0f + std::clamp(pan, -1.0f, 1.0f), 0.0f, 1.0f);
}

}  // namespace localmixer::dsp::fx
