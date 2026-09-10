#include "dsp/Dynamics.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp {
namespace {

constexpr float kSilenceDb = -120.0f;

float coefficient(double sampleRate, float timeMs) {
  if (sampleRate <= 0.0 || timeMs <= 0.0f) return 0.0f;
  return std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
}

float maxFrameAbs(std::span<float> samples, std::size_t frame, std::size_t channelCount) {
  float maximum = 0.0f;
  const auto offset = frame * channelCount;
  for (std::size_t channel = 0; channel < channelCount; channel += 1) {
    maximum = std::max(maximum, std::fabs(samples[offset + channel]));
  }
  return maximum;
}

}  // namespace

float linearToDecibels(float value) {
  const auto magnitude = std::max(std::fabs(value), 0.000001f);
  return std::max(kSilenceDb, 20.0f * std::log10(magnitude));
}

void Compressor::configure(const CompressorConfig& config) {
  config_ = config;
  config_.ratio = std::max(1.0f, config_.ratio);
  config_.kneeDb = std::max(0.0f, config_.kneeDb);
}

void Compressor::reset() {
  smoothedGainDb_ = 0.0f;
  lastGainReductionDb_ = 0.0f;
}

void Compressor::processMono(std::span<float> samples) {
  if (!config_.enabled) return;
  for (auto& sample : samples) {
    const auto gainDb = smoothedGain(gainForLevel(linearToDecibels(sample)));
    sample *= decibelsToLinear(gainDb + config_.makeupGainDb);
  }
}

void Compressor::processInterleavedLinked(std::span<float> samples, std::size_t channelCount) {
  if (!config_.enabled || channelCount == 0) return;
  const auto frameCount = samples.size() / channelCount;
  for (std::size_t frame = 0; frame < frameCount; frame += 1) {
    const auto gainDb = smoothedGain(gainForLevel(linearToDecibels(maxFrameAbs(samples, frame, channelCount))));
    const auto gain = decibelsToLinear(gainDb + config_.makeupGainDb);
    const auto offset = frame * channelCount;
    for (std::size_t channel = 0; channel < channelCount; channel += 1) {
      samples[offset + channel] *= gain;
    }
  }
}

float Compressor::lastGainReductionDb() const {
  return lastGainReductionDb_;
}

float Compressor::gainForLevel(float levelDb) const {
  const auto threshold = config_.thresholdDb;
  const auto ratioSlope = (1.0f / config_.ratio) - 1.0f;
  if (config_.kneeDb <= 0.0f) {
    if (levelDb <= threshold) return 0.0f;
    return (threshold + (levelDb - threshold) / config_.ratio) - levelDb;
  }

  const auto kneeHalf = config_.kneeDb * 0.5f;
  const auto distance = levelDb - threshold;
  if (distance <= -kneeHalf) return 0.0f;
  if (distance >= kneeHalf) return ratioSlope * distance;
  const auto kneePosition = distance + kneeHalf;
  return ratioSlope * kneePosition * kneePosition / (2.0f * config_.kneeDb);
}

float Compressor::smoothedGain(float targetGainDb) {
  const auto coeff = targetGainDb < smoothedGainDb_ ? coefficient(config_.sampleRate, config_.attackMs)
                                                    : coefficient(config_.sampleRate, config_.releaseMs);
  smoothedGainDb_ = coeff * smoothedGainDb_ + (1.0f - coeff) * targetGainDb;
  lastGainReductionDb_ = -std::min(0.0f, smoothedGainDb_);
  return smoothedGainDb_;
}

void NoiseGate::configure(const NoiseGateConfig& config) {
  config_ = config;
  config_.hysteresisDb = std::max(0.0f, config_.hysteresisDb);
  config_.ratio = std::max(1.0f, config_.ratio);
  config_.rangeDb = std::min(0.0f, config_.rangeDb);
  attenuationDb_ = config_.rangeDb;
}

void NoiseGate::reset() {
  open_ = false;
  holdFramesRemaining_ = 0;
  attenuationDb_ = config_.rangeDb;
}

void NoiseGate::processMono(std::span<float> samples) {
  if (!config_.enabled) return;
  for (auto& sample : samples) {
    const auto levelDb = linearToDecibels(sample);
    updateOpenState(levelDb);
    sample *= decibelsToLinear(smoothedAttenuation(targetAttenuation(levelDb)));
  }
}

void NoiseGate::processInterleavedLinked(std::span<float> samples, std::size_t channelCount) {
  if (!config_.enabled || channelCount == 0) return;
  const auto frameCount = samples.size() / channelCount;
  for (std::size_t frame = 0; frame < frameCount; frame += 1) {
    const auto levelDb = linearToDecibels(maxFrameAbs(samples, frame, channelCount));
    updateOpenState(levelDb);
    const auto gain = decibelsToLinear(smoothedAttenuation(targetAttenuation(levelDb)));
    const auto offset = frame * channelCount;
    for (std::size_t channel = 0; channel < channelCount; channel += 1) {
      samples[offset + channel] *= gain;
    }
  }
}

bool NoiseGate::isOpen() const {
  return open_;
}

float NoiseGate::currentAttenuationDb() const {
  return attenuationDb_;
}

float NoiseGate::targetAttenuation(float levelDb) {
  if (open_) return 0.0f;
  if (config_.mode == NoiseMode::gate) return config_.rangeDb;
  const auto belowThreshold = std::max(0.0f, config_.thresholdDb - levelDb);
  const auto attenuation = -belowThreshold * (config_.ratio - 1.0f);
  return std::max(config_.rangeDb, attenuation);
}

float NoiseGate::smoothedAttenuation(float targetDb) {
  const auto coeff = targetDb > attenuationDb_ ? coefficient(config_.sampleRate, config_.attackMs)
                                               : coefficient(config_.sampleRate, config_.releaseMs);
  attenuationDb_ = coeff * attenuationDb_ + (1.0f - coeff) * targetDb;
  return attenuationDb_;
}

void NoiseGate::updateOpenState(float levelDb) {
  if (levelDb >= config_.thresholdDb + config_.hysteresisDb) {
    open_ = true;
    holdFramesRemaining_ = static_cast<std::size_t>(config_.sampleRate * config_.holdMs * 0.001);
    return;
  }

  if (!open_) return;
  if (levelDb > config_.thresholdDb - config_.hysteresisDb) {
    holdFramesRemaining_ = static_cast<std::size_t>(config_.sampleRate * config_.holdMs * 0.001);
    return;
  }

  if (holdFramesRemaining_ > 0) {
    holdFramesRemaining_ -= 1;
    return;
  }
  open_ = false;
}

}  // namespace localmixer::dsp
