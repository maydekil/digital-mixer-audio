#pragma once

#include "dsp/OutputProtection.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace localmixer::engine {

struct DeviceDescriptor {
  std::string uid;
  std::string name;
  bool isDefaultInput = false;
  bool isDefaultOutput = false;
  std::uint32_t inputChannels = 0;
  std::uint32_t outputChannels = 0;
  double sampleRate = 0.0;
  std::uint32_t maxBlockSize = 512;
};

struct ChannelLabels {
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
};

struct ProjectAudioConfig {
  double sampleRate = 48000.0;
  std::uint32_t blockSize = 256;
};

struct PassthroughSelection {
  std::string inputUid;
  std::string outputUid;
  std::uint32_t inputChannel = 0;
  std::uint32_t outputChannel = 0;
  bool monitoringEnabled = false;
};

enum class DevicePrepareError {
  none,
  inputNotFound,
  outputNotFound,
  inputChannelOutOfRange,
  outputChannelOutOfRange,
  sampleRateMismatch,
  blockSizeTooLarge,
};

struct PreparedPassthrough {
  DeviceDescriptor inputDevice;
  DeviceDescriptor outputDevice;
  PassthroughSelection selection;
  ProjectAudioConfig project;
  std::vector<float> scratch;
};

struct PrepareResult {
  DevicePrepareError error = DevicePrepareError::none;
  std::optional<PreparedPassthrough> passthrough;
};

class DeviceService {
 public:
  explicit DeviceService(std::vector<DeviceDescriptor> devices);

  const std::vector<DeviceDescriptor>& devices() const;
  std::optional<DeviceDescriptor> findByUid(const std::string& uid) const;
  ChannelLabels labelsFor(const DeviceDescriptor& device) const;
  PrepareResult preparePassthrough(
    const ProjectAudioConfig& project,
    const PassthroughSelection& selection
  ) const;

 private:
  std::vector<DeviceDescriptor> devices_;
};

void renderPassthrough(
  const PreparedPassthrough& prepared,
  std::span<const float> inputInterleaved,
  std::uint32_t frameCount,
  std::uint32_t inputChannelCount,
  std::span<float> outputInterleaved,
  std::uint32_t outputChannelCount,
  const dsp::OutputProtection& protection
);

}  // namespace localmixer::engine
