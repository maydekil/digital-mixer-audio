#include "dsp/DeEsser.hpp"

#include "dsp/Dynamics.hpp"
#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp {
namespace {

float coefficient(double sampleRate, float timeMs) {
  if (sampleRate <= 0.0 || timeMs <= 0.0f) return 0.0f;
  return std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
}

}  // namespace

void DeEsser::configure(const DeEsserConfig& config) {
  config_ = config;
  config_.detectorFrequencyHz = std::clamp(config_.detectorFrequencyHz, 2000.0f, 12000.0f);
  config_.maxReductionDb = std::clamp(config_.maxReductionDb, 0.0f, 12.0f);
  detectorCoefficients_ = makeBiquad(EqBandConfig{
    .type = EqFilterType::highPass,
    .sampleRate = config_.sampleRate,
    .frequencyHz = config_.detectorFrequencyHz,
    .q = 0.70710678,
  });
  resetDetectorStates();
}

void DeEsser::reset() {
  resetDetectorStates();
  reductionDb_ = 0.0f;
}

void DeEsser::processMono(std::span<float> samples) {
  if (!config_.enabled) return;
  for (auto& sample : samples) {
    const auto detector = std::fabs(detectSample(sample, 0));
    const auto reduction = smoothedReduction(linearToDecibels(detector));
    sample *= decibelsToLinear(-reduction);
  }
}

void DeEsser::processInterleavedLinked(std::span<float> samples, std::size_t channelCount) {
  if (!config_.enabled || channelCount == 0) return;
  const auto safeChannels = std::min(channelCount, detectorStates_.size());
  const auto frameCount = samples.size() / channelCount;
  for (std::size_t frame = 0; frame < frameCount; frame += 1) {
    const auto offset = frame * channelCount;
    float detector = 0.0f;
    for (std::size_t channel = 0; channel < safeChannels; channel += 1) {
      detector = std::max(detector, std::fabs(detectSample(samples[offset + channel], channel)));
    }
    const auto gain = decibelsToLinear(-smoothedReduction(linearToDecibels(detector)));
    for (std::size_t channel = 0; channel < channelCount; channel += 1) {
      samples[offset + channel] *= gain;
    }
  }
}

void DeEsser::renderDetectorAuditionMono(std::span<const float> input, std::span<float> output) {
  const auto count = std::min(input.size(), output.size());
  BiquadState auditionState;
  for (std::size_t index = 0; index < count; index += 1) {
    output[index] = runDetector(input[index], auditionState);
  }
}

float DeEsser::lastReductionDb() const {
  return reductionDb_;
}

float DeEsser::detectSample(float sample, std::size_t channel) {
  return runDetector(sample, detectorStates_[channel % detectorStates_.size()]);
}

float DeEsser::smoothedReduction(float detectorDb) {
  const auto target = std::clamp(detectorDb - config_.thresholdDb, 0.0f, config_.maxReductionDb);
  const auto coeff = target > reductionDb_ ? coefficient(config_.sampleRate, config_.attackMs)
                                           : coefficient(config_.sampleRate, config_.releaseMs);
  reductionDb_ = coeff * reductionDb_ + (1.0f - coeff) * target;
  return reductionDb_;
}

float DeEsser::runDetector(float sample, BiquadState& state) const {
  const auto input = static_cast<double>(sample);
  const auto output = detectorCoefficients_.b0 * input + detectorCoefficients_.b1 * state.x1 +
    detectorCoefficients_.b2 * state.x2 - detectorCoefficients_.a1 * state.y1 - detectorCoefficients_.a2 * state.y2;
  state.x2 = state.x1;
  state.x1 = input;
  state.y2 = state.y1;
  state.y1 = output;
  return static_cast<float>(output);
}

void DeEsser::resetDetectorStates() {
  for (auto& state : detectorStates_) state = {};
}

}  // namespace localmixer::dsp
