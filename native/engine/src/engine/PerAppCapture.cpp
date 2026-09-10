#include "engine/PerAppCapture.hpp"

#include <unordered_set>

namespace localmixer::engine {
namespace {

std::string escapeJson(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (const char ch : value) {
    switch (ch) {
      case '\\': escaped += "\\\\"; break;
      case '"': escaped += "\\\""; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default: escaped += ch; break;
    }
  }
  return escaped;
}

const ProcessAudioSource* findSource(
  const std::vector<ProcessAudioSource>& sources,
  const std::string& id
) {
  for (const auto& source : sources) {
    if (source.persistentId == id) return &source;
  }
  return nullptr;
}

}  // namespace

std::string_view perAppCaptureStatusName(PerAppCaptureStatus status) {
  switch (status) {
    case PerAppCaptureStatus::available: return "AVAILABLE";
    case PerAppCaptureStatus::unsupportedPlatform: return "UNSUPPORTED_PLATFORM";
    case PerAppCaptureStatus::permissionRequired: return "PERMISSION_REQUIRED";
    case PerAppCaptureStatus::permissionDenied: return "PERMISSION_DENIED";
    case PerAppCaptureStatus::noSources: return "NO_SOURCES";
    case PerAppCaptureStatus::systemMixConflict: return "SYSTEM_MIX_CONFLICT";
    case PerAppCaptureStatus::sourceNotFound: return "SOURCE_NOT_FOUND";
    case PerAppCaptureStatus::sourceUnavailable: return "SOURCE_UNAVAILABLE";
    case PerAppCaptureStatus::ownProcessRejected: return "OWN_PROCESS_REJECTED";
    case PerAppCaptureStatus::duplicateChannel: return "DUPLICATE_CHANNEL";
    case PerAppCaptureStatus::duplicateSource: return "DUPLICATE_SOURCE";
  }
  return "UNKNOWN";
}

PerAppCaptureAudit validatePerAppCaptureAssignments(
  const PerAppCaptureCapability& capability,
  const std::vector<PerAppCaptureAssignment>& assignments,
  bool systemMixActive
) {
  if (!capability.platformSupported) {
    return {.status = PerAppCaptureStatus::unsupportedPlatform};
  }
  if (capability.permissionDenied) {
    return {.status = PerAppCaptureStatus::permissionDenied};
  }
  if (!capability.permissionGranted) {
    return {.status = PerAppCaptureStatus::permissionRequired};
  }
  if (systemMixActive && !assignments.empty()) {
    return {.status = PerAppCaptureStatus::systemMixConflict};
  }
  if (capability.sources.empty()) {
    return {.status = PerAppCaptureStatus::noSources};
  }

  std::unordered_set<std::string> channels;
  std::unordered_set<std::string> sourceIds;
  PerAppCaptureAudit audit;
  audit.excludesOwnProcess = capability.canExcludeOwnProcess;
  for (const auto& assignment : assignments) {
    if (!channels.insert(assignment.channelId).second) {
      return {.status = PerAppCaptureStatus::duplicateChannel};
    }
    if (!sourceIds.insert(assignment.sourceId).second) {
      return {.status = PerAppCaptureStatus::duplicateSource};
    }
    const auto* source = findSource(capability.sources, assignment.sourceId);
    if (source == nullptr) {
      return {.status = PerAppCaptureStatus::sourceNotFound};
    }
    if (!source->running || !source->capturable) {
      return {.status = PerAppCaptureStatus::sourceUnavailable};
    }
    if (source->ownProcess || (!capability.canExcludeOwnProcess && source->bundleId == "audio.local-mixer")) {
      return {.status = PerAppCaptureStatus::ownProcessRejected};
    }
    audit.assignedCount += 1;
    if (assignment.muteOriginal) audit.mutedOriginalCount += 1;
  }
  return audit;
}

std::string perAppCaptureCapabilityJson(const PerAppCaptureCapability& capability) {
  std::string json = "\"platformSupported\":" + std::string(capability.platformSupported ? "true" : "false");
  json += ",\"permissionGranted\":" + std::string(capability.permissionGranted ? "true" : "false");
  json += ",\"permissionDenied\":" + std::string(capability.permissionDenied ? "true" : "false");
  json += ",\"canMuteOriginal\":" + std::string(capability.canMuteOriginal ? "true" : "false");
  json += ",\"canExcludeOwnProcess\":" + std::string(capability.canExcludeOwnProcess ? "true" : "false");
  json += ",\"clockBridgeRequired\":" + std::string(capability.clockBridgeRequired ? "true" : "false");
  json += ",\"backend\":\"" + escapeJson(capability.backend) + "\"";
  json += ",\"note\":\"" + escapeJson(capability.note) + "\"";
  json += ",\"sources\":[";
  for (std::size_t index = 0; index < capability.sources.size(); index += 1) {
    const auto& source = capability.sources[index];
    if (index > 0) json += ",";
    json += "{\"persistentId\":\"" + escapeJson(source.persistentId) + "\"";
    json += ",\"pid\":" + std::to_string(source.pid);
    json += ",\"name\":\"" + escapeJson(source.name) + "\"";
    json += ",\"bundleId\":\"" + escapeJson(source.bundleId) + "\"";
    json += ",\"running\":" + std::string(source.running ? "true" : "false");
    json += ",\"capturable\":" + std::string(source.capturable ? "true" : "false");
    json += ",\"ownProcess\":" + std::string(source.ownProcess ? "true" : "false");
    json += ",\"sampleRate\":" + std::to_string(source.sampleRate);
    json += "}";
  }
  json += "]";
  return json;
}

}  // namespace localmixer::engine
