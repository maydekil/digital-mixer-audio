#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace localmixer::dsp {

struct DelayConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  float delayMs = 250.0f;
  float feedback = 0.35f;
  float wetGain = 1.0f;
  float tone = 0.5f;
};

class DelayLine {
 public:
  void configure(const DelayConfig& config);
  void reset();
  void processWet(std::span<const float> input, std::span<float> output);

 private:
  DelayConfig config_;
  std::vector<float> buffer_;
  std::size_t writeIndex_ = 0;
  float toneState_ = 0.0f;
};

struct ReverbConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  float preDelayMs = 20.0f;
  float decay = 0.45f;
  float wetGain = 0.35f;
};

class SimpleReverb {
 public:
  void configure(const ReverbConfig& config);
  void reset();
  void processWet(std::span<const float> input, std::span<float> output);

 private:
  ReverbConfig config_;
  std::vector<float> delayA_;
  std::vector<float> delayB_;
  std::size_t indexA_ = 0;
  std::size_t indexB_ = 0;
};

struct DuckerConfig {
  bool enabled = true;
  double sampleRate = 48000.0;
  float thresholdDb = -30.0f;
  float depthDb = -9.0f;
  float attackMs = 10.0f;
  float holdMs = 80.0f;
  float releaseMs = 180.0f;
};

class VoiceDucker {
 public:
  void configure(const DuckerConfig& config);
  void reset();
  void process(std::span<const float> detector, std::span<float> target);
  float currentGainDb() const;

 private:
  DuckerConfig config_;
  std::size_t holdFrames_ = 0;
  float gainDb_ = 0.0f;
};

}  // namespace localmixer::dsp
