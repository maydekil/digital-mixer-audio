#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class PluginFormat {
  au,
  vst3,
};

enum class PluginScanError {
  none,
  pathNotAllowed,
  unsupportedFormat,
  blacklisted,
  timeout,
  crashed,
};

struct PluginDescriptor {
  std::string identifier;
  std::string name;
  std::string version;
  PluginFormat format = PluginFormat::vst3;
  std::filesystem::path path;
  std::uint32_t inputChannels = 2;
  std::uint32_t outputChannels = 2;
  std::uint32_t latencySamples = 0;
  bool missing = false;
};

struct PluginScanRecord {
  PluginDescriptor descriptor;
  PluginScanError error = PluginScanError::none;
};

class PluginRegistry {
 public:
  bool allowRoot(std::filesystem::path root);
  bool addScanRecord(PluginScanRecord record);
  void blacklist(std::string identifier);
  bool isBlacklisted(const std::string& identifier) const;
  std::optional<PluginDescriptor> find(const std::string& identifier) const;
  PluginDescriptor missingPlaceholder(const std::string& identifier, std::string version) const;
  const std::vector<PluginScanRecord>& cache() const noexcept { return cache_; }

 private:
  bool pathAllowed(const std::filesystem::path& path) const;

  std::vector<std::filesystem::path> allowedRoots_;
  std::vector<std::string> blacklist_;
  std::vector<PluginScanRecord> cache_;
};

PluginFormat pluginFormatForPath(const std::filesystem::path& path);
const char* pluginFormatName(PluginFormat format) noexcept;
const char* pluginScanErrorName(PluginScanError error) noexcept;

}  // namespace localmixer::engine
