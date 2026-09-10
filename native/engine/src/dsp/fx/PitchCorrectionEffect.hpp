#pragma once

#include "dsp/fx/EffectProcessor.hpp"
#include "dsp/fx/PitchBackend.hpp"
#include "dsp/fx/PitchDetector.hpp"

#include <memory>
#include <vector>

namespace localmixer::dsp::fx {

struct PitchCorrectionConfig {
  int keySemitone = 0;
  ScaleType scale = ScaleType::major;
  float a4Hz = 440.0f;
  float retuneMs = 80.0f;
  float amount = 0.7f;
  float toleranceCents = 10.0f;
  float confidenceThreshold = 0.85f;
};

class PitchCorrectionEffect final : public EffectProcessor {
 public:
  explicit PitchCorrectionEffect(PitchCorrectionConfig config = {});

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

  PitchEstimate lastEstimate() const noexcept { return lastEstimate_; }
  PitchTarget lastTarget() const noexcept { return lastTarget_; }

 private:
  void pushAnalysisSample(float sample, std::uint64_t frame) noexcept;
  void analyzeIfReady(std::uint64_t frame) noexcept;
  float nextSmoothedSemitones() noexcept;
  bool processBackendBlock(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept;

  PitchCorrectionConfig config_;
  ProcessSpec spec_;
  PitchDetector detector_;
  std::unique_ptr<PitchBackend> backend_;
  std::vector<float> analysisRing_;
  std::vector<float> analysisWindow_;
  std::vector<float> backendInput_;
  std::vector<float> backendOutput_;
  std::size_t analysisWrite_ = 0;
  std::uint32_t samplesSinceAnalysis_ = 0;
  std::uint32_t filledAnalysis_ = 0;
  std::uint32_t hopFrames_ = 256;
  float currentSemitones_ = 0.0f;
  float targetSemitones_ = 0.0f;
  PitchEstimate lastEstimate_;
  PitchTarget lastTarget_;
};

}  // namespace localmixer::dsp::fx
