#pragma once

#include <cstdint>
#include <vector>

namespace localmixer::engine {

struct MetronomeConfig {
  bool enabled = false;
  double sampleRate = 48000.0;
  double bpm = 120.0;
  std::uint32_t beatsPerBar = 4;
  bool printToMaster = false;
};

struct MetronomeClick {
  std::uint64_t frame = 0;
  bool accented = false;
};

std::vector<MetronomeClick> metronomeClicks(
  const MetronomeConfig& config, std::uint64_t startFrame, std::uint64_t frameCount);
bool metronomeContributesToExport(const MetronomeConfig& config);

}  // namespace localmixer::engine
