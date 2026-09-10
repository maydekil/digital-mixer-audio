#pragma once

#include <string>

namespace localmixer::engine {

std::string exportPlanJsonFields(const std::string& line);
std::string exportRenderJsonFields(const std::string& line);

}  // namespace localmixer::engine
