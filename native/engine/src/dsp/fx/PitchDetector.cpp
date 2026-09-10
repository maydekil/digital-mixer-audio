#include "dsp/fx/PitchDetector.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace localmixer::dsp::fx {
namespace {

constexpr float kInvalidScore = std::numeric_limits<float>::max();

bool isAllowedScaleDegree(int semitone, ScaleType scale) {
  if (scale == ScaleType::chromatic) return true;
  constexpr std::array<bool, 12> kMajor{true, false, true, false, true, true, false, true, false, true, false, true};
  constexpr std::array<bool, 12> kMinor{true, false, true, true, false, true, false, true, true, false, true, false};
  const auto index = ((semitone % 12) + 12) % 12;
  return scale == ScaleType::major ? kMajor[static_cast<std::size_t>(index)] : kMinor[static_cast<std::size_t>(index)];
}

float centsBetween(float fromHz, float toHz) {
  if (fromHz <= 0.0f || toHz <= 0.0f) return 0.0f;
  return 1200.0f * std::log2(toHz / fromHz);
}

}  // namespace

void PitchDetector::prepare(const PitchDetectorConfig& config) {
  config_ = config;
  const auto maxTau = static_cast<std::size_t>(std::ceil(config_.sampleRate / config_.minFrequencyHz));
  difference_.assign(maxTau + 1, 0.0f);
  cmndf_.assign(maxTau + 1, 1.0f);
}

PitchEstimate PitchDetector::analyze(std::span<const float> monoWindow, std::uint64_t startFrame) noexcept {
  PitchEstimate estimate;
  estimate.centerFrame = startFrame + monoWindow.size() / 2;
  if (monoWindow.size() < config_.windowFrames || difference_.empty()) return estimate;

  const auto minTau = std::max<std::size_t>(2, static_cast<std::size_t>(config_.sampleRate / config_.maxFrequencyHz));
  const auto maxTau = std::min<std::size_t>(difference_.size() - 1, static_cast<std::size_t>(config_.sampleRate / config_.minFrequencyHz));

  for (std::size_t tau = 1; tau <= maxTau; tau += 1) {
    float sum = 0.0f;
    for (std::size_t index = 0; index + tau < config_.windowFrames; index += 1) {
      const auto delta = monoWindow[index] - monoWindow[index + tau];
      sum += delta * delta;
    }
    difference_[tau] = sum;
  }

  float running = 0.0f;
  auto bestTau = minTau;
  auto bestScore = kInvalidScore;
  cmndf_[0] = 1.0f;
  for (std::size_t tau = 1; tau <= maxTau; tau += 1) {
    running += difference_[tau];
    cmndf_[tau] = running > 0.0f ? difference_[tau] * static_cast<float>(tau) / running : 1.0f;
    if (tau < minTau) continue;
    if (cmndf_[tau] < bestScore) {
      bestScore = cmndf_[tau];
      bestTau = tau;
    }
  }

  if (bestScore >= 1.0f) return estimate;
  estimate.frequencyHz = static_cast<float>(config_.sampleRate / static_cast<double>(bestTau));
  estimate.confidence = std::clamp(1.0f - bestScore, 0.0f, 1.0f);
  estimate.voiced = bestScore <= config_.threshold && estimate.frequencyHz >= config_.minFrequencyHz && estimate.frequencyHz <= config_.maxFrequencyHz;
  return estimate;
}

float midiToFrequency(int midi, float a4Hz) noexcept {
  return a4Hz * std::pow(2.0f, (static_cast<float>(midi) - 69.0f) / 12.0f);
}

float frequencyToMidi(float frequencyHz, float a4Hz) noexcept {
  if (frequencyHz <= 0.0f || a4Hz <= 0.0f) return 0.0f;
  return 69.0f + 12.0f * std::log2(frequencyHz / a4Hz);
}

PitchTarget mapPitchToScale(float frequencyHz, const ScaleMapperConfig& config) noexcept {
  PitchTarget target;
  if (frequencyHz <= 0.0f || config.a4Hz <= 0.0f) return target;

  const auto midiFloat = frequencyToMidi(frequencyHz, config.a4Hz);
  target.inputMidi = static_cast<int>(std::lround(midiFloat));

  auto bestMidi = target.inputMidi;
  auto bestDistance = kInvalidScore;
  for (auto candidate = target.inputMidi - 12; candidate <= target.inputMidi + 12; candidate += 1) {
    const auto relative = candidate - config.keySemitone;
    if (!isAllowedScaleDegree(relative, config.scale)) continue;
    const auto distance = std::fabs(static_cast<float>(candidate) - midiFloat);
    if (distance < bestDistance || (distance == bestDistance && candidate < bestMidi)) {
      bestDistance = distance;
      bestMidi = candidate;
    }
  }

  const auto targetHz = midiToFrequency(bestMidi, config.a4Hz);
  target.active = true;
  target.targetMidi = bestMidi;
  target.detuneCents = centsBetween(targetHz, frequencyHz);
  if (std::fabs(target.detuneCents) <= config.toleranceCents) {
    target.correctionSemitones = 0.0f;
  } else {
    target.correctionSemitones = -target.detuneCents * std::clamp(config.amount, 0.0f, 1.0f) / 100.0f;
  }
  return target;
}

}  // namespace localmixer::dsp::fx
