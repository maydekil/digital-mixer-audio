#include "engine/EngineResponseJson.hpp"

#include "engine/JsonProtocol.hpp"

namespace localmixer::engine::protocol {

std::string devicesJson(std::span<const DeviceDescriptor> devices) {
  std::string json = "[";
  for (std::size_t index = 0; index < devices.size(); index += 1) {
    const auto& device = devices[index];
    if (index > 0) json += ",";
    json += "{\"uid\":\"" + escapeJson(device.uid) + "\"";
    json += ",\"name\":\"" + escapeJson(device.name) + "\"";
    json += ",\"defaultInput\":" + std::string(device.isDefaultInput ? "true" : "false");
    json += ",\"defaultOutput\":" + std::string(device.isDefaultOutput ? "true" : "false");
    json += ",\"inputChannels\":" + std::to_string(device.inputChannels);
    json += ",\"outputChannels\":" + std::to_string(device.outputChannels);
    json += ",\"sampleRate\":" + std::to_string(device.sampleRate);
    json += ",\"maxBlockSize\":" + std::to_string(device.maxBlockSize);
    json += "}";
  }
  json += "]";
  return json;
}

std::string statusJson(const RuntimeStatus& status) {
  std::string json = "{\"state\":\"";
  json += runtimeStateName(status.state);
  json += "\",\"deviceCount\":" + std::to_string(status.deviceCount);
  json += ",\"monitoringEnabled\":" + std::string(status.monitoringEnabled ? "true" : "false");
  json += ",\"inputUid\":\"" + escapeJson(status.inputUid) + "\"";
  json += ",\"outputUid\":\"" + escapeJson(status.outputUid) + "\"";
  if (!status.error.empty()) json += ",\"error\":\"" + escapeJson(status.error) + "\"";
  json += "}";
  return json;
}

std::string persistentMonitorStatusJson(
  bool running,
  const std::string& error,
  std::uint32_t inputChannels,
  std::uint32_t outputChannels,
  double inputSampleRate,
  double outputSampleRate,
  float inputPeak,
  float inputPeakLeft,
  float inputPeakRight,
  std::uint64_t callbackCount,
  std::uint64_t deadlineMissCount,
  std::uint64_t maxCallbackNanos
) {
  std::string json = "\"monitoring\":" + std::string(running ? "true" : "false");
  json += ",\"error\":\"" + escapeJson(error) + "\"";
  json += ",\"inputChannels\":" + std::to_string(inputChannels);
  json += ",\"outputChannels\":" + std::to_string(outputChannels);
  json += ",\"inputSampleRate\":" + std::to_string(inputSampleRate);
  json += ",\"outputSampleRate\":" + std::to_string(outputSampleRate);
  json += ",\"inputPeak\":" + std::to_string(inputPeak);
  json += ",\"inputPeakLeft\":" + std::to_string(inputPeakLeft);
  json += ",\"inputPeakRight\":" + std::to_string(inputPeakRight);
  json += ",\"callbackCount\":" + std::to_string(callbackCount);
  json += ",\"deadlineMissCount\":" + std::to_string(deadlineMissCount);
  json += ",\"maxCallbackNanos\":" + std::to_string(maxCallbackNanos);
  return json;
}

std::string fxProgramSnapshotJson(const FxProgramUnitSnapshot& snapshot) {
  std::string json = "\"programId\":" + std::to_string(snapshot.programId);
  json += ",\"previousProgramId\":" + std::to_string(snapshot.previousProgramId);
  json += ",\"revision\":" + std::to_string(snapshot.revision);
  json += ",\"modified\":" + std::string(snapshot.modified ? "true" : "false");
  json += ",\"pending\":" + std::string(snapshot.pending ? "true" : "false");
  json += ",\"transitionActive\":" + std::string(snapshot.transitionActive ? "true" : "false");
  json += ",\"crossfadeFramesRemaining\":" + std::to_string(snapshot.crossfadeFramesRemaining);
  return json;
}

std::string fxProgramAckJson(const FxProgramAck& ack) {
  std::string json = "\"accepted\":" + std::string(ack.accepted ? "true" : "false");
  json += ",\"applied\":" + std::string(ack.applied ? "true" : "false");
  json += ",\"error\":\"" + std::string(fxProgramErrorName(ack.error)) + "\"";
  json += ",\"unitId\":\"" + std::string(fxUnitName(ack.unit)) + "\"";
  json += ",\"requestedProgramId\":" + std::to_string(ack.requestedProgramId);
  json += "," + fxProgramSnapshotJson(ack.snapshot);
  return json;
}

std::string harmonyStateJson(const ChannelHarmonyState& state) {
  std::string json = "\"channelId\":\"" + escapeJson(state.channelId) + "\"";
  json += ",\"contentRole\":\"" + std::string(channelContentRoleName(state.contentRole)) + "\"";
  json += ",\"primaryHarmonyInstanceId\":\"" + escapeJson(state.primaryInstanceId) + "\"";
  json += ",\"desiredEnabled\":" + std::string(state.desiredEnabled ? "true" : "false");
  json += ",\"effectiveEnabled\":" + std::string(state.effectiveEnabled ? "true" : "false");
  json += ",\"pending\":" + std::string(state.pending ? "true" : "false");
  json += ",\"revision\":" + std::to_string(state.revision);
  json += ",\"latencySamples\":" + std::to_string(state.latencySamples);
  json += ",\"key\":\"" + escapeJson(state.params.key) + "\"";
  json += ",\"scale\":\"" + escapeJson(state.params.scale) + "\"";
  json += ",\"mode\":\"" + escapeJson(state.params.mode) + "\"";
  json += ",\"voice1\":\"" + escapeJson(state.params.voice1) + "\"";
  json += ",\"voice2\":\"" + escapeJson(state.params.voice2) + "\"";
  json += ",\"levelDb\":" + std::to_string(state.params.levelDb);
  return json;
}

std::string harmonyAckJson(const HarmonyCommandAck& ack) {
  std::string json = "\"accepted\":" + std::string(ack.accepted ? "true" : "false");
  json += ",\"error\":\"" + std::string(harmonyCommandErrorName(ack.error)) + "\"";
  json += "," + harmonyStateJson(ack.state);
  return json;
}

}  // namespace localmixer::engine::protocol
