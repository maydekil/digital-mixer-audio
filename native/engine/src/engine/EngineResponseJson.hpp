#pragma once

#include "engine/ChannelHarmonyController.hpp"
#include "engine/DeviceService.hpp"
#include "engine/EngineRuntime.hpp"
#include "engine/FxProgramController.hpp"

#include <cstdint>
#include <span>
#include <string>

namespace localmixer::engine::protocol {

std::string devicesJson(std::span<const DeviceDescriptor> devices);
std::string statusJson(const RuntimeStatus& status);
std::string persistentMonitorStatusJson(
  bool running,
  const std::string& error,
  std::uint32_t inputChannels,
  std::uint32_t outputChannels,
  double inputSampleRate,
  double outputSampleRate,
  float inputPeak,
  float inputPeakLeft = 0.0f,
  float inputPeakRight = 0.0f,
  std::uint64_t callbackCount = 0,
  std::uint64_t deadlineMissCount = 0,
  std::uint64_t maxCallbackNanos = 0
);
std::string fxProgramSnapshotJson(const FxProgramUnitSnapshot& snapshot);
std::string fxProgramAckJson(const FxProgramAck& ack);
std::string harmonyStateJson(const ChannelHarmonyState& state);
std::string harmonyAckJson(const HarmonyCommandAck& ack);

}  // namespace localmixer::engine::protocol
