#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace localmixer::dsp {

struct MeterReading {
  float peakDb = -120.0f;
  float rmsDb = -120.0f;
  bool clipped = false;
};

class PeakRmsMeter {
 public:
  explicit PeakRmsMeter(std::uint32_t rmsWindowFrames = 14400);

  MeterReading process(std::span<const float> samples);
  void resetClip();
  bool clipLatched() const;

 private:
  std::vector<float> squares_;
  std::uint32_t cursor_ = 0;
  std::uint32_t filled_ = 0;
  double sumSquares_ = 0.0;
  bool clipped_ = false;
};

float amplitudeToDb(float amplitude);

}  // namespace localmixer::dsp
