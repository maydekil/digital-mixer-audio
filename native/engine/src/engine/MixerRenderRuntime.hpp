#pragma once

#include "engine/FxProgramWetProcessor.hpp"
#include "engine/MixerGraph.hpp"

namespace localmixer::engine {

class MixerRenderRuntime {
 public:
  explicit MixerRenderRuntime(double sampleRate = 48000.0);

  MixerGraph& graph() noexcept;
  const MixerGraph& graph() const noexcept;
  void prepare(std::uint32_t maximumFrames);
  bool setFxProgram(FxBusId bus, std::uint32_t programId);
  MixerError process(std::span<const SourceBuffer> sources, StereoOutput output);
  std::uint32_t fxProgramId(FxBusId bus) const noexcept;

 private:
  MixerGraph graph_;
  FxProgramWetProcessor fxA_;
  FxProgramWetProcessor fxB_;
};

}  // namespace localmixer::engine
