#include "engine/SystemRoutingJson.hpp"

#include "engine/JsonProtocol.hpp"

namespace localmixer::engine::protocol {

std::string systemRouteDiagnosticsJson(
  std::span<const DeviceDescriptor> devices,
  const std::string& blackHoleUid,
  const std::string& outputUid,
  double sampleRate,
  std::uint32_t inputStartChannel,
  std::uint32_t outputStartChannel
) {
  const auto diagnostics = validateSystemRoute(
    devices,
    ProjectAudioConfig{.sampleRate = sampleRate, .blockSize = 256},
    SystemRouteSelection{
      .blackHoleUid = blackHoleUid,
      .physicalOutputUid = outputUid,
      .blackHoleInputStartChannel = inputStartChannel,
      .physicalOutputStartChannel = outputStartChannel,
    }
  );
  std::string json = "\"routeValid\":" + std::string(diagnostics.routeValid ? "true" : "false");
  json += ",\"blackHoleAvailable\":" + std::string(diagnostics.blackHoleAvailable ? "true" : "false");
  json += ",\"error\":\"" + std::string(systemRouteErrorName(diagnostics.error)) + "\"";
  json += ",\"blackHoleUid\":\"" + escapeJson(diagnostics.selection.blackHoleUid) + "\"";
  json += ",\"physicalOutputUid\":\"" + escapeJson(diagnostics.selection.physicalOutputUid) + "\"";
  json += ",\"selectedInputStart\":" + std::to_string(diagnostics.selectedInputStart);
  json += ",\"selectedInputEnd\":" + std::to_string(diagnostics.selectedInputEnd);
  json += ",\"selectedOutputStart\":" + std::to_string(diagnostics.selectedOutputStart);
  json += ",\"selectedOutputEnd\":" + std::to_string(diagnostics.selectedOutputEnd);
  json += ",\"blackHoleSampleRate\":" + std::to_string(diagnostics.blackHoleSampleRate);
  json += ",\"outputSampleRate\":" + std::to_string(diagnostics.outputSampleRate);
  return json;
}

std::string systemRouteTransactionJson(const SystemRouteTransaction& transaction) {
  std::string json = "\"state\":\"";
  json += systemRouteTransactionStateName(transaction.state);
  json += "\",\"error\":\"" + std::string(systemRouteErrorName(transaction.error)) + "\"";
  json += ",\"ownsSystemRoute\":" + std::string(transaction.ownsSystemRoute ? "true" : "false");
  json += ",\"originalOutputUid\":\"" + escapeJson(transaction.originalOutputUid) + "\"";
  json += ",\"blackHoleUid\":\"" + escapeJson(transaction.selection.blackHoleUid) + "\"";
  json += ",\"physicalOutputUid\":\"" + escapeJson(transaction.selection.physicalOutputUid) + "\"";
  return json;
}

}  // namespace localmixer::engine::protocol
