#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class ChannelContentRole {
  vocal,
  instrument,
  music,
  other,
};

enum class HarmonyCommandError {
  none,
  channelNotFound,
  roleUnsupported,
  rackFull,
  multipleCandidates,
  revisionConflict,
};

struct HarmonyQuickParams {
  std::string key = "C";
  std::string scale = "Major";
  std::string mode = "Diatonic";
  std::string voice1 = "+3rd";
  std::string voice2 = "+5th";
  float levelDb = 0.0f;
};

struct ChannelHarmonyState {
  std::string channelId;
  ChannelContentRole contentRole = ChannelContentRole::other;
  std::string primaryInstanceId;
  HarmonyQuickParams params;
  bool desiredEnabled = false;
  bool effectiveEnabled = false;
  bool pending = false;
  std::uint64_t revision = 0;
  std::uint32_t latencySamples = 0;
  std::uint32_t rackSlotCount = 0;
  std::uint32_t harmonyCandidateCount = 0;
};

struct HarmonyCommandAck {
  bool accepted = false;
  HarmonyCommandError error = HarmonyCommandError::none;
  ChannelHarmonyState state;
};

class ChannelHarmonyController {
 public:
  void registerChannel(
    std::string channelId,
    ChannelContentRole role,
    std::vector<std::string> existingHarmonyInstanceIds = {},
    std::uint32_t rackSlotCount = 0
  );

  HarmonyCommandAck setEnabled(const std::string& channelId, bool enabled, std::uint64_t expectedRevision);
  HarmonyCommandAck configure(const std::string& channelId, HarmonyQuickParams params, std::uint64_t expectedRevision);
  std::optional<ChannelHarmonyState> snapshot(const std::string& channelId) const;

 private:
  ChannelHarmonyState* find(const std::string& channelId) noexcept;
  const ChannelHarmonyState* find(const std::string& channelId) const noexcept;

  std::vector<ChannelHarmonyState> channels_;
};

const char* channelContentRoleName(ChannelContentRole role) noexcept;
std::optional<ChannelContentRole> channelContentRoleFromName(const std::string& name) noexcept;
const char* harmonyCommandErrorName(HarmonyCommandError error) noexcept;

}  // namespace localmixer::engine
