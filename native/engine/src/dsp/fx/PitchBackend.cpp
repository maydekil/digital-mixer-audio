#include "dsp/fx/PitchBackend.hpp"

#include <cmath>
#include <memory>
#include <rubberband/RubberBandLiveShifter.h>

namespace localmixer::dsp::fx {
namespace {

class RubberBandPitchBackend final : public PitchBackend {
 public:
  bool prepare(const PitchBackendSpec& spec) override {
    spec_ = spec;
    auto options = RubberBand::RubberBandLiveShifter::OptionWindowShort |
      RubberBand::RubberBandLiveShifter::OptionChannelsTogether;
    if (spec.preserveFormants) options |= RubberBand::RubberBandLiveShifter::OptionFormantPreserved;
    shifter_ = std::make_unique<RubberBand::RubberBandLiveShifter>(
      static_cast<std::size_t>(spec.sampleRate),
      static_cast<std::size_t>(spec.channels),
      options
    );
    const auto frames = shifter_->getBlockSize();
    inputs_.assign(spec.channels, std::vector<float>(frames, 0.0f));
    outputs_.assign(spec.channels, std::vector<float>(frames, 0.0f));
    inputPtrs_.resize(spec.channels);
    outputPtrs_.resize(spec.channels);
    for (std::size_t channel = 0; channel < spec.channels; channel += 1) {
      inputPtrs_[channel] = inputs_[channel].data();
      outputPtrs_[channel] = outputs_[channel].data();
    }
    return frames > 0;
  }

  void reset() noexcept override {
    if (shifter_) shifter_->reset();
  }

  void setPitchSemitones(float semitones) noexcept override {
    if (shifter_) shifter_->setPitchScale(semitonesToRatio(semitones));
  }

  void setFormantSemitones(float semitones) noexcept override {
    if (shifter_) shifter_->setFormantScale(semitonesToRatio(semitones));
  }

  std::uint32_t blockSize() const noexcept override {
    return shifter_ ? static_cast<std::uint32_t>(shifter_->getBlockSize()) : 0;
  }

  std::uint32_t startDelaySamples() const noexcept override {
    return shifter_ ? static_cast<std::uint32_t>(shifter_->getStartDelay()) : 0;
  }

  bool processBlock(std::span<const float> input, std::span<float> output) noexcept override {
    if (!shifter_ || spec_.channels == 0 || input.size() != output.size()) return false;
    const auto frames = blockSize();
    if (input.size() != static_cast<std::size_t>(frames * spec_.channels)) return false;
    for (std::uint32_t frame = 0; frame < frames; frame += 1) {
      for (std::uint32_t channel = 0; channel < spec_.channels; channel += 1) {
        inputs_[channel][frame] = input[frame * spec_.channels + channel];
      }
    }
    shifter_->shift(inputPtrs_.data(), outputPtrs_.data());
    for (std::uint32_t frame = 0; frame < frames; frame += 1) {
      for (std::uint32_t channel = 0; channel < spec_.channels; channel += 1) {
        output[frame * spec_.channels + channel] = outputs_[channel][frame];
      }
    }
    return true;
  }

 private:
  PitchBackendSpec spec_;
  std::unique_ptr<RubberBand::RubberBandLiveShifter> shifter_;
  std::vector<std::vector<float>> inputs_;
  std::vector<std::vector<float>> outputs_;
  std::vector<float*> inputPtrs_;
  std::vector<float*> outputPtrs_;
};

}  // namespace

std::unique_ptr<PitchBackend> makeRubberBandPitchBackend() {
  return std::make_unique<RubberBandPitchBackend>();
}

double semitonesToRatio(float semitones) {
  return std::pow(2.0, static_cast<double>(semitones) / 12.0);
}

}  // namespace localmixer::dsp::fx
