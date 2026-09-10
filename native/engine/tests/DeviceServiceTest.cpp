#include "engine/DeviceService.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::engine::DeviceDescriptor;
using localmixer::engine::DevicePrepareError;
using localmixer::engine::DeviceService;
using localmixer::engine::PassthroughSelection;
using localmixer::engine::ProjectAudioConfig;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

DeviceService makeService() {
  return DeviceService({
    DeviceDescriptor{
      .uid = "input-a",
      .name = "Input A",
      .isDefaultInput = true,
      .inputChannels = 2,
      .sampleRate = 48000.0,
      .maxBlockSize = 4,
    },
    DeviceDescriptor{
      .uid = "output-a",
      .name = "Output A",
      .isDefaultOutput = true,
      .outputChannels = 2,
      .sampleRate = 48000.0,
      .maxBlockSize = 4,
    },
  });
}

}  // namespace

int main() {
  const auto service = makeService();
  const auto input = service.findByUid("input-a");
  if (!input.has_value() || input->name != "Input A") {
    std::cerr << "UID lookup failed\n";
    return 1;
  }

  const auto labels = service.labelsFor(*input);
  if (labels.inputs.size() != 2 || labels.inputs[0] != "In 1" || labels.inputs[1] != "In 2") {
    std::cerr << "input labels mismatch\n";
    return 1;
  }

  const auto mismatch = service.preparePassthrough(
    ProjectAudioConfig{.sampleRate = 44100.0, .blockSize = 4},
    PassthroughSelection{.inputUid = "input-a", .outputUid = "output-a"}
  );
  if (mismatch.error != DevicePrepareError::sampleRateMismatch) {
    std::cerr << "sample-rate mismatch should be rejected\n";
    return 1;
  }

  const auto oversized = service.preparePassthrough(
    ProjectAudioConfig{.sampleRate = 48000.0, .blockSize = 8},
    PassthroughSelection{.inputUid = "input-a", .outputUid = "output-a"}
  );
  if (oversized.error != DevicePrepareError::blockSizeTooLarge) {
    std::cerr << "oversized block should be rejected at prepare time\n";
    return 1;
  }

  const auto prepared = service.preparePassthrough(
    ProjectAudioConfig{.sampleRate = 48000.0, .blockSize = 4},
    PassthroughSelection{
      .inputUid = "input-a",
      .outputUid = "output-a",
      .inputChannel = 1,
      .outputChannel = 0,
      .monitoringEnabled = false,
    }
  );
  if (prepared.error != DevicePrepareError::none || !prepared.passthrough.has_value()) {
    std::cerr << "prepare passthrough failed\n";
    return 1;
  }

  std::array<float, 12> inputFrames{0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f};
  std::array<float, 12> outputFrames;
  outputFrames.fill(9.0f);

  localmixer::engine::renderPassthrough(
    *prepared.passthrough,
    inputFrames,
    6,
    2,
    outputFrames,
    2,
    localmixer::dsp::OutputProtection{}
  );
  for (const auto sample : outputFrames) {
    if (!near(sample, 0.0f)) {
      std::cerr << "monitoring off must zero output\n";
      return 1;
    }
  }

  auto active = *prepared.passthrough;
  active.selection.monitoringEnabled = true;
  localmixer::engine::renderPassthrough(
    active,
    inputFrames,
    6,
    2,
    outputFrames,
    2,
    localmixer::dsp::OutputProtection{.masterGainDb = 0.0f, .muted = false, .limitCeiling = 0.75f}
  );

  const std::array<float, 12> expected{0.2f, 0.0f, 0.4f, 0.0f, 0.6f, 0.0f, 0.75f, 0.0f, 0.75f, 0.0f, 0.75f, 0.0f};
  for (std::size_t index = 0; index < expected.size(); index += 1) {
    if (!near(outputFrames[index], expected[index])) {
      std::cerr << "passthrough render mismatch\n";
      return 1;
    }
  }

  std::cout << "local-mixer-device-service-tests ok\n";
  return 0;
}
