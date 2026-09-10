#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace localmixer::dsp::fx {

struct PitchDetectorConfig {
  double sampleRate = 48000.0;
  std::uint32_t windowFrames = 4096;
  float minFrequencyHz = 70.0f;
  float maxFrequencyHz = 1000.0f;
  float threshold = 0.15f;
};

struct PitchEstimate {
  bool voiced = false;
  float frequencyHz = 0.0f;
  float confidence = 0.0f;
  std::uint64_t centerFrame = 0;
};

class PitchDetector {
 public:
  void prepare(const PitchDetectorConfig& config);
  PitchEstimate analyze(std::span<const float> monoWindow, std::uint64_t startFrame) noexcept;

  std::uint32_t windowFrames() const noexcept { return config_.windowFrames; }

 private:
  PitchDetectorConfig config_;
  std::vector<float> difference_;
  std::vector<float> cmndf_;
};

enum class ScaleType {
  chromatic,
  major,
  naturalMinor,
};

struct ScaleMapperConfig {
  int keySemitone = 0;
  ScaleType scale = ScaleType::major;
  float a4Hz = 440.0f;
  float toleranceCents = 10.0f;
  float amount = 1.0f;
};

struct PitchTarget {
  bool active = false;
  int inputMidi = 0;
  int targetMidi = 0;
  float detuneCents = 0.0f;
  float correctionSemitones = 0.0f;
};

PitchTarget mapPitchToScale(float frequencyHz, const ScaleMapperConfig& config) noexcept;
float midiToFrequency(int midi, float a4Hz) noexcept;
float frequencyToMidi(float frequencyHz, float a4Hz) noexcept;

}  // namespace localmixer::dsp::fx
