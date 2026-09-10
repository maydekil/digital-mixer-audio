#include "engine/EngineRuntime.hpp"

#include <algorithm>
#include <utility>

namespace localmixer::engine {

EngineRuntime::EngineRuntime() : service_({}) {}

void EngineRuntime::refreshDevices(std::vector<DeviceDescriptor> devices) {
  service_ = DeviceService(std::move(devices));
  prepared_.reset();
  status_ = RuntimeStatus{
    .state = service_.devices().empty() ? RuntimeState::idle : RuntimeState::deviceReady,
    .deviceCount = service_.devices().size(),
  };
}

const std::vector<DeviceDescriptor>& EngineRuntime::devices() const {
  return service_.devices();
}

RuntimeStatus EngineRuntime::status() const {
  return status_;
}

PrepareResult EngineRuntime::preparePassthrough(const PreparePassthroughRequest& request) {
  const auto input = request.inputUid.empty() ? defaultInput() : service_.findByUid(request.inputUid);
  if (!input.has_value()) {
    setError(DevicePrepareError::inputNotFound);
    return {.error = DevicePrepareError::inputNotFound};
  }

  const auto output = request.outputUid.empty() ? defaultOutput() : service_.findByUid(request.outputUid);
  if (!output.has_value()) {
    setError(DevicePrepareError::outputNotFound);
    return {.error = DevicePrepareError::outputNotFound};
  }

  auto result = service_.preparePassthrough(
    request.project,
    PassthroughSelection{
      .inputUid = input->uid,
      .outputUid = output->uid,
      .inputChannel = request.inputChannel,
      .outputChannel = request.outputChannel,
      .monitoringEnabled = request.monitoringEnabled,
    }
  );

  if (result.error != DevicePrepareError::none || !result.passthrough.has_value()) {
    setError(result.error);
    return result;
  }

  prepared_ = result.passthrough;
  status_ = RuntimeStatus{
    .state = request.monitoringEnabled ? RuntimeState::deviceReady : RuntimeState::monitoringOff,
    .deviceCount = service_.devices().size(),
    .monitoringEnabled = request.monitoringEnabled,
    .inputUid = input->uid,
    .outputUid = output->uid,
  };

  return result;
}

void EngineRuntime::clearError() {
  if (status_.state != RuntimeState::error) return;
  status_.state = service_.devices().empty() ? RuntimeState::idle : RuntimeState::deviceReady;
  status_.error.clear();
}

std::optional<DeviceDescriptor> EngineRuntime::defaultInput() const {
  const auto& devices = service_.devices();
  auto it = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
    return device.isDefaultInput && device.inputChannels > 0;
  });
  if (it != devices.end()) return *it;

  it = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
    return device.inputChannels > 0;
  });
  if (it == devices.end()) return std::nullopt;
  return *it;
}

std::optional<DeviceDescriptor> EngineRuntime::defaultOutput() const {
  const auto& devices = service_.devices();
  auto it = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
    return device.isDefaultOutput && device.outputChannels > 0;
  });
  if (it != devices.end()) return *it;

  it = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
    return device.outputChannels > 0;
  });
  if (it == devices.end()) return std::nullopt;
  return *it;
}

void EngineRuntime::setError(DevicePrepareError error) {
  status_.state = RuntimeState::error;
  status_.error = prepareErrorName(error);
  status_.deviceCount = service_.devices().size();
}

const char* runtimeStateName(RuntimeState state) {
  switch (state) {
    case RuntimeState::idle: return "IDLE";
    case RuntimeState::deviceReady: return "DEVICE_READY";
    case RuntimeState::monitoringOff: return "MONITORING_OFF";
    case RuntimeState::error: return "ERROR";
  }
  return "ERROR";
}

const char* prepareErrorName(DevicePrepareError error) {
  switch (error) {
    case DevicePrepareError::none: return "NONE";
    case DevicePrepareError::inputNotFound: return "INPUT_NOT_FOUND";
    case DevicePrepareError::outputNotFound: return "OUTPUT_NOT_FOUND";
    case DevicePrepareError::inputChannelOutOfRange: return "INPUT_CHANNEL_OUT_OF_RANGE";
    case DevicePrepareError::outputChannelOutOfRange: return "OUTPUT_CHANNEL_OUT_OF_RANGE";
    case DevicePrepareError::sampleRateMismatch: return "SAMPLE_RATE_MISMATCH";
    case DevicePrepareError::blockSizeTooLarge: return "BLOCK_SIZE_TOO_LARGE";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::engine
