#include "dsp/fx/VocoderEffect.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::VocoderCarrierMode;
using localmixer::dsp::fx::VocoderConfig;
using localmixer::dsp::fx::VocoderEffect;

constexpr double kPi = 3.14159265358979323846;

std::vector<float> sine(double frequency, double sampleRate, std::size_t frames) {
  std::vector<float> samples(frames);
  for (std::size_t index = 0; index < frames; index += 1) {
    samples[index] = static_cast<float>(std::sin(2.0 * kPi * frequency * static_cast<double>(index) / sampleRate) * 0.5);
  }
  return samples;
}

float rms(const std::vector<float>& samples, std::size_t start = 0) {
  double sum = 0.0;
  auto count = 0U;
  for (auto index = start; index < samples.size(); index += 1) {
    if (!std::isfinite(samples[index])) return -1.0f;
    sum += samples[index] * samples[index];
    count += 1;
  }
  return count == 0 ? 0.0f : static_cast<float>(std::sqrt(sum / count));
}

double estimateFrequency(const std::vector<float>& samples, double sampleRate, std::size_t start, std::size_t count) {
  const auto end = std::min(samples.size(), start + count);
  double bestScore = -1.0;
  std::size_t bestLag = 0;
  for (auto lag = static_cast<std::size_t>(sampleRate / 600.0); lag <= static_cast<std::size_t>(sampleRate / 60.0); lag += 1) {
    double score = 0.0;
    for (auto index = start + lag; index < end; index += 1) score += samples[index] * samples[index - lag];
    if (score > bestScore) {
      bestScore = score;
      bestLag = lag;
    }
  }
  return bestLag == 0 ? 0.0 : sampleRate / static_cast<double>(bestLag);
}

void render(VocoderEffect& effect, std::vector<float>& left, std::vector<float>& right, std::size_t blockFrames) {
  for (std::size_t offset = 0; offset < left.size(); offset += blockFrames) {
    AudioBlockView block{
      std::span<float>(left.data() + offset, blockFrames),
      std::span<float>(right.data() + offset, blockFrames),
    };
    effect.process(block, ProcessContext{.sampleRate = 48000.0, .absoluteFrame = static_cast<std::uint64_t>(offset)});
  }
}

bool testSilence() {
  VocoderEffect effect;
  constexpr auto blockFrames = std::size_t{256};
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  std::vector<float> left(blockFrames * 16, 0.0f);
  std::vector<float> right(left.size(), 0.0f);
  render(effect, left, right, blockFrames);
  return rms(left) == 0.0f && rms(right) == 0.0f;
}

bool testFixedCarrierNoteChangesColor() {
  constexpr auto blockFrames = std::size_t{256};
  auto leftA = sine(800.0, 48000.0, blockFrames * 96);
  auto rightA = leftA;
  auto leftB = leftA;
  auto rightB = rightA;
  VocoderEffect low(VocoderConfig{.carrierMode = VocoderCarrierMode::fixed, .fixedMidiNote = 48, .outputDb = -3.0f});
  VocoderEffect high(VocoderConfig{.carrierMode = VocoderCarrierMode::fixed, .fixedMidiNote = 60, .outputDb = -3.0f});
  low.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  high.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  render(low, leftA, rightA, blockFrames);
  render(high, leftB, rightB, blockFrames);
  const auto start = blockFrames * 20;
  const auto lowHz = estimateFrequency(leftA, 48000.0, start, blockFrames * 48);
  const auto highHz = estimateFrequency(leftB, 48000.0, start, blockFrames * 48);
  return rms(leftA, start) > 0.001f && rms(leftB, start) > 0.001f && highHz > lowHz * 1.7;
}

bool testMidiNotesAndPanic() {
  constexpr auto blockFrames = std::size_t{256};
  VocoderEffect effect(VocoderConfig{.carrierMode = VocoderCarrierMode::midi, .unvoicedPct = 0.0f, .outputDb = -3.0f});
  effect.prepare(ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = blockFrames, .channels = 1});
  effect.noteOn(60);
  effect.noteOn(64);
  auto left = sine(1000.0, 48000.0, blockFrames * 32);
  auto right = left;
  render(effect, left, right, blockFrames);
  const auto noteRms = rms(left, blockFrames * 8);
  if (noteRms <= 0.001f) {
    std::cerr << "noteRms=" << noteRms << "\n";
    return false;
  }
  effect.allNotesOff();
  left = sine(1000.0, 48000.0, blockFrames * 8);
  right = left;
  render(effect, left, right, blockFrames);
  const auto offRms = rms(left);
  if (offRms > 0.00001f) {
    std::cerr << "offRms=" << offRms << "\n";
    return false;
  }
  effect.noteOn(60);
  effect.panic();
  left = sine(1000.0, 48000.0, blockFrames * 8);
  right = left;
  render(effect, left, right, blockFrames);
  const auto panicRms = rms(left);
  if (panicRms > 0.00001f) std::cerr << "panicRms=" << panicRms << "\n";
  return panicRms <= 0.00001f;
}

}  // namespace

int main() {
  if (!testSilence()) {
    std::cerr << "vocoder wet output should remain silent when modulator is silent\n";
    return 1;
  }
  if (!testFixedCarrierNoteChangesColor()) {
    std::cerr << "fixed carrier MIDI note should change vocoder output pitch/color\n";
    return 1;
  }
  if (!testMidiNotesAndPanic()) {
    std::cerr << "MIDI carrier notes and panic should gate/stabilize vocoder output\n";
    return 1;
  }
  std::cout << "local-mixer-vocoder-effect-tests ok\n";
  return 0;
}
