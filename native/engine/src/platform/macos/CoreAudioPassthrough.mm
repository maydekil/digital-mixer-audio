#include "platform/macos/CoreAudioPassthrough.hpp"

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

struct RingBuffer {
  std::vector<float> samples;
  std::atomic<std::uint64_t> writeFrame{0};
  std::atomic<std::uint64_t> readFrame{0};
  std::atomic<int> peakScaled{0};
};

struct PassthroughState {
  RingBuffer ring;
  std::uint32_t inputChannel = 0;
  std::uint32_t outputChannel = 0;
  bool mirrorToAllOutputChannels = true;
  float gain = 1.0f;
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

std::string readString(AudioObjectID object, AudioObjectPropertySelector selector) {
  CFStringRef value = nullptr;
  if (!readProperty(object, selector, kAudioObjectPropertyScopeGlobal, value) || value == nullptr) return "";

  char buffer[512] = {};
  const bool ok = CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8);
  CFRelease(value);
  return ok ? std::string(buffer) : "";
}

AudioObjectID defaultDevice(AudioObjectPropertySelector selector) {
  AudioObjectID device = kAudioObjectUnknown;
  readProperty(kAudioObjectSystemObject, selector, kAudioObjectPropertyScopeGlobal, device);
  return device;
}

AudioObjectID deviceByUid(const std::string& uid, AudioObjectPropertySelector defaultSelector) {
  if (uid.empty()) return defaultDevice(defaultSelector);

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

std::uint32_t countChannels(AudioObjectID device, AudioObjectPropertyScope scope) {
  auto address = property(kAudioDevicePropertyStreamConfiguration, scope);
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

bool sampleRate(AudioObjectID device, double& rate) {
  Float64 actualRate = 0.0;
  if (!readProperty(device, kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, actualRate)) return false;
  rate = actualRate;
  return true;
}

float readChannel(const AudioBufferList* data, std::uint32_t channel, UInt32 frame) {
  std::uint32_t base = 0;
  for (UInt32 bufferIndex = 0; bufferIndex < data->mNumberBuffers; bufferIndex += 1) {
    const auto& buffer = data->mBuffers[bufferIndex];
    const auto channels = std::max<UInt32>(1, buffer.mNumberChannels);
    if (channel < base + channels) {
      const auto* samples = static_cast<const float*>(buffer.mData);
      if (samples == nullptr) return 0.0f;
      return samples[frame * channels + (channel - base)];
    }
    base += channels;
  }
  return 0.0f;
}

void writeChannel(AudioBufferList* data, std::uint32_t channel, UInt32 frame, float sample) {
  std::uint32_t base = 0;
  for (UInt32 bufferIndex = 0; bufferIndex < data->mNumberBuffers; bufferIndex += 1) {
    auto& buffer = data->mBuffers[bufferIndex];
    const auto channels = std::max<UInt32>(1, buffer.mNumberChannels);
    if (channel < base + channels) {
      auto* samples = static_cast<float*>(buffer.mData);
      if (samples != nullptr) samples[frame * channels + (channel - base)] = sample;
      return;
    }
    base += channels;
  }
}

UInt32 frameCountFor(const AudioBufferList* data) {
  if (data == nullptr || data->mNumberBuffers == 0) return 0;
  const auto channels = std::max<UInt32>(1, data->mBuffers[0].mNumberChannels);
  return data->mBuffers[0].mDataByteSize / (sizeof(float) * channels);
}

void updatePeak(RingBuffer& ring, float sample) {
  const auto scaled = static_cast<int>(std::clamp(std::fabs(sample), 0.0f, 1.0f) * 1'000'000.0f);
  auto current = ring.peakScaled.load(std::memory_order_relaxed);
  while (scaled > current &&
         !ring.peakScaled.compare_exchange_weak(current, scaled, std::memory_order_relaxed)) {
  }
}

OSStatus inputCallback(
  AudioObjectID,
  const AudioTimeStamp*,
  const AudioBufferList* inputData,
  const AudioTimeStamp*,
  AudioBufferList*,
  const AudioTimeStamp*,
  void* clientData
) {
  if (inputData == nullptr) return noErr;
  auto* state = static_cast<PassthroughState*>(clientData);
  const auto frames = frameCountFor(inputData);
  const auto capacity = state->ring.samples.size();
  if (capacity == 0) return noErr;

  for (UInt32 frame = 0; frame < frames; frame += 1) {
    const auto sample = readChannel(inputData, state->inputChannel, frame);
    const auto write = state->ring.writeFrame.load(std::memory_order_relaxed);
    state->ring.samples[write % capacity] = sample;
    state->ring.writeFrame.store(write + 1, std::memory_order_release);
    updatePeak(state->ring, sample);
  }
  return noErr;
}

OSStatus outputCallback(
  AudioObjectID,
  const AudioTimeStamp*,
  const AudioBufferList*,
  const AudioTimeStamp*,
  AudioBufferList* outputData,
  const AudioTimeStamp*,
  void* clientData
) {
  if (outputData == nullptr) return noErr;
  auto* state = static_cast<PassthroughState*>(clientData);
  const auto frames = frameCountFor(outputData);
  const auto capacity = state->ring.samples.size();

  for (UInt32 bufferIndex = 0; bufferIndex < outputData->mNumberBuffers; bufferIndex += 1) {
    auto& buffer = outputData->mBuffers[bufferIndex];
    auto* samples = static_cast<float*>(buffer.mData);
    if (samples != nullptr) std::fill(samples, samples + (buffer.mDataByteSize / sizeof(float)), 0.0f);
  }

  if (capacity == 0) return noErr;
  for (UInt32 frame = 0; frame < frames; frame += 1) {
    const auto read = state->ring.readFrame.load(std::memory_order_relaxed);
    const auto write = state->ring.writeFrame.load(std::memory_order_acquire);
    float sample = 0.0f;
    if (read < write) {
      sample = state->ring.samples[read % capacity] * state->gain;
      sample = std::clamp(sample, -kLimitCeiling, kLimitCeiling);
      state->ring.readFrame.store(read + 1, std::memory_order_release);
    }
    if (state->mirrorToAllOutputChannels) {
      std::uint32_t channelCount = 0;
      for (UInt32 bufferIndex = 0; bufferIndex < outputData->mNumberBuffers; bufferIndex += 1) {
        channelCount += std::max<UInt32>(1, outputData->mBuffers[bufferIndex].mNumberChannels);
      }
      for (std::uint32_t channel = 0; channel < channelCount; channel += 1) {
        writeChannel(outputData, channel, frame, sample);
      }
    } else {
      writeChannel(outputData, state->outputChannel, frame, sample);
    }
  }
  return noErr;
}

}  // namespace

PassthroughMonitorResult monitorPassthrough(const PassthroughMonitorRequest& request) {
  const auto input = deviceByUid(request.inputUid, kAudioHardwarePropertyDefaultInputDevice);
  if (input == kAudioObjectUnknown) return {.error = "NO_INPUT_DEVICE"};
  const auto output = deviceByUid(request.outputUid, kAudioHardwarePropertyDefaultOutputDevice);
  if (output == kAudioObjectUnknown) return {.error = "NO_OUTPUT_DEVICE"};

  double inputRate = 0.0;
  double outputRate = 0.0;
  if (!sampleRate(input, inputRate)) return {.error = "INPUT_RATE_UNAVAILABLE"};
  if (!sampleRate(output, outputRate)) return {.error = "OUTPUT_RATE_UNAVAILABLE", .inputSampleRate = inputRate};
  if (std::fabs(inputRate - request.projectSampleRate) > kRateTolerance ||
      std::fabs(outputRate - request.projectSampleRate) > kRateTolerance) {
    return {.error = "SAMPLE_RATE_MISMATCH", .inputSampleRate = inputRate, .outputSampleRate = outputRate};
  }

  const auto inputChannels = countChannels(input, kAudioDevicePropertyScopeInput);
  const auto outputChannels = countChannels(output, kAudioDevicePropertyScopeOutput);
  if (inputChannels == 0) return {.error = "NO_INPUT_CHANNELS", .inputSampleRate = inputRate, .outputSampleRate = outputRate};
  if (outputChannels == 0) return {.error = "NO_OUTPUT_CHANNELS", .inputSampleRate = inputRate, .outputSampleRate = outputRate};
  if (request.inputChannel >= inputChannels) {
    return {.error = "INPUT_CHANNEL_OUT_OF_RANGE", .inputChannels = inputChannels, .outputChannels = outputChannels};
  }
  if (request.outputChannel >= outputChannels) {
    return {.error = "OUTPUT_CHANNEL_OUT_OF_RANGE", .inputChannels = inputChannels, .outputChannels = outputChannels};
  }

  PassthroughState state;
  state.inputChannel = request.inputChannel;
  state.outputChannel = request.outputChannel;
  state.mirrorToAllOutputChannels = request.mirrorToAllOutputChannels;
  state.gain = localmixer::dsp::decibelsToLinear(request.monitorGainDb);
  state.ring.samples.assign(static_cast<std::size_t>(request.projectSampleRate), 0.0f);

  AudioDeviceIOProcID inputProc = nullptr;
  AudioDeviceIOProcID outputProc = nullptr;
  auto status = AudioDeviceCreateIOProcID(input, inputCallback, &state, &inputProc);
  if (status != noErr || inputProc == nullptr) return {.error = "INPUT_CALLBACK_CREATE_FAILED"};
  status = AudioDeviceCreateIOProcID(output, outputCallback, &state, &outputProc);
  if (status != noErr || outputProc == nullptr) {
    AudioDeviceDestroyIOProcID(input, inputProc);
    return {.error = "OUTPUT_CALLBACK_CREATE_FAILED"};
  }

  status = AudioDeviceStart(input, inputProc);
  if (status != noErr) {
    AudioDeviceDestroyIOProcID(output, outputProc);
    AudioDeviceDestroyIOProcID(input, inputProc);
    return {.error = "INPUT_START_FAILED"};
  }
  status = AudioDeviceStart(output, outputProc);
  if (status != noErr) {
    AudioDeviceStop(input, inputProc);
    AudioDeviceDestroyIOProcID(output, outputProc);
    AudioDeviceDestroyIOProcID(input, inputProc);
    return {.error = "OUTPUT_START_FAILED"};
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(request.durationMs));

  AudioDeviceStop(output, outputProc);
  AudioDeviceStop(input, inputProc);
  AudioDeviceDestroyIOProcID(output, outputProc);
  AudioDeviceDestroyIOProcID(input, inputProc);
  return {
    .ok = true,
    .inputChannels = inputChannels,
    .outputChannels = outputChannels,
    .inputSampleRate = inputRate,
    .outputSampleRate = outputRate,
    .inputPeak = static_cast<float>(state.ring.peakScaled.load(std::memory_order_relaxed)) / 1'000'000.0f,
  };
}

}  // namespace localmixer::platform::macos
