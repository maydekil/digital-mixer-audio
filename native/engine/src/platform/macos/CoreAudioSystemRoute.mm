#include "platform/macos/CoreAudioSystemRoute.hpp"

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <vector>

namespace localmixer::platform::macos {
namespace {

AudioObjectPropertyAddress property(AudioObjectPropertySelector selector, AudioObjectPropertyScope scope) {
  return {selector, scope, kAudioObjectPropertyElementMain};
}

template <typename T>
bool readProperty(AudioObjectID object, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope, T& value) {
  auto address = property(selector, scope);
  UInt32 size = sizeof(T);
  return AudioObjectGetPropertyData(object, &address, 0, nullptr, &size, &value) == noErr;
}

std::string readString(AudioObjectID object, AudioObjectPropertySelector selector) {
  CFStringRef value = nullptr;
  if (!readProperty(object, selector, kAudioObjectPropertyScopeGlobal, value) || value == nullptr) return "";

  char buffer[512] = {};
  const bool ok = CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8);
  CFRelease(value);
  return ok ? std::string(buffer) : "";
}

AudioObjectID deviceByUid(const std::string& uid) {
  if (uid.empty()) return kAudioObjectUnknown;
  auto address = property(kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal);
  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &size) != noErr || size == 0) {
    return kAudioObjectUnknown;
  }

  std::vector<AudioObjectID> ids(size / sizeof(AudioObjectID));
  if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, ids.data()) != noErr) {
    return kAudioObjectUnknown;
  }

  for (const auto device : ids) {
    if (readString(device, kAudioDevicePropertyDeviceUID) == uid) return device;
  }
  return kAudioObjectUnknown;
}

}  // namespace

std::string currentDefaultOutputUid() {
  AudioObjectID device = kAudioObjectUnknown;
  if (!readProperty(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, device) ||
      device == kAudioObjectUnknown) {
    return "";
  }
  return readString(device, kAudioDevicePropertyDeviceUID);
}

SystemRouteApplyResult setDefaultOutputUid(const std::string& uid) {
  const auto device = deviceByUid(uid);
  if (device == kAudioObjectUnknown) return {.error = "OUTPUT_NOT_FOUND", .currentOutputUid = currentDefaultOutputUid()};

  auto address = property(kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal);
  AudioObjectID next = device;
  const auto status = AudioObjectSetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, sizeof(next), &next);
  if (status != noErr) {
    return {.error = "OS_ROUTE_APPLY_FAILED", .currentOutputUid = currentDefaultOutputUid()};
  }
  return {.ok = true, .currentOutputUid = currentDefaultOutputUid()};
}

}  // namespace localmixer::platform::macos
