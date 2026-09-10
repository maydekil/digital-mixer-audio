#include "engine/ChannelProcessorChain.hpp"

#include <array>

namespace localmixer::engine {

ChannelProcessorConfig defaultChannelProcessorConfig(double sampleRate) {
  auto config = ChannelProcessorConfig{};
  config.sampleRate = sampleRate;
  config.eqBands = {
    dsp::EqBandConfig{.type = dsp::EqFilterType::lowShelf, .enabled = true, .sampleRate = sampleRate, .frequencyHz = 100.0, .gainDb = 3.0},
    dsp::EqBandConfig{.type = dsp::EqFilterType::peaking, .enabled = true, .sampleRate = sampleRate, .frequencyHz = 350.0, .gainDb = -2.5, .q = 1.2},
    dsp::EqBandConfig{.type = dsp::EqFilterType::peaking, .enabled = true, .sampleRate = sampleRate, .frequencyHz = 2500.0, .gainDb = 2.0, .q = 1.0},
    dsp::EqBandConfig{.type = dsp::EqFilterType::highShelf, .enabled = true, .sampleRate = sampleRate, .frequencyHz = 10000.0, .gainDb = 4.0},
  };
  config.noise = dsp::NoiseGateConfig{.enabled = true, .sampleRate = sampleRate};
  config.compressor = dsp::CompressorConfig{
    .enabled = true,
    .sampleRate = sampleRate,
    .thresholdDb = -18.0f,
    .ratio = 3.0f,
    .attackMs = 10.0f,
    .releaseMs = 120.0f,
  };
  config.deEsser = dsp::DeEsserConfig{.enabled = true, .sampleRate = sampleRate};
  return config;
}

ChannelProcessorChain::ChannelProcessorChain() {
  configure(defaultChannelProcessorConfig());
}

void ChannelProcessorChain::configure(const ChannelProcessorConfig& config) {
  config_ = config;
  for (std::size_t index = 0; index < config_.eqBands.size(); index += 1) {
    auto band = config_.eqBands[index];
    band.enabled = config_.eqEnabled && band.enabled;
    band.sampleRate = config_.sampleRate;
    const auto coefficients = dsp::makeBiquad(band);
    eqLeft_[index].setCoefficients(coefficients);
    eqRight_[index].setCoefficients(coefficients);
  }

  auto noise = config_.noise;
  noise.enabled = config_.noiseEnabled && noise.enabled;
  noise.sampleRate = config_.sampleRate;
  noise_.configure(noise);

  auto compressor = config_.compressor;
  compressor.enabled = config_.compressorEnabled && compressor.enabled;
  compressor.sampleRate = config_.sampleRate;
  compressor_.configure(compressor);

  auto deEsser = config_.deEsser;
  deEsser.enabled = config_.deEsserEnabled && deEsser.enabled;
  deEsser.sampleRate = config_.sampleRate;
  deEsser_.configure(deEsser);
}

void ChannelProcessorChain::reset() {
  for (auto& filter : eqLeft_) filter.reset();
  for (auto& filter : eqRight_) filter.reset();
  noise_.reset();
  compressor_.reset();
  deEsser_.reset();
}

void ChannelProcessorChain::processFrame(float& left, float& right) {
  std::array<float, 1> leftSample{left};
  std::array<float, 1> rightSample{right};
  std::array<float, 2> linked{leftSample[0], rightSample[0]};
  if (config_.noiseEnabled) noise_.processInterleavedLinked(linked, 2);
  leftSample[0] = linked[0];
  rightSample[0] = linked[1];
  if (config_.eqEnabled) {
    for (auto& filter : eqLeft_) filter.process(leftSample);
    for (auto& filter : eqRight_) filter.process(rightSample);
  }

  linked = {leftSample[0], rightSample[0]};
  if (config_.compressorEnabled) compressor_.processInterleavedLinked(linked, 2);
  if (config_.deEsserEnabled) deEsser_.processInterleavedLinked(linked, 2);
  left = linked[0];
  right = linked[1];
}

const ChannelProcessorConfig& ChannelProcessorChain::config() const {
  return config_;
}

float ChannelProcessorChain::compressorReductionDb() const {
  return compressor_.lastGainReductionDb();
}

float ChannelProcessorChain::noiseAttenuationDb() const {
  return noise_.currentAttenuationDb();
}

float ChannelProcessorChain::deEsserReductionDb() const {
  return deEsser_.lastReductionDb();
}

}  // namespace localmixer::engine
