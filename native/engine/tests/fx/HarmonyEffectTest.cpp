#include "dsp/fx/HarmonyEffect.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::HarmonyConfig;
using localmixer::dsp::fx::HarmonyEffect;
using localmixer::dsp::fx::HarmonyMode;
using localmixer::dsp::fx::HarmonyVoiceConfig;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::ScaleType;
using localmixer::dsp::fx::mapDiatonicInterval;
using localmixer::dsp::fx::panLeft;
using localmixer::dsp::fx::panRight;

constexpr double kPi = 3.14159265358979323846;

std::vector<float> sine(double frequency, double sampleRate, std::size_t frames) {
  std::vector<float> samples(frames);
  for (std::size_t index = 0; index < frames; index += 1) {
    samples[index] = static_cast<float>(std::sin(2.0 * kPi * frequency * static_cast<double>(index) / sampleRate) * 0.25);
  }
  return samples;
}

float rmsDelta(const std::vector<float>& a, const std::vector<float>& b, std::size_t start) {
  double sum = 0.0;
  auto count = 0U;
  for (auto index = start; index < a.size() && index < b.size(); index += 1) {
    const auto delta = static_cast<double>(a[index] - b[index]);
    sum += delta * delta;
    count += 1;
  }
  return count == 0 ? 0.0f : static_cast<float>(std::sqrt(sum / count));
}

float rms(const std::vector<float>& samples, std::size_t start) {
  double sum = 0.0;
  auto count = 0U;
  for (auto index = start; index < samples.size(); index += 1) {
    sum += samples[index] * samples[index];
    count += 1;
  }
  return count == 0 ? 0.0f : static_cast<float>(std::sqrt(sum / count));
}

bool testIntervals() {
  return mapDiatonicInterval(60, 0, ScaleType::major, 2) == 64 &&
    mapDiatonicInterval(64, 0, ScaleType::major, 2) == 67 &&
    mapDiatonicInterval(60, 0, ScaleType::naturalMinor, 2) == 63 &&
    mapDiatonicInterval(71, 0, ScaleType::major, 2) == 74 &&
    panLeft(-1.0f) == 1.0f && panRight(-1.0f) == 0.0f &&
    panLeft(1.0f) == 0.0f && panRight(1.0f) == 1.0f;
}

bool testHarmonyProcessing() {
  HarmonyEffect effect(HarmonyConfig{
    .mode = HarmonyMode::diatonic,
    .keySemitone = 0,
    .scale = ScaleType::major,
    .a4Hz = 440.0f,
    .confidenceThreshold = 0.75f,
    .preserveFormants = true,
    .voice1 = HarmonyVoiceConfig{.enabled = true, .interval = 2, .levelDb = -6.0f, .pan = -1.0f},
    .voice2 = HarmonyVoiceConfig{.enabled = true, .interval = 4, .levelDb = -6.0f, .pan = 1.0f},
  });
  constexpr auto blockFrames = std::size_t{2048};
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  const auto input = sine(261.625565, 48000.0, blockFrames * 80);
  auto left = input;
  auto right = input;
  for (std::size_t offset = 0; offset < input.size(); offset += blockFrames) {
    AudioBlockView block{
      std::span<float>(left.data() + offset, blockFrames),
      std::span<float>(right.data() + offset, blockFrames),
    };
    effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = static_cast<std::uint64_t>(offset)});
  }

  const auto start = static_cast<std::size_t>(effect.latencySamples()) + blockFrames * 20;
  const auto target = effect.lastTarget();
  const auto stereoDelta = rmsDelta(left, right, start);
  const auto leftRms = rms(left, start);
  const auto rightRms = rms(right, start);
  const auto expectedVoice1 = mapDiatonicInterval(target.sourceMidi, 0, ScaleType::major, 2);
  const auto expectedVoice2 = mapDiatonicInterval(target.sourceMidi, 0, ScaleType::major, 4);
  const auto ok = target.active && target.targetMidi[0] == expectedVoice1 && target.targetMidi[1] == expectedVoice2 &&
    stereoDelta > 0.001f && leftRms > 0.01f && rightRms > 0.01f;
  if (!ok) {
    std::cerr << "target active=" << target.active << " source=" << target.sourceMidi
              << " voice1=" << target.targetMidi[0] << " voice2=" << target.targetMidi[1]
              << " stereoDelta=" << stereoDelta << " leftRms=" << leftRms << " rightRms=" << rightRms << "\n";
  }
  return ok;
}

bool testSilenceGate() {
  HarmonyEffect effect;
  constexpr auto blockFrames = std::size_t{2048};
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  std::vector<float> left(blockFrames * 8, 0.0f);
  std::vector<float> right(blockFrames * 8, 0.0f);
  for (std::size_t offset = 0; offset < left.size(); offset += blockFrames) {
    AudioBlockView block{
      std::span<float>(left.data() + offset, blockFrames),
      std::span<float>(right.data() + offset, blockFrames),
    };
    effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = static_cast<std::uint64_t>(offset)});
  }
  return !effect.lastTarget().active && rms(left, 0) == 0.0f && rms(right, 0) == 0.0f;
}

bool testVoiceOnlyDisableKeepsLead() {
  HarmonyEffect effect(HarmonyConfig{.enabled = false});
  constexpr auto blockFrames = std::size_t{2048};
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  auto left = sine(261.625565, 48000.0, blockFrames * 20);
  auto right = left;
  const auto original = left;
  for (std::size_t offset = 0; offset < left.size(); offset += blockFrames) {
    AudioBlockView block{
      std::span<float>(left.data() + offset, blockFrames),
      std::span<float>(right.data() + offset, blockFrames),
    };
    effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = static_cast<std::uint64_t>(offset)});
  }
  return rmsDelta(left, original, 0) < 0.0001f && rmsDelta(right, original, 0) < 0.0001f;
}

bool testHarmonyLevelDoesNotTrimLead() {
  HarmonyEffect effect(HarmonyConfig{.enabled = false, .harmonyLevelDb = -30.0f});
  constexpr auto blockFrames = std::size_t{2048};
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  auto left = sine(329.627557, 48000.0, blockFrames * 4);
  auto right = left;
  const auto original = left;
  AudioBlockView block{std::span<float>(left.data(), blockFrames), std::span<float>(right.data(), blockFrames)};
  effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = 0});
  return rmsDelta(left, original, 0) < 0.0001f && rmsDelta(right, original, 0) < 0.0001f;
}

}  // namespace

int main() {
  if (!testIntervals()) {
    std::cerr << "harmony interval and pan helpers should match expected mapping\n";
    return 1;
  }
  if (!testHarmonyProcessing()) {
    std::cerr << "harmony should derive C major third/fifth voices and produce panned output\n";
    return 1;
  }
  if (!testSilenceGate()) {
    std::cerr << "harmony should keep silence silent and avoid false targets\n";
    return 1;
  }
  if (!testVoiceOnlyDisableKeepsLead()) {
    std::cerr << "disabled harmony should keep lead unchanged while voices are silent\n";
    return 1;
  }
  if (!testHarmonyLevelDoesNotTrimLead()) {
    std::cerr << "harmony level should not trim lead signal\n";
    return 1;
  }
  std::cout << "local-mixer-harmony-effect-tests ok\n";
  return 0;
}
