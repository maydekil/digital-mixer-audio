#include "dsp/DeEsser.hpp"
#include "dsp/Dynamics.hpp"
#include "dsp/Gain.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::DeEsser;
using localmixer::dsp::DeEsserConfig;
using localmixer::dsp::decibelsToLinear;
using localmixer::dsp::linearToDecibels;

constexpr float kPi = 3.14159265358979323846f;

std::vector<float> sine(float frequency, float levelDb, std::size_t count = 48000) {
  std::vector<float> samples(count);
  const auto amplitude = decibelsToLinear(levelDb);
  for (std::size_t index = 0; index < samples.size(); index += 1) {
    const auto phase = 2.0f * kPi * frequency * static_cast<float>(index) / 48000.0f;
    samples[index] = std::sin(phase) * amplitude;
  }
  return samples;
}

float peakDb(const std::vector<float>& samples) {
  float peak = 0.0f;
  for (const auto sample : samples) peak = std::max(peak, std::fabs(sample));
  return linearToDecibels(peak);
}

}  // namespace

int main() {
  DeEsser deEsser;
  deEsser.configure(DeEsserConfig{
    .detectorFrequencyHz = 6000.0f,
    .thresholdDb = -30.0f,
    .maxReductionDb = 6.0f,
    .attackMs = 0.0f,
    .releaseMs = 20.0f,
  });

  auto highBurst = sine(8000.0f, -10.0f);
  deEsser.processMono(highBurst);
  if (deEsser.lastReductionDb() < 5.0f || peakDb(highBurst) > -15.0f) {
    std::cerr << "de-esser should reduce high-frequency burst above threshold\n";
    return 1;
  }

  deEsser.reset();
  auto lowTone = sine(500.0f, -10.0f);
  deEsser.processMono(lowTone);
  if (deEsser.lastReductionDb() > 1.0f || peakDb(lowTone) < -12.0f) {
    std::cerr << "de-esser should not over-reduce low-frequency content\n";
    return 1;
  }

  deEsser.reset();
  std::vector<float> stereo;
  stereo.reserve(4096);
  for (std::size_t index = 0; index < 2048; index += 1) {
    stereo.push_back(std::sin(2.0f * kPi * 8000.0f * static_cast<float>(index) / 48000.0f) * decibelsToLinear(-10.0f));
    stereo.push_back(decibelsToLinear(-24.0f));
  }
  deEsser.processInterleavedLinked(stereo, 2);
  if (linearToDecibels(std::fabs(stereo.back())) > -28.0f) {
    std::cerr << "linked de-esser should apply detector reduction to both stereo channels\n";
    return 1;
  }

  deEsser.reset();
  const auto source = sine(8000.0f, -12.0f, 256);
  std::vector<float> monitor(source.size(), 0.0f);
  deEsser.renderDetectorAuditionMono(source, monitor);
  if (source[8] == monitor[8]) {
    std::cerr << "detector audition should render a separate monitor signal\n";
    return 1;
  }

  std::cout << "local-mixer-de-esser-tests ok\n";
  return 0;
}
