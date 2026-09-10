#pragma once

#include "engine/DeviceService.hpp"

#include <string>

namespace localmixer::engine {

enum class SystemRouteError {
  none,
  blackHoleNotFound,
  outputNotFound,
  outputIsLoopback,
  blackHoleHasNoStereoInput,
  outputHasNoStereoOutput,
  sampleRateMismatch,
  engineNotReady,
  osApplyUnavailable,
  noOwnedRoute,
};

enum class SystemRouteTransactionState {
  idle,
  pending,
  active,
  restoring,
  error,
};

struct SystemRouteSelection {
  std::string blackHoleUid;
  std::string physicalOutputUid;
  std::uint32_t blackHoleInputStartChannel = 0;
  std::uint32_t physicalOutputStartChannel = 0;
};

struct SystemRouteDiagnostics {
  bool blackHoleAvailable = false;
  bool routeValid = false;
  SystemRouteError error = SystemRouteError::blackHoleNotFound;
  SystemRouteSelection selection;
  std::uint32_t selectedInputStart = 0;
  std::uint32_t selectedInputEnd = 0;
  std::uint32_t selectedOutputStart = 0;
  std::uint32_t selectedOutputEnd = 0;
  double blackHoleSampleRate = 0.0;
  double outputSampleRate = 0.0;
};

struct SystemRouteTransaction {
  SystemRouteTransactionState state = SystemRouteTransactionState::idle;
  SystemRouteError error = SystemRouteError::none;
  bool ownsSystemRoute = false;
  std::string originalOutputUid;
  SystemRouteSelection selection;
};

class SystemRouteTransactionManager {
 public:
  SystemRouteTransaction requestEnable(
    const SystemRouteDiagnostics& diagnostics,
    std::string originalOutputUid,
    bool engineReady,
    bool osApplySupported
  );
  SystemRouteTransaction disable(bool osRestoreSupported);
  const SystemRouteTransaction& transaction() const;

 private:
  SystemRouteTransaction transaction_;
};

std::optional<DeviceDescriptor> findBlackHoleDevice(std::span<const DeviceDescriptor> devices);
bool isLoopbackDevice(const DeviceDescriptor& device);
SystemRouteDiagnostics validateSystemRoute(
  std::span<const DeviceDescriptor> devices,
  const ProjectAudioConfig& project,
  const SystemRouteSelection& requested
);
const char* systemRouteErrorName(SystemRouteError error);
const char* systemRouteTransactionStateName(SystemRouteTransactionState state);

}  // namespace localmixer::engine
