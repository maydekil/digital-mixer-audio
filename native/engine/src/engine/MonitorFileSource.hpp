#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace localmixer::engine {

struct MonitorFileBuffer {
  std::vector<float> samples;
  std::uint16_t channels = 0;
  std::uint64_t frameCount = 0;
  std::string error;
};

// Control-thread preparation only: file I/O, allocation and rate conversion
// must finish before any audio callback starts.
MonitorFileBuffer prepareMonitorFile(const std::filesystem::path& path, double outputSampleRate);

}  // namespace localmixer::engine
