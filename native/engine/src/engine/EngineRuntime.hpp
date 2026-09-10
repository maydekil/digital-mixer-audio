#pragma once

#include "engine/DeviceService.hpp"

#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class RuntimeState {
  idle,
  deviceReady,
  monitoringOff,
  error,
};

struct RuntimeStatus {
  RuntimeState state = RuntimeState::idle;
  std::string error;
  std::size_t deviceCount = 0;
  bool monitoringEnabled = false;
  std::string inputUid;
  std::string outputUid;
};

struct PreparePassthroughRequest {
  ProjectAudioConfig project;
  std::string inputUid;
  std::string outputUid;
  std::uint32_t inputChannel = 0;
  std::uint32_t outputChannel = 0;
  bool monitoringEnabled = false;
};

class EngineRuntime {
 public:
  EngineRuntime();

  void refreshDevices(std::vector<DeviceDescriptor> devices);
  const std::vector<DeviceDescriptor>& devices() const;
  RuntimeStatus status() const;
  PrepareResult preparePassthrough(const PreparePassthroughRequest& request);
  void clearError();

 private:
  std::optional<DeviceDescriptor> defaultInput() const;
  std::optional<DeviceDescriptor> defaultOutput() const;
  void setError(DevicePrepareError error);

  DeviceService service_;
  RuntimeStatus status_;
  std::optional<PreparedPassthrough> prepared_;
};

const char* runtimeStateName(RuntimeState state);
const char* prepareErrorName(DevicePrepareError error);

}  // namespace localmixer::engine
