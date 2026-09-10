#pragma once

#include <cstddef>
#include <span>

namespace localmixer::dsp {

float linearToDecibels(float value);

struct CompressorConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  float thresholdDb = -24.0f;
  float ratio = 4.0f;
  float kneeDb = 0.0f;
  float attackMs = 10.0f;
  float releaseMs = 120.0f;
  float makeupGainDb = 0.0f;
};

class Compressor {
 public:
  void configure(const CompressorConfig& config);
  void reset();
  void processMono(std::span<float> samples);
  void processInterleavedLinked(std::span<float> samples, std::size_t channelCount);
  float lastGainReductionDb() const;

 private:
  float gainForLevel(float levelDb) const;
  float smoothedGain(float targetGainDb);

  CompressorConfig config_;
  float smoothedGainDb_ = 0.0f;
  float lastGainReductionDb_ = 0.0f;
};

enum class NoiseMode {
  gate,
  expander,
};

struct NoiseGateConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  NoiseMode mode = NoiseMode::gate;
  float thresholdDb = -50.0f;
  float hysteresisDb = 3.0f;
  float holdMs = 20.0f;
  float rangeDb = -80.0f;
  float ratio = 2.0f;
  float attackMs = 5.0f;
  float releaseMs = 80.0f;
};

class NoiseGate {
 public:
  void configure(const NoiseGateConfig& config);
  void reset();
  void processMono(std::span<float> samples);
  void processInterleavedLinked(std::span<float> samples, std::size_t channelCount);
  bool isOpen() const;
  float currentAttenuationDb() const;

 private:
  float targetAttenuation(float levelDb);
  float smoothedAttenuation(float targetDb);
  void updateOpenState(float levelDb);

  NoiseGateConfig config_;
  bool open_ = false;
  std::size_t holdFramesRemaining_ = 0;
  float attenuationDb_ = -80.0f;
};

}  // namespace localmixer::dsp
