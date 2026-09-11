#include "engine/EngineGraphSyncJson.hpp"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool near(double actual, double expected) {
  return std::fabs(actual - expected) < 0.0001;
}

bool contains(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}

}  // namespace

int main() {
  localmixer::engine::MixerGraphController controller;
  localmixer::engine::protocol::SyncedMonitorSelection selection;
  const std::string payload =
    "{\"channelCount\":2,"
    "\"outputUid\":\"headphones\","
    "\"monitorGainDb\":-18,"
    "\"fxAEnabled\":true,"
    "\"fxAProgramId\":12,"
    "\"fxAReturnDb\":-6,"
    "\"fxBEnabled\":false,"
    "\"fxBProgramId\":50,"
    "\"fxBReturnDb\":-12,"
    "\"vocalFxSlotCount\":2,"
    "\"vocalFxSlot0Id\":\"drive\","
    "\"vocalFxSlot0Type\":\"saturation\","
    "\"vocalFxSlot0Enabled\":true,"
    "\"vocalFxSlot1Id\":\"room\","
    "\"vocalFxSlot1Type\":\"reverb\","
    "\"vocalFxSlot1Enabled\":false,"
    "\"channel0Kind\":\"source\","
    "\"channel0Name\":\"VOICE\","
    "\"channel0Color\":\"#18d6e7\","
    "\"channel0SourceUid\":\"mic\","
    "\"channel0Assignment\":\"mono\","
    "\"channel0Enabled\":true,"
    "\"channel0Mute\":false,"
    "\"channel0Solo\":false,"
    "\"channel0Monitor\":true,"
    "\"channel0ProcessorEq\":true,"
    "\"channel0ProcessorComp\":true,"
    "\"channel0ProcessorNoise\":true,"
    "\"channel0ProcessorInsertFx\":true,"
    "\"channel0ProcessorDeEsser\":true,"
    "\"channel0NoiseThresholdDb\":-48,"
    "\"channel0NoiseRangeDb\":-72,"
    "\"channel0NoiseHoldMs\":40,"
    "\"channel0NoiseReleaseMs\":120,"
    "\"channel0CompThresholdDb\":-22,"
    "\"channel0CompRatio\":4,"
    "\"channel0CompAttackMs\":12,"
    "\"channel0CompReleaseMs\":150,"
    "\"channel0DeEsserFrequencyHz\":7200,"
    "\"channel0DeEsserThresholdDb\":-30,"
    "\"channel0DeEsserMaxReductionDb\":8,"
    "\"channel0SendAEnabled\":true,"
    "\"channel0SendAGainDb\":-9,"
    "\"channel0SendBEnabled\":false,"
    "\"channel0SendBGainDb\":-24,"
    "\"channel0Eq0FreqHz\":120,"
    "\"channel0Eq0GainDb\":5,"
    "\"channel0Eq0Q\":0.8,"
    "\"channel0Eq0Type\":\"Low Shelf\","
    "\"channel0TrimDb\":1,"
    "\"channel0FaderDb\":-6,"
    "\"channel0Pan\":0.25,"
    "\"channel1Kind\":\"master\","
    "\"channel1Name\":\"MASTER\","
    "\"channel1Color\":\"#20f0a0\","
    "\"channel1SourceUid\":\"\","
    "\"channel1Assignment\":\"stereo\","
    "\"channel1Enabled\":true,"
    "\"channel1Mute\":false,"
    "\"channel1Solo\":false,"
    "\"channel1Monitor\":false,"
    "\"channel1ProcessorEq\":false,"
    "\"channel1ProcessorComp\":false,"
    "\"channel1ProcessorNoise\":false,"
    "\"channel1ProcessorInsertFx\":false,"
    "\"channel1ProcessorDeEsser\":false,"
    "\"channel1SendAEnabled\":false,"
    "\"channel1SendAGainDb\":0,"
    "\"channel1SendBEnabled\":false,"
    "\"channel1SendBGainDb\":0,"
    "\"channel1TrimDb\":2,"
    "\"channel1FaderDb\":-3,"
    "\"channel1Pan\":0}";

  const auto response = localmixer::engine::protocol::syncMixerGraphResultJson(payload, controller, selection);
  if (!contains(response, "\"synced\":true") || selection.inputUid != "mic" || selection.outputUid != "headphones") {
    std::cerr << "sync response should publish monitored source selection\n";
    return 1;
  }
  if (!selection.processors.eqEnabled || !selection.processors.compressorEnabled || !selection.processors.noiseEnabled ||
      !selection.processors.deEsserEnabled) {
    std::cerr << "monitor selection should keep processor enable flags\n";
    return 1;
  }
  if (!near(selection.processors.noise.thresholdDb, -48.0) ||
      !near(selection.processors.noise.rangeDb, -72.0) ||
      !near(selection.processors.noise.holdMs, 40.0) ||
      !near(selection.processors.noise.releaseMs, 120.0)) {
    std::cerr << "monitor selection should keep noise parameters\n";
    return 1;
  }
  if (!near(selection.processors.compressor.thresholdDb, -22.0) ||
      !near(selection.processors.compressor.ratio, 4.0) ||
      !near(selection.processors.compressor.attackMs, 12.0) ||
      !near(selection.processors.compressor.releaseMs, 150.0)) {
    std::cerr << "monitor selection should keep compressor parameters\n";
    return 1;
  }
  if (!near(selection.processors.deEsser.detectorFrequencyHz, 7200.0) ||
      !near(selection.processors.deEsser.thresholdDb, -30.0) ||
      !near(selection.processors.deEsser.maxReductionDb, 8.0)) {
    std::cerr << "monitor selection should keep de-esser parameters\n";
    return 1;
  }
  if (!near(selection.processors.eqBands[0].frequencyHz, 120.0) ||
      !near(selection.processors.eqBands[0].gainDb, 5.0) ||
      !near(selection.processors.eqBands[0].q, 0.8)) {
    std::cerr << "monitor selection should keep EQ band parameters\n";
    return 1;
  }
  if (selection.fxAProgramId != 12 || selection.fxBProgramId != 50 || !selection.fxA.enabled ||
      !near(selection.fxA.returnDb, -6.0) || selection.fxB.enabled || !near(selection.fxB.returnDb, -12.0) ||
      !selection.sendA.enabled || !near(selection.sendA.gainDb, -9.0) || selection.sendB.enabled ||
      !near(selection.sendB.gainDb, -24.0)) {
    std::cerr << "monitor selection should keep FX unit, program, and send state\n";
    return 1;
  }
  if (!selection.insertFxEnabled || selection.vocalFxSlots.size() != 2 || selection.vocalFxSlots[0].effectType != "saturation" ||
      selection.vocalFxSlots[0].bypassed || selection.vocalFxSlots[1].effectType != "reverb" ||
      !selection.vocalFxSlots[1].bypassed) {
    std::cerr << "monitor selection should keep Vocal FX rack slot state\n";
    return 1;
  }
  if (!near(selection.channelTrimDb, 1.0) || !near(selection.channelFaderDb, -6.0) || !near(selection.channelPan, 0.25)) {
    std::cerr << "monitor selection should keep gain and pan\n";
    return 1;
  }
  if (!selection.masterEnabled || selection.masterMuted || !near(selection.masterTrimDb, 2.0) ||
      !near(selection.masterFaderDb, -3.0)) {
    std::cerr << "monitor selection should keep master output state\n";
    return 1;
  }
  const auto published = controller.active().strip(localmixer::engine::StripId{1});
  if (!published.has_value() || !published->sendA.enabled || !near(published->sendA.gainDb, -9.0) ||
      published->sendB.enabled || !near(published->sendB.gainDb, -24.0)) {
    std::cerr << "published graph should keep FX send states\n";
    return 1;
  }

  localmixer::engine::MixerGraphController mutedController;
  localmixer::engine::protocol::SyncedMonitorSelection mutedSelection;
  const std::string mutedPayload =
    "{\"channelCount\":1,"
    "\"outputUid\":\"headphones\","
    "\"monitorGainDb\":-18,"
    "\"channel0Kind\":\"source\","
    "\"channel0Name\":\"VOICE\","
    "\"channel0Color\":\"#18d6e7\","
    "\"channel0SourceUid\":\"mic\","
    "\"channel0Assignment\":\"mono\","
    "\"channel0Enabled\":true,"
    "\"channel0Mute\":true,"
    "\"channel0Solo\":false,"
    "\"channel0Monitor\":true,"
    "\"channel0ProcessorEq\":false,"
    "\"channel0ProcessorComp\":false,"
    "\"channel0ProcessorNoise\":false,"
    "\"channel0ProcessorInsertFx\":false,"
    "\"channel0ProcessorDeEsser\":false,"
    "\"channel0SendAEnabled\":false,"
    "\"channel0SendAGainDb\":0,"
    "\"channel0SendBEnabled\":false,"
    "\"channel0SendBGainDb\":0,"
    "\"channel0TrimDb\":0,"
    "\"channel0FaderDb\":0,"
    "\"channel0Pan\":0}";
  const auto mutedResponse = localmixer::engine::protocol::syncMixerGraphResultJson(
    mutedPayload, mutedController, mutedSelection);
  if (!contains(mutedResponse, "\"synced\":true") || mutedSelection.activeMonitorCount != 0 ||
      !mutedSelection.inputUid.empty()) {
    std::cerr << "muted monitored source should not feed the persistent monitor\n";
    return 1;
  }

  const std::string multiMonitorPayload =
    "{\"channelCount\":2,"
    "\"channel0Kind\":\"source\","
    "\"channel0Name\":\"VOICE\","
    "\"channel0SourceUid\":\"mic-a\","
    "\"channel0Assignment\":\"mono\","
    "\"channel0Enabled\":true,"
    "\"channel0Mute\":false,"
    "\"channel0Monitor\":true,"
    "\"channel0TrimDb\":0,"
    "\"channel0FaderDb\":0,"
    "\"channel0Pan\":0,"
    "\"channel1Kind\":\"source\","
    "\"channel1Name\":\"GUITAR\","
    "\"channel1SourceUid\":\"mic-b\","
    "\"channel1Assignment\":\"mono\","
    "\"channel1Enabled\":true,"
    "\"channel1Mute\":false,"
    "\"channel1Monitor\":true,"
    "\"channel1TrimDb\":0,"
    "\"channel1FaderDb\":0,"
    "\"channel1Pan\":0}";
  const auto multiMonitorResponse = localmixer::engine::protocol::syncMixerGraphResultJson(
    multiMonitorPayload, controller, selection);
  if (!contains(multiMonitorResponse, "\"synced\":false") || selection.activeMonitorCount != 0 ||
      !selection.inputUid.empty()) {
    std::cerr << "rejected monitor sync should clear stale monitor selection\n";
    return 1;
  }

  std::cout << "local-mixer-engine-graph-sync-json-tests ok\n";
  return 0;
}
