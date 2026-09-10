#pragma once

#include "engine/MixerGraphController.hpp"

#include <cstdint>
#include <string>

namespace localmixer::engine::protocol {

struct SyncedMonitorSelection {
  std::string inputUid;
  std::string outputUid;
  std::uint32_t activeMonitorCount = 0;
  float monitorGainDb = -18.0f;
  float channelTrimDb = 0.0f;
  float channelFaderDb = 0.0f;
  float channelPan = 0.0f;
};

std::string syncMixerGraphResultJson(
  const std::string& line,
  MixerGraphController& controller,
  SyncedMonitorSelection& monitorSelection
);

}  // namespace localmixer::engine::protocol
