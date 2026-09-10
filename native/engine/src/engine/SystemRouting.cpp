#include "engine/SystemRouting.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace localmixer::engine {
namespace {

constexpr double kSampleRateTolerance = 0.01;

std::string lowerAscii(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return text;
}

std::optional<DeviceDescriptor> findByUid(std::span<const DeviceDescriptor> devices, const std::string& uid) {
  const auto it = std::find_if(devices.begin(), devices.end(), [&](const auto& device) {
    return device.uid == uid;
  });
  if (it == devices.end()) return std::nullopt;
  return *it;
}

bool isBlackHoleName(const std::string& text) {
  const auto lower = lowerAscii(text);
  return lower.find("blackhole") != std::string::npos || lower.find("black hole") != std::string::npos;
}

}  // namespace

std::optional<DeviceDescriptor> findBlackHoleDevice(std::span<const DeviceDescriptor> devices) {
  const auto it = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
    return isBlackHoleName(device.name) || isBlackHoleName(device.uid);
  });
  if (it == devices.end()) return std::nullopt;
  return *it;
}

bool isLoopbackDevice(const DeviceDescriptor& device) {
  if (isBlackHoleName(device.name) || isBlackHoleName(device.uid)) return true;
  const auto name = lowerAscii(device.name + " " + device.uid);
  return name.find("loopback") != std::string::npos ||
         name.find("soundflower") != std::string::npos ||
         name.find("aggregate") != std::string::npos;
}

SystemRouteDiagnostics validateSystemRoute(
  std::span<const DeviceDescriptor> devices,
  const ProjectAudioConfig& project,
  const SystemRouteSelection& requested
) {
  SystemRouteDiagnostics diagnostics;
  const auto detectedBlackHole = requested.blackHoleUid.empty()
    ? findBlackHoleDevice(devices)
    : findByUid(devices, requested.blackHoleUid);
  diagnostics.blackHoleAvailable = detectedBlackHole.has_value();
  if (!detectedBlackHole.has_value()) return diagnostics;

  const auto requestedOutput = requested.physicalOutputUid.empty()
    ? std::find_if(devices.begin(), devices.end(), [](const auto& device) {
        return device.isDefaultOutput && device.outputChannels > 0 && !isLoopbackDevice(device);
      })
    : devices.end();
  const auto physicalOutput = requested.physicalOutputUid.empty()
    ? (requestedOutput == devices.end() ? std::optional<DeviceDescriptor>{} : std::optional<DeviceDescriptor>{*requestedOutput})
    : findByUid(devices, requested.physicalOutputUid);
  if (!physicalOutput.has_value()) {
    diagnostics.error = SystemRouteError::outputNotFound;
    diagnostics.selection.blackHoleUid = detectedBlackHole->uid;
    return diagnostics;
  }

  diagnostics.selection = requested;
  diagnostics.selection.blackHoleUid = detectedBlackHole->uid;
  diagnostics.selection.physicalOutputUid = physicalOutput->uid;
  diagnostics.blackHoleSampleRate = detectedBlackHole->sampleRate;
  diagnostics.outputSampleRate = physicalOutput->sampleRate;
  diagnostics.selectedInputStart = requested.blackHoleInputStartChannel;
  diagnostics.selectedInputEnd = requested.blackHoleInputStartChannel + 1;
  diagnostics.selectedOutputStart = requested.physicalOutputStartChannel;
  diagnostics.selectedOutputEnd = requested.physicalOutputStartChannel + 1;

  if (isLoopbackDevice(*physicalOutput)) {
    diagnostics.error = SystemRouteError::outputIsLoopback;
    return diagnostics;
  }
  if (requested.blackHoleInputStartChannel + 1 >= detectedBlackHole->inputChannels) {
    diagnostics.error = SystemRouteError::blackHoleHasNoStereoInput;
    return diagnostics;
  }
  if (requested.physicalOutputStartChannel + 1 >= physicalOutput->outputChannels) {
    diagnostics.error = SystemRouteError::outputHasNoStereoOutput;
    return diagnostics;
  }
  if (std::fabs(detectedBlackHole->sampleRate - project.sampleRate) > kSampleRateTolerance ||
      std::fabs(physicalOutput->sampleRate - project.sampleRate) > kSampleRateTolerance) {
    diagnostics.error = SystemRouteError::sampleRateMismatch;
    return diagnostics;
  }

  diagnostics.routeValid = true;
  diagnostics.error = SystemRouteError::none;
  return diagnostics;
}

