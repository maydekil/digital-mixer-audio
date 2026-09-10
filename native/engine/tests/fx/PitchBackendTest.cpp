#include "dsp/fx/PitchBackend.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::fx::PitchBackendSpec;
using localmixer::dsp::fx::makeRubberBandPitchBackend;
using localmixer::dsp::fx::semitonesToRatio;

constexpr double kPi = 3.14159265358979323846;

std::vector<float> sine(double frequency, double sampleRate, std::size_t frames) {
  std::vector<float> samples(frames);
  for (std::size_t index = 0; index < frames; index += 1) {
    samples[index] = static_cast<float>(std::sin(2.0 * kPi * frequency * static_cast<double>(index) / sampleRate) * 0.5);
  }
  return samples;
}

double estimateFrequency(const std::vector<float>& samples, double sampleRate, std::size_t start, std::size_t count) {
  const auto end = std::min(samples.size(), start + count);
  if (end <= start + 256) return 0.0;
  double bestScore = -1.0;
  std::size_t bestLag = 0;
  const auto minLag = static_cast<std::size_t>(sampleRate / 900.0);
  const auto maxLag = static_cast<std::size_t>(sampleRate / 70.0);
  for (auto lag = minLag; lag <= maxLag; lag += 1) {
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

bool runShift(float semitones, double expectedHz) {
  auto backend = makeRubberBandPitchBackend();
  if (!backend->prepare(PitchBackendSpec{.sampleRate = 48000.0, .channels = 1, .preserveFormants = true})) return false;
  backend->setPitchSemitones(semitones);
  backend->setFormantSemitones(0.0f);
  const auto blockSize = backend->blockSize();
  if (blockSize == 0 || backend->startDelaySamples() == 0) return false;

  const auto input = sine(220.0, 48000.0, blockSize * 180);
  std::vector<float> output(input.size(), 0.0f);
  std::vector<float> blockIn(blockSize);
  std::vector<float> blockOut(blockSize);
  for (std::size_t offset = 0; offset < input.size(); offset += blockSize) {
    std::copy(input.begin() + static_cast<std::ptrdiff_t>(offset),
      input.begin() + static_cast<std::ptrdiff_t>(offset + blockSize),
      blockIn.begin());
    if (!backend->processBlock(blockIn, blockOut)) return false;
    std::copy(blockOut.begin(), blockOut.end(), output.begin() + static_cast<std::ptrdiff_t>(offset));
  }

  const auto start = static_cast<std::size_t>(backend->startDelaySamples()) + blockSize * 12;
  const auto actual = estimateFrequency(output, 48000.0, start, blockSize * 80);
  return std::fabs(centsError(actual, expectedHz)) <= 10.0;
}

}  // namespace

int main() {
  if (std::fabs(semitonesToRatio(12.0f) - 2.0) > 0.0001 || std::fabs(semitonesToRatio(-12.0f) - 0.5) > 0.0001) {
    std::cerr << "semitone ratio conversion mismatch\n";
    return 1;
  }

  if (!runShift(12.0f, 440.0)) {
    std::cerr << "Rubber Band backend should shift 220 Hz up to 440 Hz within 10 cents\n";
    return 1;
  }

  if (!runShift(-12.0f, 110.0)) {
    std::cerr << "Rubber Band backend should shift 220 Hz down to 110 Hz within 10 cents\n";
    return 1;
  }

  std::cout << "local-mixer-pitch-backend-tests ok\n";
  return 0;
}
