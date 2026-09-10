#pragma once

#include <string>
#include <vector>

namespace soundpad {

struct RenderedSound {
  double sampleRate = 48000.0;
  int channels = 2;
  std::vector<float> interleaved;
};

bool isSupportedPad(const std::string& id);
std::vector<std::string> supportedPads();
RenderedSound renderPad(const std::string& id);

}  // namespace soundpad
