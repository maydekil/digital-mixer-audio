#include "platform/macos/CoreAudioInputMeter.hpp"

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <vector>

namespace localmixer::platform::macos {
namespace {

constexpr double kRateTolerance = 0.01;

struct MeterState {
  std::atomic<int> peakScaled{0};
};

AudioObjectPropertyAddress property(AudioObjectPropertySelector selector, AudioObjectPropertyScope scope) {
  return {selector, scope, kAudioObjectPropertyElementMain};
}

template <typename T>
bool readProperty(AudioObjectID object, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope, T& value) {
  auto address = property(selector, scope);
  UInt32 size = sizeof(T);
  return AudioObjectGetPropertyData(object, &address, 0, nullptr, &size, &value) == noErr;
}

AudioObjectID defaultInputDevice() {
  AudioObjectID device = kAudioObjectUnknown;
  readProperty(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, device);
  return device;
}

std::string readString(AudioObjectID object, AudioObjectPropertySelector selector) {
  CFStringRef value = nullptr;
  if (!readProperty(object, selector, kAudioObjectPropertyScopeGlobal, value) || value == nullptr) return "";

  char buffer[512] = {};
  const bool ok = CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8);
  CFRelease(value);
  return ok ? std::string(buffer) : "";
}

AudioObjectID inputDeviceByUid(const std::string& uid) {
  if (uid.empty()) return defaultInputDevice();

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

std::uint32_t countInputChannels(AudioObjectID device) {
  auto address = property(kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeInput);
  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(device, &address, 0, nullptr, &size) != noErr || size == 0) return 0;

  auto* list = static_cast<AudioBufferList*>(std::calloc(1, size));
  if (list == nullptr) return 0;
  const auto status = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, list);
  std::uint32_t channels = 0;
  if (status == noErr) {
    for (UInt32 index = 0; index < list->mNumberBuffers; index += 1) channels += list->mBuffers[index].mNumberChannels;
  }
  std::free(list);
  return channels;
}

void updatePeak(MeterState& state, float sample) {
  const auto scaled = static_cast<int>(std::clamp(std::fabs(sample), 0.0f, 1.0f) * 1'000'000.0f);
  auto current = state.peakScaled.load(std::memory_order_relaxed);
  while (scaled > current &&
         !state.peakScaled.compare_exchange_weak(current, scaled, std::memory_order_relaxed)) {
  }
}

OSStatus meterCallback(
  AudioObjectID,
  const AudioTimeStamp*,
  const AudioBufferList* inputData,
  const AudioTimeStamp*,
  AudioBufferList*,
  const AudioTimeStamp*,
  void* clientData
) {
  if (inputData == nullptr) return noErr;
  auto* state = static_cast<MeterState*>(clientData);

  for (UInt32 bufferIndex = 0; bufferIndex < inputData->mNumberBuffers; bufferIndex += 1) {
    const auto& buffer = inputData->mBuffers[bufferIndex];
    const auto* samples = static_cast<const float*>(buffer.mData);
    if (samples == nullptr) continue;

    const auto sampleCount = buffer.mDataByteSize / sizeof(float);
    for (UInt32 sampleIndex = 0; sampleIndex < sampleCount; sampleIndex += 1) {
      updatePeak(*state, samples[sampleIndex]);
    }
  }

  return noErr;
}

}  // namespace

InputMeterResult measureInputPeak(const InputMeterRequest& request) {
  const auto device = inputDeviceByUid(request.inputUid);
  if (device == kAudioObjectUnknown) return {.error = "NO_INPUT_DEVICE"};

  Float64 actualRate = 0.0;
  if (!readProperty(device, kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, actualRate)) {
    return {.error = "INPUT_RATE_UNAVAILABLE"};
  }
  if (std::fabs(actualRate - request.projectSampleRate) > kRateTolerance) {
    return {.error = "SAMPLE_RATE_MISMATCH", .actualSampleRate = actualRate};
  }

  const auto channels = countInputChannels(device);
  if (channels == 0) return {.error = "NO_INPUT_CHANNELS", .actualSampleRate = actualRate};

  MeterState state;
  AudioDeviceIOProcID procId = nullptr;
  auto status = AudioDeviceCreateIOProcID(device, meterCallback, &state, &procId);
  if (status != noErr || procId == nullptr) return {.error = "INPUT_CALLBACK_CREATE_FAILED", .actualSampleRate = actualRate};

  status = AudioDeviceStart(device, procId);
  if (status != noErr) {
    AudioDeviceDestroyIOProcID(device, procId);
    return {.error = "INPUT_START_FAILED", .actualSampleRate = actualRate};
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(request.durationMs));

  AudioDeviceStop(device, procId);
  AudioDeviceDestroyIOProcID(device, procId);
  return {
    .ok = true,
    .inputChannels = channels,
    .actualSampleRate = actualRate,
    .peak = static_cast<float>(state.peakScaled.load(std::memory_order_relaxed)) / 1'000'000.0f,
  };
}

}  // namespace localmixer::platform::macos
