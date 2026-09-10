#pragma once

#include "engine/SystemRouting.hpp"

#include <filesystem>
#include <optional>

namespace localmixer::engine {

struct SystemRouteRecoveryMarker {
  std::string originalOutputUid;
  SystemRouteSelection selection;
};

class SystemRouteRecoveryStore {
 public:
  explicit SystemRouteRecoveryStore(std::filesystem::path path);

  bool save(const SystemRouteTransaction& transaction) const;
  std::optional<SystemRouteRecoveryMarker> load() const;
  bool clear() const;
  const std::filesystem::path& path() const;

 private:
  std::filesystem::path path_;
};

}  // namespace localmixer::engine
