#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace localmixer::platform::macos {

struct AudioDeviceInfo {
  std::uint32_t id = 0;
  std::string uid;
  std::string name;
  bool isDefaultInput = false;
  bool isDefaultOutput = false;
  std::uint32_t inputChannels = 0;
  std::uint32_t outputChannels = 0;
  double nominalSampleRate = 0.0;
};

std::vector<AudioDeviceInfo> listAudioDevices();
std::string microphonePermissionState();
std::string requestMicrophonePermission();

}  // namespace localmixer::platform::macos
