#include "dsp/fx/PitchCorrectionEffect.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::PitchCorrectionConfig;
using localmixer::dsp::fx::PitchCorrectionEffect;
using localmixer::dsp::fx::PitchDetector;
using localmixer::dsp::fx::PitchDetectorConfig;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::ScaleMapperConfig;
using localmixer::dsp::fx::ScaleType;
using localmixer::dsp::fx::mapPitchToScale;

constexpr double kPi = 3.14159265358979323846;

std::vector<float> sine(double frequency, double sampleRate, std::size_t frames) {
  std::vector<float> samples(frames);
  for (std::size_t index = 0; index < frames; index += 1) {
    samples[index] = static_cast<float>(std::sin(2.0 * kPi * frequency * static_cast<double>(index) / sampleRate) * 0.4);
  }
  return samples;
}

double estimateFrequency(const std::vector<float>& samples, double sampleRate, std::size_t start, std::size_t count) {
  const auto end = std::min(samples.size(), start + count);
  double bestScore = -1.0;
  std::size_t bestLag = 0;
  for (auto lag = static_cast<std::size_t>(sampleRate / 900.0); lag <= static_cast<std::size_t>(sampleRate / 70.0); lag += 1) {
    double score = 0.0;
    for (auto index = start + lag; index < end; index += 1) {
      score += samples[index] * samples[index - lag];
    }
    if (score > bestScore) {
      bestScore = score;
      bestLag = lag;
    }
  }
  return bestLag == 0 ? 0.0 : sampleRate / static_cast<double>(bestLag);
}

double centsError(double actual, double expected) {
  return 1200.0 * std::log2(actual / expected);
}

bool testDetector() {
  PitchDetector detector;
  detector.prepare(PitchDetectorConfig{.sampleRate = 48000.0});
  const auto input = sine(220.0, 48000.0, detector.windowFrames());
  const auto estimate = detector.analyze(input, 0);
  if (!estimate.voiced || estimate.confidence < 0.85f || std::fabs(centsError(estimate.frequencyHz, 220.0)) > 10.0) {
    std::cerr << "detector estimate voiced=" << estimate.voiced << " frequency=" << estimate.frequencyHz
              << " confidence=" << estimate.confidence << "\n";
    return false;
  }
  std::vector<float> silence(detector.windowFrames(), 0.0f);
  return !detector.analyze(silence, 0).voiced;
}

bool testScaleMapper() {
  const auto sharpA = static_cast<float>(440.0 * std::pow(2.0, 35.0 / 1200.0));
  const auto mapped = mapPitchToScale(sharpA, ScaleMapperConfig{
    .keySemitone = 0,
    .scale = ScaleType::chromatic,
    .a4Hz = 440.0f,
    .toleranceCents = 10.0f,
    .amount = 1.0f,
  });
  return mapped.active && mapped.targetMidi == 69 && std::fabs(mapped.detuneCents - 35.0f) < 5.0f &&
    std::fabs(mapped.correctionSemitones + 0.35f) < 0.05f;
}

bool testCorrectionEffect() {
  PitchCorrectionEffect effect(PitchCorrectionConfig{
    .keySemitone = 0,
    .scale = ScaleType::chromatic,
    .a4Hz = 440.0f,
    .retuneMs = 0.0f,
    .amount = 1.0f,
    .toleranceCents = 0.0f,
    .confidenceThreshold = 0.75f,
  });
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = 2048, .channels = 1});
  const auto blockFrames = static_cast<std::size_t>(2048);
  const auto inputHz = 440.0 * std::pow(2.0, 35.0 / 1200.0);
  const auto input = sine(inputHz, 48000.0, blockFrames * 120);
  std::vector<float> left(input.size(), 0.0f);
  std::vector<float> right(input.size(), 0.0f);
  for (std::size_t offset = 0; offset < input.size(); offset += blockFrames) {
    std::copy(input.begin() + static_cast<std::ptrdiff_t>(offset),
      input.begin() + static_cast<std::ptrdiff_t>(offset + blockFrames),
      left.begin() + static_cast<std::ptrdiff_t>(offset));
    std::copy(left.begin() + static_cast<std::ptrdiff_t>(offset),
      left.begin() + static_cast<std::ptrdiff_t>(offset + blockFrames),
      right.begin() + static_cast<std::ptrdiff_t>(offset));
    AudioBlockView block{
      std::span<float>(left.data() + offset, blockFrames),
      std::span<float>(right.data() + offset, blockFrames),
    };
    effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = static_cast<std::uint64_t>(offset)});
  }
  const auto start = effect.latencySamples() + blockFrames * 30;
  const auto actual = estimateFrequency(left, 48000.0, start, blockFrames * 50);
  return std::fabs(centsError(actual, 440.0)) <= 10.0 && effect.lastEstimate().voiced && effect.lastTarget().active;
}

}  // namespace

int main() {
  if (!testDetector()) {
    std::cerr << "YIN pitch detector should detect 220 Hz and ignore silence\n";
    return 1;
  }
  if (!testScaleMapper()) {
    std::cerr << "scale mapper should target chromatic A4 and compute correction amount\n";
    return 1;
  }
  if (!testCorrectionEffect()) {
    std::cerr << "pitch correction effect should settle a sharp A4 within 10 cents of target\n";
    return 1;
  }
  std::cout << "local-mixer-pitch-correction-tests ok\n";
  return 0;
}
