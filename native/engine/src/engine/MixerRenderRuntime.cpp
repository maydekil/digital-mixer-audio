#include "engine/MixerRenderRuntime.hpp"

namespace localmixer::engine {

MixerRenderRuntime::MixerRenderRuntime(double sampleRate) : fxA_(sampleRate), fxB_(sampleRate) {
  fxA_.configure(12);
  fxB_.configure(50);
}

MixerGraph& MixerRenderRuntime::graph() noexcept {
  return graph_;
}

const MixerGraph& MixerRenderRuntime::graph() const noexcept {
  return graph_;
}

void MixerRenderRuntime::prepare(std::uint32_t maximumFrames) {
  graph_.prepare(maximumFrames);
  fxA_.prepare(maximumFrames);
  fxB_.prepare(maximumFrames);
}

bool MixerRenderRuntime::setFxProgram(FxBusId bus, std::uint32_t programId) {
  return bus == FxBusId::a ? fxA_.configure(programId) : fxB_.configure(programId);
}

MixerError MixerRenderRuntime::process(std::span<const SourceBuffer> sources, StereoOutput output) {
  return graph_.processWithFx(sources, output, fxA_.callback(), fxB_.callback());
}

std::uint32_t MixerRenderRuntime::fxProgramId(FxBusId bus) const noexcept {
  return bus == FxBusId::a ? fxA_.programId() : fxB_.programId();
}

}  // namespace localmixer::engine
