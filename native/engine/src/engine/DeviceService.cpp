#include "engine/DeviceService.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {
namespace {

constexpr double kSampleRateTolerance = 0.01;

std::vector<std::string> numberedLabels(const char* prefix, std::uint32_t count) {
  std::vector<std::string> labels;
  labels.reserve(count);
  for (std::uint32_t index = 0; index < count; index += 1) {
    labels.push_back(std::string(prefix) + " " + std::to_string(index + 1));
  }
  return labels;
}

}  // namespace

DeviceService::DeviceService(std::vector<DeviceDescriptor> devices) : devices_(std::move(devices)) {}

const std::vector<DeviceDescriptor>& DeviceService::devices() const {
  return devices_;
}

std::optional<DeviceDescriptor> DeviceService::findByUid(const std::string& uid) const {
  const auto it = std::find_if(devices_.begin(), devices_.end(), [&](const auto& device) {
    return device.uid == uid;
  });
  if (it == devices_.end()) return std::nullopt;
  return *it;
}

ChannelLabels DeviceService::labelsFor(const DeviceDescriptor& device) const {
  return {
    .inputs = numberedLabels("In", device.inputChannels),
    .outputs = numberedLabels("Out", device.outputChannels),
  };
}

PrepareResult DeviceService::preparePassthrough(
  const ProjectAudioConfig& project,
  const PassthroughSelection& selection
) const {
  const auto input = findByUid(selection.inputUid);
  if (!input.has_value()) return {.error = DevicePrepareError::inputNotFound};

  const auto output = findByUid(selection.outputUid);
  if (!output.has_value()) return {.error = DevicePrepareError::outputNotFound};

  if (selection.inputChannel >= input->inputChannels) {
    return {.error = DevicePrepareError::inputChannelOutOfRange};
  }

  if (selection.outputChannel >= output->outputChannels) {
    return {.error = DevicePrepareError::outputChannelOutOfRange};
  }

  if (std::fabs(input->sampleRate - project.sampleRate) > kSampleRateTolerance ||
      std::fabs(output->sampleRate - project.sampleRate) > kSampleRateTolerance) {
    return {.error = DevicePrepareError::sampleRateMismatch};
  }

  const auto maxBlockSize = std::min(input->maxBlockSize, output->maxBlockSize);
  if (project.blockSize > maxBlockSize) {
    return {.error = DevicePrepareError::blockSizeTooLarge};
  }

  PreparedPassthrough passthrough{
    .inputDevice = *input,
    .outputDevice = *output,
    .selection = selection,
    .project = project,
    .scratch = std::vector<float>(maxBlockSize * output->outputChannels, 0.0f),
  };

  return {.error = DevicePrepareError::none, .passthrough = std::move(passthrough)};
}

void renderPassthrough(
  const PreparedPassthrough& prepared,
  std::span<const float> inputInterleaved,
  std::uint32_t frameCount,
  std::uint32_t inputChannelCount,
  std::span<float> outputInterleaved,
  std::uint32_t outputChannelCount,
  const dsp::OutputProtection& protection
) {
  std::fill(outputInterleaved.begin(), outputInterleaved.end(), 0.0f);
  if (!prepared.selection.monitoringEnabled) return;
  if (inputChannelCount == 0 || outputChannelCount == 0) return;
  if (prepared.selection.inputChannel >= inputChannelCount ||
      prepared.selection.outputChannel >= outputChannelCount) return;

  const std::uint32_t maxChunkFrames =
    static_cast<std::uint32_t>(prepared.scratch.size() / outputChannelCount);
  if (maxChunkFrames == 0) return;

  for (std::uint32_t offset = 0; offset < frameCount; offset += maxChunkFrames) {
    const auto chunkFrames = std::min(maxChunkFrames, frameCount - offset);
    for (std::uint32_t frame = 0; frame < chunkFrames; frame += 1) {
      const auto sourceFrame = offset + frame;
      const auto inIndex = sourceFrame * inputChannelCount + prepared.selection.inputChannel;
      const auto outIndex = sourceFrame * outputChannelCount + prepared.selection.outputChannel;
      if (inIndex < inputInterleaved.size() && outIndex < outputInterleaved.size()) {
        outputInterleaved[outIndex] = inputInterleaved[inIndex];
      }
    }
  }

  dsp::applyOutputProtection(outputInterleaved, protection);
}

}  // namespace localmixer::engine
