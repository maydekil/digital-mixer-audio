#pragma once

#include "dsp/DeEsser.hpp"
#include "dsp/Dynamics.hpp"
#include "dsp/Eq.hpp"

#include <array>

namespace localmixer::engine {

struct ChannelProcessorConfig {
  bool eqEnabled = false;
  bool noiseEnabled = false;
  bool compressorEnabled = false;
  bool deEsserEnabled = false;
  double sampleRate = 48000.0;
  std::array<dsp::EqBandConfig, 4> eqBands{};
  dsp::NoiseGateConfig noise{};
  dsp::CompressorConfig compressor{};
  dsp::DeEsserConfig deEsser{};
};

ChannelProcessorConfig defaultChannelProcessorConfig(double sampleRate = 48000.0);

class ChannelProcessorChain {
 public:
  ChannelProcessorChain();

  void configure(const ChannelProcessorConfig& config);
  void reset();
  void processFrame(float& left, float& right);
  const ChannelProcessorConfig& config() const;
  float compressorReductionDb() const;
  float noiseAttenuationDb() const;
  float deEsserReductionDb() const;

 private:
  ChannelProcessorConfig config_;
  std::array<dsp::BiquadFilter, 4> eqLeft_;
  std::array<dsp::BiquadFilter, 4> eqRight_;
  dsp::NoiseGate noise_;
  dsp::Compressor compressor_;
  dsp::DeEsser deEsser_;
};

}  // namespace localmixer::engine
