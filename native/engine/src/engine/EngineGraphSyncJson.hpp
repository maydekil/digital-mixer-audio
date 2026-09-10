#pragma once

#include "engine/ChannelProcessorChain.hpp"
#include "engine/MixerGraphController.hpp"

#include <cstdint>
#include <string>

namespace localmixer::engine::protocol {

struct SyncedMonitorSelection {
  std::string inputUid;
  std::string outputUid;
  std::uint32_t activeMonitorCount = 0;
  std::uint32_t fxAProgramId = 12;
  std::uint32_t fxBProgramId = 50;
  FxUnitRuntime fxA;
  FxUnitRuntime fxB;
  FxSendState sendA;
  FxSendState sendB;
  float monitorGainDb = -18.0f;
  float channelTrimDb = 0.0f;
  float channelFaderDb = 0.0f;
  float channelPan = 0.0f;
  ChannelProcessorConfig processors = defaultChannelProcessorConfig();
};

std::string syncMixerGraphResultJson(
  const std::string& line,
  MixerGraphController& controller,
  SyncedMonitorSelection& monitorSelection
);

}  // namespace localmixer::engine::protocol
