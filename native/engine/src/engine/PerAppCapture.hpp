#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace localmixer::engine {

enum class PerAppCaptureStatus {
  available,
  unsupportedPlatform,
  permissionRequired,
  permissionDenied,
  noSources,
  systemMixConflict,
  sourceNotFound,
  sourceUnavailable,
  ownProcessRejected,
  duplicateChannel,
  duplicateSource,
};

struct ProcessAudioSource {
  std::string persistentId;
  std::uint32_t pid = 0;
  std::string name;
  std::string bundleId;
  bool running = true;
  bool capturable = true;
  bool ownProcess = false;
  double sampleRate = 48000.0;
};

struct PerAppCaptureCapability {
  bool platformSupported = false;
  bool permissionGranted = false;
  bool permissionDenied = false;
  bool canMuteOriginal = false;
  bool canExcludeOwnProcess = false;
  bool clockBridgeRequired = true;
  std::string backend;
  std::string note;
  std::vector<ProcessAudioSource> sources;
};

struct PerAppCaptureAssignment {
  std::string channelId;
  std::string sourceId;
  bool muteOriginal = true;
};

struct PerAppCaptureAudit {
  PerAppCaptureStatus status = PerAppCaptureStatus::available;
  std::uint32_t assignedCount = 0;
  std::uint32_t mutedOriginalCount = 0;
  bool excludesOwnProcess = false;
};

std::string_view perAppCaptureStatusName(PerAppCaptureStatus status);
PerAppCaptureAudit validatePerAppCaptureAssignments(
  const PerAppCaptureCapability& capability,
  const std::vector<PerAppCaptureAssignment>& assignments,
  bool systemMixActive
);
std::string perAppCaptureCapabilityJson(const PerAppCaptureCapability& capability);

}  // namespace localmixer::engine
