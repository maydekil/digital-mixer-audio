#include "engine/PluginRegistry.hpp"

#include <algorithm>

namespace localmixer::engine {
namespace {

std::filesystem::path normalized(std::filesystem::path path) {
  return path.lexically_normal();
}

bool startsWithPath(const std::filesystem::path& path, const std::filesystem::path& root) {
  const auto normalizedPath = normalized(path);
  const auto normalizedRoot = normalized(root);
  auto pathIt = normalizedPath.begin();
  for (auto rootIt = normalizedRoot.begin(); rootIt != normalizedRoot.end(); ++rootIt, ++pathIt) {
    if (pathIt == normalizedPath.end() || *pathIt != *rootIt) return false;
  }
  return true;
}

}  // namespace

bool PluginRegistry::allowRoot(std::filesystem::path root) {
  if (root.empty()) return false;
  allowedRoots_.push_back(normalized(std::move(root)));
  return true;
}

bool PluginRegistry::addScanRecord(PluginScanRecord record) {
  if (!pathAllowed(record.descriptor.path)) {
    record.error = PluginScanError::pathNotAllowed;
    cache_.push_back(std::move(record));
    return false;
  }
  if (isBlacklisted(record.descriptor.identifier)) {
    record.error = PluginScanError::blacklisted;
    cache_.push_back(std::move(record));
    return false;
  }
  const auto extension = record.descriptor.path.extension().string();
  if (extension != ".component" && extension != ".vst3") {
    record.error = PluginScanError::unsupportedFormat;
    cache_.push_back(std::move(record));
    return false;
  }
  record.descriptor.format = pluginFormatForPath(record.descriptor.path);
  cache_.push_back(std::move(record));
  return true;
}

void PluginRegistry::blacklist(std::string identifier) {
  if (identifier.empty() || isBlacklisted(identifier)) return;
  blacklist_.push_back(std::move(identifier));
}

bool PluginRegistry::isBlacklisted(const std::string& identifier) const {
  return std::find(blacklist_.begin(), blacklist_.end(), identifier) != blacklist_.end();
}

std::optional<PluginDescriptor> PluginRegistry::find(const std::string& identifier) const {
  for (const auto& record : cache_) {
    if (record.error == PluginScanError::none && record.descriptor.identifier == identifier) return record.descriptor;
  }
  return std::nullopt;
}

PluginDescriptor PluginRegistry::missingPlaceholder(const std::string& identifier, std::string version) const {
  return PluginDescriptor{
    .identifier = identifier,
    .name = "Missing Plugin",
    .version = std::move(version),
    .missing = true,
  };
}

bool PluginRegistry::pathAllowed(const std::filesystem::path& path) const {
  return std::any_of(allowedRoots_.begin(), allowedRoots_.end(), [&](const auto& root) {
    return startsWithPath(path, root);
  });
}

PluginFormat pluginFormatForPath(const std::filesystem::path& path) {
  return path.extension() == ".component" ? PluginFormat::au : PluginFormat::vst3;
}

const char* pluginFormatName(PluginFormat format) noexcept {
  return format == PluginFormat::au ? "AU" : "VST3";
}

const char* pluginScanErrorName(PluginScanError error) noexcept {
  switch (error) {
    case PluginScanError::none: return "";
    case PluginScanError::pathNotAllowed: return "PATH_NOT_ALLOWED";
    case PluginScanError::unsupportedFormat: return "UNSUPPORTED_FORMAT";
    case PluginScanError::blacklisted: return "PLUGIN_BLACKLISTED";
    case PluginScanError::timeout: return "PLUGIN_SCAN_TIMEOUT";
    case PluginScanError::crashed: return "PLUGIN_SCAN_CRASHED";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::engine
