#pragma once

#include "engine/Transport.hpp"

#include <string>

namespace localmixer::engine::protocol {

std::string mediaInfoJson(const std::string& path);
std::string transportJson(const localmixer::engine::TransportSnapshot& snapshot);

}  // namespace localmixer::engine::protocol
