#include "engine/SystemRouting.hpp"

#include <iostream>
#include <vector>

namespace {

using localmixer::engine::DeviceDescriptor;
using localmixer::engine::ProjectAudioConfig;
using localmixer::engine::SystemRouteError;
using localmixer::engine::SystemRouteSelection;
using localmixer::engine::SystemRouteTransactionManager;
using localmixer::engine::SystemRouteTransactionState;
using localmixer::engine::validateSystemRoute;

std::vector<DeviceDescriptor> devices() {
  return {
    DeviceDescriptor{
      .uid = "blackhole-2ch",
      .name = "BlackHole 2ch",
      .inputChannels = 2,
      .outputChannels = 2,
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
  };
}

}  // namespace

int main() {
  const std::vector<DeviceDescriptor> noBlackHole{
    DeviceDescriptor{.uid = "phones", .name = "Headphones", .outputChannels = 2, .sampleRate = 48000.0},
  };
  const auto missing = validateSystemRoute(
    noBlackHole,
    ProjectAudioConfig{},
    SystemRouteSelection{}
  );
  if (missing.routeValid || missing.error != SystemRouteError::blackHoleNotFound) {
    std::cerr << "missing BlackHole should reject route\n";
    return 1;
  }

  const auto valid = validateSystemRoute(devices(), ProjectAudioConfig{}, SystemRouteSelection{});
  if (!valid.routeValid || !valid.blackHoleAvailable || valid.selectedInputStart != 0 || valid.selectedInputEnd != 1 ||
      valid.selectedOutputStart != 0 || valid.selectedOutputEnd != 1) {
    std::cerr << "default BlackHole to physical output route should validate\n";
    return 1;
  }

  const auto loopback = validateSystemRoute(
    devices(),
    ProjectAudioConfig{},
    SystemRouteSelection{.physicalOutputUid = "blackhole-2ch"}
  );
  if (loopback.routeValid || loopback.error != SystemRouteError::outputIsLoopback) {
    std::cerr << "loopback output should be rejected\n";
    return 1;
  }

  const auto badRate = validateSystemRoute(devices(), ProjectAudioConfig{.sampleRate = 44100.0}, SystemRouteSelection{});
  if (badRate.routeValid || badRate.error != SystemRouteError::sampleRateMismatch) {
    std::cerr << "sample-rate mismatch should be rejected\n";
    return 1;
  }

  const auto badRange = validateSystemRoute(
    devices(),
    ProjectAudioConfig{},
    SystemRouteSelection{.blackHoleInputStartChannel = 1}
  );
  if (badRange.routeValid || badRange.error != SystemRouteError::blackHoleHasNoStereoInput) {
    std::cerr << "invalid selected input range should be rejected\n";
    return 1;
  }

  SystemRouteTransactionManager manager;
  const auto notReady = manager.requestEnable(valid, "speakers", false, true);
  if (notReady.state != SystemRouteTransactionState::error || notReady.error != SystemRouteError::engineNotReady ||
      notReady.ownsSystemRoute) {
    std::cerr << "system route transaction should reject before engine ready\n";
    return 1;
  }

  const auto invalid = manager.requestEnable(missing, "speakers", true, true);
  if (invalid.state != SystemRouteTransactionState::error || invalid.error != SystemRouteError::blackHoleNotFound) {
    std::cerr << "system route transaction should preserve validation error\n";
    return 1;
  }

  const auto unavailable = manager.requestEnable(valid, "speakers", true, false);
  if (unavailable.state != SystemRouteTransactionState::error ||
      unavailable.error != SystemRouteError::osApplyUnavailable ||
      unavailable.ownsSystemRoute) {
    std::cerr << "system route transaction should report unavailable OS apply\n";
    return 1;
  }

  const auto active = manager.requestEnable(valid, "speakers", true, true);
  if (active.state != SystemRouteTransactionState::active || !active.ownsSystemRoute ||
      active.originalOutputUid != "speakers") {
    std::cerr << "system route transaction should own active route after apply\n";
    return 1;
  }
  const auto restored = manager.disable(true);
  if (restored.state != SystemRouteTransactionState::idle || restored.ownsSystemRoute ||
      restored.error != SystemRouteError::none) {
    std::cerr << "system route transaction should restore owned route\n";
    return 1;
  }
  const auto noOwned = manager.disable(true);
  if (noOwned.error != SystemRouteError::noOwnedRoute) {
    std::cerr << "system route transaction should not restore unowned route\n";
    return 1;
  }

  std::cout << "local-mixer-system-routing-tests ok\n";
  return 0;
}
