#pragma once

#include "SoundPadSynth.hpp"

#include <filesystem>

namespace soundpad {

bool writeWav(const RenderedSound& sound, const std::filesystem::path& path);

}  // namespace soundpad
