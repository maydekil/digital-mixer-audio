#include "engine/EngineRuntime.hpp"

#include <iostream>

namespace {

using localmixer::engine::DeviceDescriptor;
using localmixer::engine::DevicePrepareError;
using localmixer::engine::EngineRuntime;
using localmixer::engine::PreparePassthroughRequest;
using localmixer::engine::RuntimeState;

}  // namespace

int main() {
  EngineRuntime runtime;
  runtime.refreshDevices({});

  auto missing = runtime.preparePassthrough(PreparePassthroughRequest{});
  if (missing.error != DevicePrepareError::inputNotFound ||
      runtime.status().state != RuntimeState::error) {
    std::cerr << "missing input should produce explicit error state\n";
    return 1;
  }

  runtime.refreshDevices({
    DeviceDescriptor{
      .uid = "mic",
      .name = "Built-in Mic",
      .isDefaultInput = true,
      .inputChannels = 1,
      .sampleRate = 48000.0,
      .maxBlockSize = 256,
    },
    DeviceDescriptor{
      .uid = "phones",
      .name = "Headphones",
      .isDefaultOutput = true,
      .outputChannels = 2,
      .sampleRate = 48000.0,
      .maxBlockSize = 256,
    },
  });

  if (runtime.status().state != RuntimeState::deviceReady || runtime.status().deviceCount != 2) {
    std::cerr << "refresh should publish device-ready status\n";
    return 1;
  }

  const auto prepared = runtime.preparePassthrough(PreparePassthroughRequest{});
  if (prepared.error != DevicePrepareError::none ||
      runtime.status().state != RuntimeState::monitoringOff ||
      runtime.status().monitoringEnabled ||
      runtime.status().inputUid != "mic" ||
      runtime.status().outputUid != "phones") {
    std::cerr << "default passthrough should prepare with monitoring off\n";
    return 1;
  }

  const auto mismatch = runtime.preparePassthrough(PreparePassthroughRequest{
    .project = {.sampleRate = 44100.0, .blockSize = 256},
  });
  if (mismatch.error != DevicePrepareError::sampleRateMismatch ||
      runtime.status().state != RuntimeState::error ||
      runtime.status().error != "SAMPLE_RATE_MISMATCH") {
    std::cerr << "rate mismatch should become explicit runtime error\n";
    return 1;
  }

  std::cout << "local-mixer-engine-runtime-tests ok\n";
  return 0;
}
