#include "engine/ChannelHarmonyController.hpp"

#include <algorithm>
#include <utility>

namespace localmixer::engine {
namespace {

constexpr std::uint32_t kMaxCreativeRackSlots = 8;
constexpr std::uint32_t kPrimaryHarmonyLatencySamples = 4416;

HarmonyCommandAck reject(HarmonyCommandError error, ChannelHarmonyState state) {
  return {.accepted = false, .error = error, .state = std::move(state)};
}

}  // namespace

void ChannelHarmonyController::registerChannel(
  std::string channelId,
  ChannelContentRole role,
  std::vector<std::string> existingHarmonyInstanceIds,
  std::uint32_t rackSlotCount
) {
  ChannelHarmonyState state{
    .channelId = std::move(channelId),
    .contentRole = role,
    .rackSlotCount = rackSlotCount,
    .harmonyCandidateCount = static_cast<std::uint32_t>(existingHarmonyInstanceIds.size()),
  };
  if (existingHarmonyInstanceIds.size() == 1) state.primaryInstanceId = existingHarmonyInstanceIds.front();

  const auto existing = find(state.channelId);
  if (existing) {
    state.revision = existing->revision;
    *existing = std::move(state);
    return;
  }
  channels_.push_back(std::move(state));
}

HarmonyCommandAck ChannelHarmonyController::setEnabled(const std::string& channelId, bool enabled, std::uint64_t expectedRevision) {
  auto* state = find(channelId);
  if (!state) return reject(HarmonyCommandError::channelNotFound, ChannelHarmonyState{.channelId = channelId});
  if (state->revision != expectedRevision) return reject(HarmonyCommandError::revisionConflict, *state);
  if (state->contentRole != ChannelContentRole::vocal) return reject(HarmonyCommandError::roleUnsupported, *state);

  if (enabled && state->primaryInstanceId.empty()) {
    if (state->harmonyCandidateCount > 1) return reject(HarmonyCommandError::multipleCandidates, *state);
    if (state->rackSlotCount >= kMaxCreativeRackSlots) return reject(HarmonyCommandError::rackFull, *state);
    state->primaryInstanceId = "harmony:" + state->channelId + ":primary";
    state->rackSlotCount += 1;
    state->harmonyCandidateCount = 1;
  }

  state->desiredEnabled = enabled;
  state->effectiveEnabled = enabled;
  state->pending = false;
  state->latencySamples = kPrimaryHarmonyLatencySamples;
  state->revision += 1;
  return {.accepted = true, .state = *state};
}

HarmonyCommandAck ChannelHarmonyController::configure(const std::string& channelId, HarmonyQuickParams params, std::uint64_t expectedRevision) {
  auto* state = find(channelId);
  if (!state) return reject(HarmonyCommandError::channelNotFound, ChannelHarmonyState{.channelId = channelId});
  if (state->revision != expectedRevision) return reject(HarmonyCommandError::revisionConflict, *state);
  if (state->contentRole != ChannelContentRole::vocal) return reject(HarmonyCommandError::roleUnsupported, *state);
  state->params = std::move(params);
  state->revision += 1;
  return {.accepted = true, .state = *state};
}

std::optional<ChannelHarmonyState> ChannelHarmonyController::snapshot(const std::string& channelId) const {
  const auto* state = find(channelId);
  if (!state) return std::nullopt;
  return *state;
}

ChannelHarmonyState* ChannelHarmonyController::find(const std::string& channelId) noexcept {
  const auto it = std::find_if(channels_.begin(), channels_.end(), [&](const auto& state) {
    return state.channelId == channelId;
  });
  return it == channels_.end() ? nullptr : &*it;
}

const ChannelHarmonyState* ChannelHarmonyController::find(const std::string& channelId) const noexcept {
  const auto it = std::find_if(channels_.begin(), channels_.end(), [&](const auto& state) {
    return state.channelId == channelId;
  });
  return it == channels_.end() ? nullptr : &*it;
}

const char* channelContentRoleName(ChannelContentRole role) noexcept {
  switch (role) {
    case ChannelContentRole::vocal: return "vocal";
    case ChannelContentRole::instrument: return "instrument";
    case ChannelContentRole::music: return "music";
    case ChannelContentRole::other: return "other";
  }
  return "other";
}

std::optional<ChannelContentRole> channelContentRoleFromName(const std::string& name) noexcept {
  if (name == "vocal") return ChannelContentRole::vocal;
  if (name == "instrument") return ChannelContentRole::instrument;
  if (name == "music") return ChannelContentRole::music;
  if (name == "other") return ChannelContentRole::other;
  return std::nullopt;
}

const char* harmonyCommandErrorName(HarmonyCommandError error) noexcept {
  switch (error) {
    case HarmonyCommandError::none: return "";
    case HarmonyCommandError::channelNotFound: return "CHANNEL_NOT_FOUND";
    case HarmonyCommandError::roleUnsupported: return "ROLE_UNSUPPORTED";
    case HarmonyCommandError::rackFull: return "FX_RACK_FULL";
    case HarmonyCommandError::multipleCandidates: return "MULTIPLE_HARMONY_CANDIDATES";
    case HarmonyCommandError::revisionConflict: return "REVISION_CONFLICT";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::engine
