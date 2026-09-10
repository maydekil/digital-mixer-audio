#pragma once

#include "dsp/fx/EffectProcessor.hpp"

#include <array>
#include <cstdint>

namespace localmixer::dsp::fx {

enum class VocoderCarrierMode {
  fixed,
  midi,
};

struct VocoderConfig {
  VocoderCarrierMode carrierMode = VocoderCarrierMode::fixed;
  int fixedMidiNote = 48;
  float attackMs = 5.0f;
  float releaseMs = 100.0f;
  float unvoicedPct = 20.0f;
  float outputDb = -6.0f;
};

class VocoderEffect final : public EffectProcessor {
 public:
  struct BiquadState {
    double b0 = 1.0;
    double b1 = 0.0;
    double b2 = 0.0;
    double a1 = 0.0;
    double a2 = 0.0;
    double x1 = 0.0;
    double x2 = 0.0;
    double y1 = 0.0;
    double y2 = 0.0;

    float process(float sample) noexcept;
    void reset() noexcept;
  };

  explicit VocoderEffect(VocoderConfig config = {});

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

  void noteOn(int midiNote) noexcept;
  void noteOff(int midiNote) noexcept;
  void allNotesOff() noexcept;
  void panic() noexcept;

 private:
  struct Band {
    BiquadState analysis;
    BiquadState synthesis;
    float envelope = 0.0f;
  };

  float nextCarrier() noexcept;
  float nextNoise() noexcept;
  void updateBands();
  void clearOscillators() noexcept;

  VocoderConfig config_;
  ProcessSpec spec_;
  std::array<Band, 16> bands_;
  std::array<int, 4> activeNotes_{};
  std::array<float, 4> phases_{};
  std::uint32_t noteCount_ = 0;
  std::uint32_t noiseState_ = 0x12345678U;
};

}  // namespace localmixer::dsp::fx
