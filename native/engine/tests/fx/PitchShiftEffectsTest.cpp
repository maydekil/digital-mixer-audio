#include "dsp/fx/PitchShiftEffects.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

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

}  // namespace

int main() {
  constexpr auto sampleRate = 48000.0;
  constexpr std::size_t blockFrames = 2048;
  constexpr std::size_t blockCount = 120;
  localmixer::dsp::fx::PitchShiftEffect pitch(localmixer::dsp::fx::PitchShiftConfig{.semitones = 12.0f});
  pitch.prepare(localmixer::dsp::fx::ProcessSpec{.sampleRate = sampleRate, .maximumBlockFrames = blockFrames, .channels = 1});

  auto left = sine(220.0, sampleRate, blockFrames * blockCount);
  auto right = left;
  for (std::size_t offset = 0; offset < left.size(); offset += blockFrames) {
    auto block = localmixer::dsp::fx::AudioBlockView{
      .left = std::span<float>(left.data() + offset, blockFrames),
      .right = std::span<float>(right.data() + offset, blockFrames),
    };
    pitch.process(block, localmixer::dsp::fx::ProcessContext{.sampleRate = sampleRate, .absoluteFrame = offset});
  }

  const auto actual = estimateFrequency(left, sampleRate, pitch.latencySamples() + blockFrames * 24, blockFrames * 60);
  if (std::fabs(centsError(actual, 440.0)) > 10.0) {
    std::cerr << "pitch shift effect should move 220 Hz to 440 Hz, actual=" << actual << "\n";
    return 1;
  }

  localmixer::dsp::fx::FormantShiftEffect formant(2.0f);
  formant.prepare(localmixer::dsp::fx::ProcessSpec{.sampleRate = sampleRate, .maximumBlockFrames = blockFrames, .channels = 1});
  if (formant.latencySamples() == 0) {
    std::cerr << "formant shift wrapper should expose backend latency\n";
    return 1;
  }

  std::cout << "local-mixer-pitch-shift-effects-tests ok\n";
  return 0;
}
