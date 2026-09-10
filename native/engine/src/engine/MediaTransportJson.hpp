#pragma once

#include "engine/MediaImportJob.hpp"
#include "engine/Transport.hpp"

#include <string>

namespace localmixer::engine::protocol {

std::string mediaInfoJson(const std::string& path);
std::string mediaImportStatusJson(const localmixer::engine::MediaImportStatus& status);
std::string transportJson(const localmixer::engine::TransportSnapshot& snapshot);

}  // namespace localmixer::engine::protocol
