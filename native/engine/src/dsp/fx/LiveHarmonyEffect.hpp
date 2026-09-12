#pragma once

#include "dsp/fx/EffectProcessor.hpp"
#include "dsp/fx/PitchBackend.hpp"

#include <array>
#include <memory>
#include <vector>

namespace localmixer::dsp::fx {

struct LiveHarmonyVoiceConfig {
  bool enabled = true;
  float semitones = 4.0f;
  float levelDb = -6.0f;
  float pan = -0.35f;
};

struct LiveHarmonyConfig {
  float levelDb = 0.0f;
  bool preserveFormants = true;
  LiveHarmonyVoiceConfig voice1{};
  LiveHarmonyVoiceConfig voice2{.enabled = true, .semitones = 7.0f, .levelDb = -8.0f, .pan = 0.35f};
};

class LiveHarmonyEffect final : public EffectProcessor {
 public:
  explicit LiveHarmonyEffect(LiveHarmonyConfig config = {});

  void prepare(const ProcessSpec& spec) override;
  void reset() noexcept override;
  void process(AudioBlockView& block, const ProcessContext& context) noexcept override;
  void applyRealtimeParameter(ParameterId parameter, float value) noexcept override;
  std::uint32_t latencySamples() const noexcept override;
  std::uint64_t maximumTailSamples() const noexcept override;

 private:
  struct VoiceState {
    std::unique_ptr<PitchBackend> backend;
    std::vector<float> outputRing;
    std::size_t read = 0;
    std::size_t write = 0;
    std::size_t fill = 0;
  };

  void configureVoices() noexcept;
  void processReadyBlock() noexcept;
  void pushVoiceOutput(VoiceState& voice, std::span<const float> samples) noexcept;
  float popVoiceOutput(VoiceState& voice) noexcept;

  LiveHarmonyConfig config_;
  ProcessSpec spec_;
  std::array<VoiceState, 2> voices_;
  std::vector<float> inputBlock_;
  std::array<std::vector<float>, 2> shiftedBlocks_;
  std::size_t blockSize_ = 0;
  std::size_t inputFill_ = 0;
};

}  // namespace localmixer::dsp::fx
