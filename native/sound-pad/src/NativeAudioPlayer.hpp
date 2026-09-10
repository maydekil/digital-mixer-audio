#pragma once

#include <filesystem>

namespace soundpad {

bool playFileBlocking(const std::filesystem::path& path);
const char* lastPlaybackError();

}  // namespace soundpad
