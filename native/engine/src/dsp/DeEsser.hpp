#pragma once

#include "dsp/Eq.hpp"

#include <array>
#include <cstddef>
#include <span>

namespace localmixer::dsp {

struct DeEsserConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  float detectorFrequencyHz = 6000.0f;
  float thresholdDb = -24.0f;
  float maxReductionDb = 6.0f;
  float attackMs = 2.0f;
  float releaseMs = 80.0f;
};

class DeEsser {
 public:
  void configure(const DeEsserConfig& config);
  void reset();
  void processMono(std::span<float> samples);
  void processInterleavedLinked(std::span<float> samples, std::size_t channelCount);
  void renderDetectorAuditionMono(std::span<const float> input, std::span<float> output);
  float lastReductionDb() const;

 private:
  float detectSample(float sample, std::size_t channel);
  float smoothedReduction(float detectorDb);

  struct BiquadState {
    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;
  };

  float runDetector(float sample, BiquadState& state) const;
  void resetDetectorStates();

  DeEsserConfig config_;
  BiquadCoefficients detectorCoefficients_;
  std::array<BiquadState, 16> detectorStates_{};
  float reductionDb_ = 0.0f;
};

}  // namespace localmixer::dsp
