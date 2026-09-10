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
