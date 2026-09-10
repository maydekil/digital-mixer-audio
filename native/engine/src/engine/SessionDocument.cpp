#include "engine/SessionDocument.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <system_error>

namespace localmixer::engine {
namespace {

std::string escapeJson(std::string_view value) {
  std::string escaped;
  for (const auto ch : value) {
    if (ch == '"' || ch == '\\') escaped += '\\';
    escaped += ch;
  }
  return escaped;
}

std::optional<std::string> readFile(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file) return std::nullopt;
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool writeFile(const std::filesystem::path& path, std::string_view content) {
  std::ofstream file(path, std::ios::trunc);
  if (!file) return false;
  file << content;
  return static_cast<bool>(file);
}

std::optional<std::string> matchString(std::string_view json, const std::string& key) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
  std::cmatch match;
  if (!std::regex_search(json.data(), json.data() + json.size(), match, pattern)) return std::nullopt;
  return match[1].str();
}

std::optional<std::uint32_t> matchU32(std::string_view json, const std::string& key) {
  const std::regex pattern("\"" + key + "\"\\s*:\\s*(\\d+)");
  std::cmatch match;
  if (!std::regex_search(json.data(), json.data() + json.size(), match, pattern)) return std::nullopt;
  return static_cast<std::uint32_t>(std::stoul(match[1].str()));
}

}  // namespace

std::string serializeSession(const SessionDocument& document) {
  std::ostringstream json;
  json << "{\n  \"schemaVersion\": " << document.schemaVersion << ",\n";
  json << "  \"projectId\": \"" << escapeJson(document.projectId) << "\",\n";
  json << "  \"media\": [";
  for (std::size_t index = 0; index < document.media.size(); index += 1) {
    const auto& media = document.media[index];
    json << (index == 0 ? "\n" : ",\n");
    json << "    {\"id\": \"" << escapeJson(media.id) << "\", \"path\": \""
         << escapeJson(media.path.string()) << "\", \"missing\": " << (media.missing ? "true" : "false") << "}";
  }
  if (!document.media.empty()) json << "\n  ";
  json << "],\n";
  json << "  \"fxUnits\": [";
  for (std::size_t index = 0; index < document.fxUnits.size(); index += 1) {
    const auto& unit = document.fxUnits[index];
    json << (index == 0 ? "\n" : ",\n");
    json << "    {\"unitId\": \"" << escapeJson(unit.unitId) << "\", \"programId\": " << unit.programId
         << ", \"processorType\": \"" << escapeJson(unit.processorType) << "\", \"revision\": " << unit.revision
         << ", \"enabled\": " << (unit.enabled ? "true" : "false")
         << ", \"modified\": " << (unit.modified ? "true" : "false")
         << ", \"returnDb\": " << unit.returnDb
         << ", \"macro1Value\": \"" << escapeJson(unit.macro1Value)
         << "\", \"macro2Value\": \"" << escapeJson(unit.macro2Value) << "\"}";
  }
  if (!document.fxUnits.empty()) json << "\n  ";
  json << "],\n";
  json << "  \"fxSends\": [";
  for (std::size_t index = 0; index < document.fxSends.size(); index += 1) {
    const auto& send = document.fxSends[index];
    json << (index == 0 ? "\n" : ",\n");
    json << "    {\"channelId\": \"" << escapeJson(send.channelId) << "\", \"unitId\": \"" << escapeJson(send.unitId)
         << "\", \"enabled\": " << (send.enabled ? "true" : "false")
         << ", \"gainDb\": " << send.gainDb << "}";
  }
  if (!document.fxSends.empty()) json << "\n  ";
  json << "]\n}\n";
  return json.str();
}

