#pragma once

#include "engine/DeviceService.hpp"
#include "engine/SystemRouting.hpp"

#include <span>
#include <string>

namespace localmixer::engine::protocol {

std::string systemRouteDiagnosticsJson(
  std::span<const DeviceDescriptor> devices,
  const std::string& blackHoleUid,
  const std::string& outputUid,
  double sampleRate,
  std::uint32_t inputStartChannel,
  std::uint32_t outputStartChannel
);

std::string systemRouteTransactionJson(const SystemRouteTransaction& transaction);

}  // namespace localmixer::engine::protocol
