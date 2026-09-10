#include "platform/macos/CoreAudioDevices.hpp"

#import <AVFoundation/AVFoundation.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>

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

std::uint32_t countChannels(AudioObjectID device, AudioObjectPropertyScope scope) {
  auto address = property(kAudioDevicePropertyStreamConfiguration, scope);
  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(device, &address, 0, nullptr, &size) != noErr || size == 0) return 0;

  std::vector<std::byte> storage(size);
  auto* list = reinterpret_cast<AudioBufferList*>(storage.data());
  if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, list) != noErr) return 0;

  std::uint32_t channels = 0;
  for (UInt32 index = 0; index < list->mNumberBuffers; index += 1) channels += list->mBuffers[index].mNumberChannels;
  return channels;
}

AudioObjectID defaultDevice(AudioObjectPropertySelector selector) {
  AudioObjectID device = kAudioObjectUnknown;
  readProperty(kAudioObjectSystemObject, selector, kAudioObjectPropertyScopeGlobal, device);
  return device;
}

}  // namespace

std::vector<AudioDeviceInfo> listAudioDevices() {
  auto address = property(kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal);
  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &size) != noErr || size == 0) return {};

  std::vector<AudioObjectID> ids(size / sizeof(AudioObjectID));
  if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, ids.data()) != noErr) return {};

  const auto defaultInput = defaultDevice(kAudioHardwarePropertyDefaultInputDevice);
  const auto defaultOutput = defaultDevice(kAudioHardwarePropertyDefaultOutputDevice);
  std::vector<AudioDeviceInfo> devices;
  devices.reserve(ids.size());

  for (const auto id : ids) {
    AudioDeviceInfo info;
    info.id = id;
    info.uid = readString(id, kAudioDevicePropertyDeviceUID);
    info.name = readString(id, kAudioObjectPropertyName);
    info.isDefaultInput = id == defaultInput;
    info.isDefaultOutput = id == defaultOutput;
    info.inputChannels = countChannels(id, kAudioDevicePropertyScopeInput);
    info.outputChannels = countChannels(id, kAudioDevicePropertyScopeOutput);
    Float64 rate = 0.0;
    if (readProperty(id, kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, rate)) info.nominalSampleRate = rate;
    devices.push_back(info);
  }

  return devices;
}

std::string microphonePermissionState() {
  const AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
  switch (status) {
    case AVAuthorizationStatusAuthorized: return "AUTHORIZED";
    case AVAuthorizationStatusDenied: return "DENIED";
    case AVAuthorizationStatusRestricted: return "RESTRICTED";
    case AVAuthorizationStatusNotDetermined: return "NOT_DETERMINED";
  }
  return "UNAVAILABLE";
}

std::string requestMicrophonePermission() {
  if ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio] != AVAuthorizationStatusNotDetermined) {
    return microphonePermissionState();
  }

  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(__unused BOOL granted) {
    dispatch_semaphore_signal(semaphore);
  }];
  dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  return microphonePermissionState();
}

}  // namespace localmixer::platform::macos
