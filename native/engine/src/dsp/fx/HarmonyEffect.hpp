#pragma once

#include "dsp/fx/EffectProcessor.hpp"
#include "dsp/fx/PitchBackend.hpp"
#include "dsp/fx/PitchDetector.hpp"

#include <array>
#include <memory>
#include <vector>

namespace localmixer::dsp::fx {

enum class HarmonyMode {
  fixed,
  diatonic,
};

struct HarmonyVoiceConfig {
  bool enabled = true;
  int interval = 2;
  float levelDb = -12.0f;
  float pan = -0.4f;
};

struct HarmonyConfig {
  HarmonyMode mode = HarmonyMode::diatonic;
  int keySemitone = 0;
  ScaleType scale = ScaleType::major;
  float a4Hz = 440.0f;
  float confidenceThreshold = 0.85f;
  bool preserveFormants = true;
  HarmonyVoiceConfig voice1{};
  HarmonyVoiceConfig voice2{.enabled = true, .interval = 4, .levelDb = -15.0f, .pan = 0.4f};
};

struct HarmonyTarget {
  bool active = false;
  int sourceMidi = 0;
  std::array<int, 2> targetMidi{};
  std::array<float, 2> shiftSemitones{};
};

class HarmonyEffect final : public EffectProcessor {
 public:
  explicit HarmonyEffect(HarmonyConfig config = {});

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

  HarmonyTarget lastTarget() const noexcept { return lastTarget_; }

 private:
  void pushAnalysisSample(float sample) noexcept;
  void analyzeIfReady(std::uint64_t frame) noexcept;
  bool processBackendBlock(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept;

  HarmonyConfig config_;
  ProcessSpec spec_;
  PitchDetector detector_;
  std::array<std::unique_ptr<PitchBackend>, 2> voices_;
  std::vector<float> analysisRing_;
  std::vector<float> analysisWindow_;
  std::vector<float> voiceInput_;
  std::array<std::vector<float>, 2> voiceOutput_;
  std::size_t analysisWrite_ = 0;
  std::uint32_t samplesSinceAnalysis_ = 0;
  std::uint32_t filledAnalysis_ = 0;
  std::uint32_t hopFrames_ = 256;
  float voiceGate_ = 0.0f;
  HarmonyTarget lastTarget_;
};

int mapDiatonicInterval(int sourceMidi, int keySemitone, ScaleType scale, int intervalSteps) noexcept;
float panLeft(float pan) noexcept;
float panRight(float pan) noexcept;

}  // namespace localmixer::dsp::fx
