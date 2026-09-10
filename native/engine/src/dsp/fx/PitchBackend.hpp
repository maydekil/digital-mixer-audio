#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace localmixer::dsp::fx {

struct PitchBackendSpec {
  double sampleRate = 48000.0;
  std::uint32_t channels = 1;
  bool preserveFormants = true;
};

class PitchBackend {
 public:
  virtual ~PitchBackend() = default;
  virtual bool prepare(const PitchBackendSpec& spec) = 0;
  virtual void reset() noexcept = 0;
  virtual void setPitchSemitones(float semitones) noexcept = 0;
  virtual void setFormantSemitones(float semitones) noexcept = 0;
  virtual std::uint32_t blockSize() const noexcept = 0;
  virtual std::uint32_t startDelaySamples() const noexcept = 0;
  virtual bool processBlock(std::span<const float> input, std::span<float> output) noexcept = 0;
};

std::unique_ptr<PitchBackend> makeRubberBandPitchBackend();
double semitonesToRatio(float semitones);

}  // namespace localmixer::dsp::fx
