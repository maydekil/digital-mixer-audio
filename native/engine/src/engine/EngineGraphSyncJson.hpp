#pragma once

#include "engine/ChannelProcessorChain.hpp"
#include "engine/MixerGraphController.hpp"
#include "dsp/fx/EffectRack.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace localmixer::engine::protocol {

struct SyncedMonitorSource {
  std::string inputUid;
  bool fileSource = false;
  FxSendState sendA;
  FxSendState sendB;
  bool insertFxEnabled = false;
  std::vector<dsp::fx::RackSlotState> vocalFxSlots;
  float channelTrimDb = 0.0f;
  float channelFaderDb = 0.0f;
  float channelPan = 0.0f;
  bool channelStereo = false;
  ChannelProcessorConfig processors = defaultChannelProcessorConfig();
};

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
  bool insertFxEnabled = false;
  std::vector<dsp::fx::RackSlotState> vocalFxSlots;
  float monitorGainDb = -18.0f;
  float channelTrimDb = 0.0f;
  float channelFaderDb = 0.0f;
  float channelPan = 0.0f;
  bool channelStereo = false;
  std::vector<SyncedMonitorSource> sources;
  bool masterEnabled = true;
  bool masterMuted = false;
  float masterTrimDb = 0.0f;
  float masterFaderDb = 0.0f;
  ChannelProcessorConfig processors = defaultChannelProcessorConfig();
};

std::string syncMixerGraphResultJson(
  const std::string& line,
  MixerGraphController& controller,
  SyncedMonitorSelection& monitorSelection
);

}  // namespace localmixer::engine::protocol
