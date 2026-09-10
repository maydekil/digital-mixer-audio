#pragma once

#include <string>

namespace localmixer::platform::macos {

struct SystemRouteApplyResult {
  bool ok = false;
  std::string error;
  std::string currentOutputUid;
};

std::string currentDefaultOutputUid();
SystemRouteApplyResult setDefaultOutputUid(const std::string& uid);

}  // namespace localmixer::platform::macos
