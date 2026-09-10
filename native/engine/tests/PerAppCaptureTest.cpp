#include "engine/PerAppCapture.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

using localmixer::engine::PerAppCaptureAssignment;
using localmixer::engine::PerAppCaptureCapability;
using localmixer::engine::PerAppCaptureStatus;
using localmixer::engine::ProcessAudioSource;
using localmixer::engine::perAppCaptureCapabilityJson;
using localmixer::engine::validatePerAppCaptureAssignments;

PerAppCaptureCapability capability() {
  return PerAppCaptureCapability{
    .platformSupported = true,
    .permissionGranted = true,
    .canMuteOriginal = true,
    .canExcludeOwnProcess = true,
    .backend = "CoreAudioTap",
    .note = "test fixture",
    .sources = {
      ProcessAudioSource{
        .persistentId = "proc:music",
        .pid = 101,
        .name = "Music",
        .bundleId = "com.example.music",
      },
      ProcessAudioSource{
        .persistentId = "proc:browser",
        .pid = 202,
        .name = "Browser",
        .bundleId = "com.example.browser",
      },
      ProcessAudioSource{
        .persistentId = "proc:local-mixer",
        .pid = 303,
        .name = "Local Audio Mixer",
        .bundleId = "audio.local-mixer",
        .ownProcess = true,
      },
    },
  };
}

}  // namespace

int main() {
  auto cap = capability();
  const auto ok = validatePerAppCaptureAssignments(
    cap,
    {
      PerAppCaptureAssignment{.channelId = "music", .sourceId = "proc:music", .muteOriginal = true},
      PerAppCaptureAssignment{.channelId = "browser", .sourceId = "proc:browser", .muteOriginal = false},
    },
    false
  );
  if (ok.status != PerAppCaptureStatus::available || ok.assignedCount != 2 || ok.mutedOriginalCount != 1 ||
      !ok.excludesOwnProcess) {
    std::cerr << "two independent app assignments should validate\n";
    return 1;
  }

  const auto conflict = validatePerAppCaptureAssignments(
    cap,
    {PerAppCaptureAssignment{.channelId = "music", .sourceId = "proc:music"}},
    true
  );
  if (conflict.status != PerAppCaptureStatus::systemMixConflict) {
    std::cerr << "per-app and system mix capture should be exclusive\n";
    return 1;
  }

  const auto ownProcess = validatePerAppCaptureAssignments(
    cap,
    {PerAppCaptureAssignment{.channelId = "self", .sourceId = "proc:local-mixer"}},
    false
  );
  if (ownProcess.status != PerAppCaptureStatus::ownProcessRejected) {
    std::cerr << "own process capture should be rejected\n";
    return 1;
  }

  cap.permissionGranted = false;
  const auto permission = validatePerAppCaptureAssignments(
    cap,
    {PerAppCaptureAssignment{.channelId = "music", .sourceId = "proc:music"}},
    false
  );
  if (permission.status != PerAppCaptureStatus::permissionRequired) {
    std::cerr << "missing tap permission should require explicit permission flow\n";
    return 1;
  }

  const auto json = perAppCaptureCapabilityJson(cap);
  if (json.find("\"backend\":\"CoreAudioTap\"") == std::string::npos ||
      json.find("\"persistentId\":\"proc:music\"") == std::string::npos) {
    std::cerr << "capability JSON should include backend and process source identity\n";
    return 1;
  }

  std::cout << "per-app capture ok\n";
  return 0;
}
