#pragma once

#include "engine/ChannelProcessorChain.hpp"
#include "engine/FxSendReturnBus.hpp"
#include "engine/RealtimeMetrics.hpp"
#include "dsp/fx/EffectRack.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace localmixer::platform::macos {

struct PassthroughMonitorRequest {
  std::string inputUid;
  std::string outputUid;
  double projectSampleRate = 48000.0;
  std::uint32_t inputChannel = 0;
  std::uint32_t outputChannel = 0;
  bool mirrorToAllOutputChannels = true;
  bool stereoInput = false;
  std::uint32_t durationMs = 750;
  float monitorGainDb = -24.0f;
  float monitorPan = 0.0f;
  localmixer::engine::ChannelProcessorConfig processors = localmixer::engine::defaultChannelProcessorConfig();
  std::uint32_t fxAProgramId = 12;
  std::uint32_t fxBProgramId = 50;
  localmixer::engine::FxUnitRuntime fxA;
  localmixer::engine::FxUnitRuntime fxB;
  localmixer::engine::FxSendState sendA;
  localmixer::engine::FxSendState sendB;
  bool insertFxEnabled = false;
  std::vector<localmixer::dsp::fx::RackSlotState> vocalFxSlots;
};

struct PassthroughMonitorResult {
  bool ok = false;
  std::string error;
  std::uint32_t inputChannels = 0;
  std::uint32_t outputChannels = 0;
  double inputSampleRate = 0.0;
  double outputSampleRate = 0.0;
  float inputPeak = 0.0f;
};

struct PersistentMonitorStatus {
  bool running = false;
  std::string error;
  std::uint32_t inputChannels = 0;
  std::uint32_t outputChannels = 0;
  double inputSampleRate = 0.0;
  double outputSampleRate = 0.0;
  float inputPeak = 0.0f;
  localmixer::engine::RealtimeMetricsSnapshot metrics;
};

class PersistentPassthroughMonitor {
 public:
  PersistentPassthroughMonitor();
  ~PersistentPassthroughMonitor();

  PersistentPassthroughMonitor(const PersistentPassthroughMonitor&) = delete;
  PersistentPassthroughMonitor& operator=(const PersistentPassthroughMonitor&) = delete;

  PersistentMonitorStatus start(const PassthroughMonitorRequest& request);
  PersistentMonitorStatus stop();
  PersistentMonitorStatus status() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

PassthroughMonitorResult monitorPassthrough(const PassthroughMonitorRequest& request);

}  // namespace localmixer::platform::macos
