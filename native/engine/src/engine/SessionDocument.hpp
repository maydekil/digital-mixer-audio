#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

constexpr std::uint32_t kCurrentSessionSchemaVersion = 1;

struct SessionMediaRef {
  std::string id;
  std::filesystem::path path;
  bool missing = false;
};

struct SessionFxUnitState {
  std::string unitId;
  std::uint32_t programId = 0;
  std::string processorType;
  std::uint64_t revision = 0;
  bool enabled = false;
  bool modified = false;
  float returnDb = -12.0f;
  std::string macro1Value;
  std::string macro2Value;
};

struct SessionFxSendAssignment {
  std::string channelId;
  std::string unitId;
  bool enabled = false;
  float gainDb = -90.0f;
};

struct SessionDocument {
  std::uint32_t schemaVersion = kCurrentSessionSchemaVersion;
  std::string projectId;
  std::vector<SessionMediaRef> media;
  std::vector<SessionFxUnitState> fxUnits;
  std::vector<SessionFxSendAssignment> fxSends;
};

enum class SessionError {
  none,
  invalidDocument,
  newerSchema,
  ioError,
};

struct SessionLoadResult {
  SessionError error = SessionError::none;
  SessionDocument document;
};

std::string serializeSession(const SessionDocument& document);
SessionLoadResult parseSession(std::string_view json);
bool saveSessionAtomic(const std::filesystem::path& path, const SessionDocument& document);
SessionLoadResult loadSession(const std::filesystem::path& path);
std::filesystem::path autosavePathFor(const std::filesystem::path& sessionPath);
std::optional<std::filesystem::path> recoveryCandidateFor(const std::filesystem::path& sessionPath);

}  // namespace localmixer::engine
