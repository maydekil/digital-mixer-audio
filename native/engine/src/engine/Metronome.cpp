#include "engine/Metronome.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {

std::vector<MetronomeClick> metronomeClicks(
  const MetronomeConfig& config, std::uint64_t startFrame, std::uint64_t frameCount) {
  if (!config.enabled || config.sampleRate <= 0.0 || config.bpm <= 0.0 || frameCount == 0) return {};
  const auto beatFrames = static_cast<std::uint64_t>(std::llround(config.sampleRate * 60.0 / config.bpm));
  if (beatFrames == 0) return {};

  const auto endFrame = startFrame + frameCount;
  const auto firstBeat = (startFrame + beatFrames - 1) / beatFrames;
  const auto beatsPerBar = std::max<std::uint64_t>(1, config.beatsPerBar);
  std::vector<MetronomeClick> clicks;

  for (auto beat = firstBeat;; beat += 1) {
    const auto frame = beat * beatFrames;
    if (frame >= endFrame) break;
    clicks.push_back(MetronomeClick{.frame = frame, .accented = beat % beatsPerBar == 0});
  }
  return clicks;
}

bool metronomeContributesToExport(const MetronomeConfig& config) {
  return config.enabled && config.printToMaster;
}

}  // namespace localmixer::engine
