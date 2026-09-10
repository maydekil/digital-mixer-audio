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

std::optional<DeviceDescriptor> findBlackHoleDevice(std::span<const DeviceDescriptor> devices);
bool isLoopbackDevice(const DeviceDescriptor& device);
SystemRouteDiagnostics validateSystemRoute(
  std::span<const DeviceDescriptor> devices,
  const ProjectAudioConfig& project,
  const SystemRouteSelection& requested
);
const char* systemRouteErrorName(SystemRouteError error);

}  // namespace localmixer::engine
