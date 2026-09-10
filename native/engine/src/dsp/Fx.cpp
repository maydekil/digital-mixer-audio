#include "dsp/Fx.hpp"

#include "dsp/Dynamics.hpp"
#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp {
namespace {

float smoothingCoefficient(double sampleRate, float timeMs) {
  if (sampleRate <= 0.0 || timeMs <= 0.0f) return 0.0f;
  return std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
}

std::size_t msToSamples(double sampleRate, float ms) {
  return std::max<std::size_t>(1, static_cast<std::size_t>(sampleRate * std::max(0.0f, ms) * 0.001f));
}

}  // namespace

void DelayLine::configure(const DelayConfig& config) {
  config_ = config;
  config_.feedback = std::clamp(config_.feedback, 0.0f, 0.95f);
  config_.tone = std::clamp(config_.tone, 0.0f, 1.0f);
  buffer_.assign(msToSamples(config_.sampleRate, config_.delayMs), 0.0f);
  writeIndex_ = 0;
  toneState_ = 0.0f;
}

void DelayLine::reset() {
  std::fill(buffer_.begin(), buffer_.end(), 0.0f);
  writeIndex_ = 0;
  toneState_ = 0.0f;
}

void DelayLine::processWet(std::span<const float> input, std::span<float> output) {
  const auto count = std::min(input.size(), output.size());
  if (!config_.enabled || buffer_.empty()) {
    std::fill(output.begin(), output.begin() + static_cast<std::ptrdiff_t>(count), 0.0f);
    return;
  }

  for (std::size_t index = 0; index < count; index += 1) {
    const auto delayed = buffer_[writeIndex_];
    toneState_ += (delayed - toneState_) * config_.tone;
    output[index] = toneState_ * config_.wetGain;
    buffer_[writeIndex_] = input[index] + toneState_ * config_.feedback;
    writeIndex_ = (writeIndex_ + 1) % buffer_.size();
  }
}

void SimpleReverb::configure(const ReverbConfig& config) {
  config_ = config;
  config_.decay = std::clamp(config_.decay, 0.0f, 0.95f);
  delayA_.assign(msToSamples(config_.sampleRate, config_.preDelayMs + 31.0f), 0.0f);
  delayB_.assign(msToSamples(config_.sampleRate, config_.preDelayMs + 47.0f), 0.0f);
  indexA_ = 0;
  indexB_ = 0;
}

void SimpleReverb::reset() {
  std::fill(delayA_.begin(), delayA_.end(), 0.0f);
  std::fill(delayB_.begin(), delayB_.end(), 0.0f);
  indexA_ = 0;
  indexB_ = 0;
}

void SimpleReverb::processWet(std::span<const float> input, std::span<float> output) {
  const auto count = std::min(input.size(), output.size());
  if (!config_.enabled || delayA_.empty() || delayB_.empty()) {
    std::fill(output.begin(), output.begin() + static_cast<std::ptrdiff_t>(count), 0.0f);
    return;
  }

  for (std::size_t index = 0; index < count; index += 1) {
    const auto a = delayA_[indexA_];
    const auto b = delayB_[indexB_];
    const auto wet = (a + b) * 0.5f;
    output[index] = wet * config_.wetGain;
    delayA_[indexA_] = input[index] + b * config_.decay;
    delayB_[indexB_] = input[index] - a * config_.decay;
    indexA_ = (indexA_ + 1) % delayA_.size();
    indexB_ = (indexB_ + 1) % delayB_.size();
  }
}

void VoiceDucker::configure(const DuckerConfig& config) {
  config_ = config;
  config_.depthDb = std::min(0.0f, config_.depthDb);
  holdFrames_ = 0;
  gainDb_ = 0.0f;
}

void VoiceDucker::reset() {
  holdFrames_ = 0;
  gainDb_ = 0.0f;
}

void VoiceDucker::process(std::span<const float> detector, std::span<float> target) {
  if (!config_.enabled) return;
  const auto count = std::min(detector.size(), target.size());
  for (std::size_t index = 0; index < count; index += 1) {
    const auto active = linearToDecibels(detector[index]) >= config_.thresholdDb;
    if (active) holdFrames_ = msToSamples(config_.sampleRate, config_.holdMs);
    const auto targetGainDb = (active || holdFrames_ > 0) ? config_.depthDb : 0.0f;
    if (!active && holdFrames_ > 0) holdFrames_ -= 1;
    const auto coeff = targetGainDb < gainDb_ ? smoothingCoefficient(config_.sampleRate, config_.attackMs)
                                              : smoothingCoefficient(config_.sampleRate, config_.releaseMs);
    gainDb_ = coeff * gainDb_ + (1.0f - coeff) * targetGainDb;
    target[index] *= decibelsToLinear(gainDb_);
  }
}

float VoiceDucker::currentGainDb() const {
  return gainDb_;
}

}  // namespace localmixer::dsp