SessionLoadResult parseSession(std::string_view json) {
  auto schemaVersion = matchU32(json, "schemaVersion");
  auto projectId = matchString(json, "projectId");
  if (!schemaVersion.has_value() || !projectId.has_value() || projectId->empty()) {
    return {.error = SessionError::invalidDocument};
  }
  if (*schemaVersion > kCurrentSessionSchemaVersion) return {.error = SessionError::newerSchema};

  SessionDocument document{.schemaVersion = *schemaVersion, .projectId = *projectId};
  const std::regex mediaPattern("\\{\"id\"\\s*:\\s*\"([^\"]*)\",\\s*\"path\"\\s*:\\s*\"([^\"]*)\",\\s*\"missing\"\\s*:\\s*(true|false)\\}");
  for (auto it = std::cregex_iterator(json.data(), json.data() + json.size(), mediaPattern);
       it != std::cregex_iterator();
       ++it) {
    document.media.push_back(SessionMediaRef{
      .id = (*it)[1].str(),
      .path = (*it)[2].str(),
      .missing = (*it)[3].str() == "true",
    });
  }

  const std::regex fxUnitPattern("\\{\"unitId\"\\s*:\\s*\"([^\"]*)\",\\s*\"programId\"\\s*:\\s*(\\d+),\\s*\"processorType\"\\s*:\\s*\"([^\"]*)\",\\s*\"revision\"\\s*:\\s*(\\d+),\\s*\"enabled\"\\s*:\\s*(true|false),\\s*\"modified\"\\s*:\\s*(true|false),\\s*\"returnDb\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?),\\s*\"macro1Value\"\\s*:\\s*\"([^\"]*)\",\\s*\"macro2Value\"\\s*:\\s*\"([^\"]*)\"\\}");
  for (auto it = std::cregex_iterator(json.data(), json.data() + json.size(), fxUnitPattern);
       it != std::cregex_iterator();
       ++it) {
    document.fxUnits.push_back(SessionFxUnitState{
      .unitId = (*it)[1].str(),
      .programId = static_cast<std::uint32_t>(std::stoul((*it)[2].str())),
      .processorType = (*it)[3].str(),
      .revision = static_cast<std::uint64_t>(std::stoull((*it)[4].str())),
      .enabled = (*it)[5].str() == "true",
      .modified = (*it)[6].str() == "true",
      .returnDb = std::stof((*it)[7].str()),
      .macro1Value = (*it)[8].str(),
      .macro2Value = (*it)[9].str(),
    });
  }

  const std::regex sendPattern("\\{\"channelId\"\\s*:\\s*\"([^\"]*)\",\\s*\"unitId\"\\s*:\\s*\"([^\"]*)\",\\s*\"enabled\"\\s*:\\s*(true|false),\\s*\"gainDb\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)\\}");
  for (auto it = std::cregex_iterator(json.data(), json.data() + json.size(), sendPattern);
       it != std::cregex_iterator();
       ++it) {
    document.fxSends.push_back(SessionFxSendAssignment{
      .channelId = (*it)[1].str(),
      .unitId = (*it)[2].str(),
      .enabled = (*it)[3].str() == "true",
      .gainDb = std::stof((*it)[4].str()),
    });
  }
  return {.document = document};
}

bool saveSessionAtomic(const std::filesystem::path& path, const SessionDocument& document) {
  if (document.schemaVersion != kCurrentSessionSchemaVersion || document.projectId.empty()) return false;
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  if (error) return false;

  const auto tempPath = path.string() + ".tmp";
  const auto backupPath = path.string() + ".bak";
  if (!writeFile(tempPath, serializeSession(document))) return false;
  if (std::filesystem::exists(path, error)) {
    std::filesystem::copy_file(path, backupPath, std::filesystem::copy_options::overwrite_existing, error);
    if (error) return false;
  }
  std::filesystem::rename(tempPath, path, error);
  return !error;
}

SessionLoadResult loadSession(const std::filesystem::path& path) {
  const auto content = readFile(path);
  if (!content.has_value()) return {.error = SessionError::ioError};
  return parseSession(*content);
}

std::filesystem::path autosavePathFor(const std::filesystem::path& sessionPath) {
  auto autosavePath = sessionPath;
  autosavePath += ".autosave";
  return autosavePath;
}

std::optional<std::filesystem::path> recoveryCandidateFor(const std::filesystem::path& sessionPath) {
  const auto autosavePath = autosavePathFor(sessionPath);
  std::error_code error;
  if (std::filesystem::exists(autosavePath, error) && std::filesystem::is_regular_file(autosavePath, error)) {
    return autosavePath;
  }
  return std::nullopt;
}

}  // namespace localmixer::engine