const char* systemRouteErrorName(SystemRouteError error) {
  switch (error) {
    case SystemRouteError::none: return "NONE";
    case SystemRouteError::blackHoleNotFound: return "BLACKHOLE_NOT_FOUND";
    case SystemRouteError::outputNotFound: return "OUTPUT_NOT_FOUND";
    case SystemRouteError::outputIsLoopback: return "OUTPUT_IS_LOOPBACK";
    case SystemRouteError::blackHoleHasNoStereoInput: return "BLACKHOLE_HAS_NO_STEREO_INPUT";
    case SystemRouteError::outputHasNoStereoOutput: return "OUTPUT_HAS_NO_STEREO_OUTPUT";
    case SystemRouteError::sampleRateMismatch: return "SAMPLE_RATE_MISMATCH";
    case SystemRouteError::engineNotReady: return "ENGINE_NOT_READY";
    case SystemRouteError::osApplyUnavailable: return "OS_APPLY_UNAVAILABLE";
    case SystemRouteError::noOwnedRoute: return "NO_OWNED_ROUTE";
  }
  return "UNKNOWN";
}

SystemRouteTransaction SystemRouteTransactionManager::requestEnable(
  const SystemRouteDiagnostics& diagnostics,
  std::string originalOutputUid,
  bool engineReady,
  bool osApplySupported
) {
  transaction_ = {
    .state = SystemRouteTransactionState::pending,
    .error = SystemRouteError::none,
    .ownsSystemRoute = false,
    .originalOutputUid = std::move(originalOutputUid),
    .selection = diagnostics.selection,
  };
  if (!engineReady) {
    transaction_.state = SystemRouteTransactionState::error;
    transaction_.error = SystemRouteError::engineNotReady;
    return transaction_;
  }
  if (!diagnostics.routeValid) {
    transaction_.state = SystemRouteTransactionState::error;
    transaction_.error = diagnostics.error;
    return transaction_;
  }
  if (!osApplySupported) {
    transaction_.state = SystemRouteTransactionState::error;
    transaction_.error = SystemRouteError::osApplyUnavailable;
    return transaction_;
  }

  transaction_.state = SystemRouteTransactionState::active;
  transaction_.ownsSystemRoute = true;
  transaction_.error = SystemRouteError::none;
  return transaction_;
}

SystemRouteTransaction SystemRouteTransactionManager::disable(bool osRestoreSupported) {
  if (!transaction_.ownsSystemRoute) {
    transaction_.state = SystemRouteTransactionState::idle;
    transaction_.error = SystemRouteError::noOwnedRoute;
    return transaction_;
  }

  transaction_.state = SystemRouteTransactionState::restoring;
  if (!osRestoreSupported) {
    transaction_.state = SystemRouteTransactionState::error;
    transaction_.error = SystemRouteError::osApplyUnavailable;
    return transaction_;
  }

  transaction_.state = SystemRouteTransactionState::idle;
  transaction_.error = SystemRouteError::none;
  transaction_.ownsSystemRoute = false;
  return transaction_;
}

const SystemRouteTransaction& SystemRouteTransactionManager::transaction() const {
  return transaction_;
}

const char* systemRouteTransactionStateName(SystemRouteTransactionState state) {
  switch (state) {
    case SystemRouteTransactionState::idle: return "IDLE";
    case SystemRouteTransactionState::pending: return "PENDING";
    case SystemRouteTransactionState::active: return "ACTIVE";
    case SystemRouteTransactionState::restoring: return "RESTORING";
    case SystemRouteTransactionState::error: return "ERROR";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::engine
