#include "dsp/Meter.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp {
namespace {

constexpr float kMinDb = -120.0f;

}  // namespace

PeakRmsMeter::PeakRmsMeter(std::uint32_t rmsWindowFrames) : squares_(std::max<std::uint32_t>(1, rmsWindowFrames), 0.0f) {}

MeterReading PeakRmsMeter::process(std::span<const float> samples) {
  float peak = 0.0f;
  for (const auto sample : samples) {
    const auto magnitude = std::fabs(sample);
    peak = std::max(peak, magnitude);
    clipped_ = clipped_ || magnitude >= 1.0f;

    const auto square = sample * sample;
    sumSquares_ -= squares_[cursor_];
    squares_[cursor_] = square;
    sumSquares_ += square;
    cursor_ = (cursor_ + 1) % static_cast<std::uint32_t>(squares_.size());
    filled_ = std::min<std::uint32_t>(filled_ + 1, static_cast<std::uint32_t>(squares_.size()));
  }

  const auto rms = filled_ == 0 ? 0.0f : std::sqrt(static_cast<float>(sumSquares_ / filled_));
  return MeterReading{
    .peakDb = amplitudeToDb(peak),
    .rmsDb = amplitudeToDb(rms),
    .clipped = clipped_,
  };
}

void PeakRmsMeter::resetClip() {
  clipped_ = false;
}

bool PeakRmsMeter::clipLatched() const {
  return clipped_;
}

float amplitudeToDb(float amplitude) {
  if (amplitude <= 0.0f) return kMinDb;
  return std::max(kMinDb, 20.0f * std::log10(amplitude));
}

}  // namespace localmixer::dsp
