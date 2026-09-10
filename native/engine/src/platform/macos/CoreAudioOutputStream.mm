#include "platform/macos/CoreAudioOutputStream.hpp"

#include "dsp/Gain.hpp"

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
constexpr float kLimitCeiling = 0.98f;
constexpr double kTwoPi = 6.283185307179586;

struct ToneState {
  std::atomic<std::uint64_t> framesRemaining{0};
  double phase = 0.0;
  double phaseInc = 0.0;
  float level = 0.0f;
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

AudioObjectID defaultOutputDevice() {
  AudioObjectID device = kAudioObjectUnknown;
  readProperty(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, device);
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

AudioObjectID outputDeviceByUid(const std::string& uid) {
  if (uid.empty()) return defaultOutputDevice();

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

std::uint32_t countOutputChannels(AudioObjectID device) {
  auto address = property(kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeOutput);
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

OSStatus toneCallback(
  AudioObjectID,
  const AudioTimeStamp*,
  const AudioBufferList*,
  const AudioTimeStamp*,
  AudioBufferList* outData,
  const AudioTimeStamp*,
  void* clientData
) {
  auto* state = static_cast<ToneState*>(clientData);
  if (outData->mNumberBuffers == 0) return noErr;

  const auto firstChannels = std::max<UInt32>(1, outData->mBuffers[0].mNumberChannels);
  const auto frameCount = outData->mBuffers[0].mDataByteSize / (sizeof(float) * firstChannels);
  for (UInt32 frame = 0; frame < frameCount; frame += 1) {
    const auto remaining = state->framesRemaining.load(std::memory_order_relaxed);
    float sample = 0.0f;
    if (remaining > 0) {
      const auto raw = static_cast<float>(std::sin(state->phase) * state->level);
      sample = std::clamp(raw, -kLimitCeiling, kLimitCeiling);
      state->phase += state->phaseInc;
      if (state->phase >= kTwoPi) state->phase -= kTwoPi;
      state->framesRemaining.store(remaining - 1, std::memory_order_relaxed);
    }

    for (UInt32 bufferIndex = 0; bufferIndex < outData->mNumberBuffers; bufferIndex += 1) {
      auto& buffer = outData->mBuffers[bufferIndex];
      auto* samples = static_cast<float*>(buffer.mData);
      const auto channels = std::max<UInt32>(1, buffer.mNumberChannels);
      const auto offset = frame * channels;
      for (UInt32 channel = 0; channel < channels; channel += 1) {
        samples[offset + channel] = sample;
      }
    }
  }
  return noErr;
}

}  // namespace

TestToneResult playProtectedTestTone(const TestToneRequest& request) {
  const auto device = outputDeviceByUid(request.outputUid);
  if (device == kAudioObjectUnknown) return {.error = "NO_OUTPUT_DEVICE"};

  Float64 actualRate = 0.0;
  if (!readProperty(device, kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, actualRate)) {
    return {.error = "OUTPUT_RATE_UNAVAILABLE"};
  }
  if (std::fabs(actualRate - request.projectSampleRate) > kRateTolerance) {
    return {.error = "SAMPLE_RATE_MISMATCH", .actualSampleRate = actualRate};
  }

  const auto channels = countOutputChannels(device);
  if (channels == 0) return {.error = "NO_OUTPUT_CHANNELS", .actualSampleRate = actualRate};

  ToneState state;
  state.phaseInc = kTwoPi * request.frequencyHz / actualRate;
  state.level = localmixer::dsp::decibelsToLinear(request.levelDb + request.monitorGainDb);
  state.framesRemaining.store(
    static_cast<std::uint64_t>((actualRate * request.durationMs) / 1000.0),
    std::memory_order_relaxed
  );

  AudioDeviceIOProcID procId = nullptr;
  auto status = AudioDeviceCreateIOProcID(device, toneCallback, &state, &procId);
  if (status != noErr || procId == nullptr) return {.error = "OUTPUT_CALLBACK_CREATE_FAILED", .actualSampleRate = actualRate};

  status = AudioDeviceStart(device, procId);
  if (status != noErr) {
    AudioDeviceDestroyIOProcID(device, procId);
    return {.error = "OUTPUT_START_FAILED", .actualSampleRate = actualRate};
  }

  while (state.framesRemaining.load(std::memory_order_relaxed) > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  AudioDeviceStop(device, procId);
  AudioDeviceDestroyIOProcID(device, procId);
  return {.ok = true, .outputChannels = channels, .actualSampleRate = actualRate};
}

}  // namespace localmixer::platform::macos
