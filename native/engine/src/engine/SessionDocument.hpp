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

struct SessionDocument {
  std::uint32_t schemaVersion = kCurrentSessionSchemaVersion;
  std::string projectId;
  std::vector<SessionMediaRef> media;
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
