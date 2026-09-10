#include "platform/macos/CoreAudioPassthrough.hpp"

#include "dsp/Gain.hpp"
#include "engine/MixerRenderRuntime.hpp"

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <span>
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
  localmixer::engine::MixerRenderRuntime runtime;
  localmixer::engine::StripId graphStrip;
  std::vector<float> graphInput;
  std::vector<float> graphLeft;
  std::vector<float> graphRight;
  std::uint32_t inputChannel = 0;
  std::uint32_t outputChannel = 0;
  bool mirrorToAllOutputChannels = true;
};

struct DeviceContext {
  AudioObjectID input = kAudioObjectUnknown;
  AudioObjectID output = kAudioObjectUnknown;
  std::uint32_t inputChannels = 0;
  std::uint32_t outputChannels = 0;
  double inputRate = 0.0;
  double outputRate = 0.0;
  std::string error;
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
      sample = state->ring.samples[read % capacity];
      state->ring.readFrame.store(read + 1, std::memory_order_release);
    }
    if (frame < state->graphInput.size()) state->graphInput[frame] = sample;
  }

  if (frames > state->graphInput.size() || frames > state->graphLeft.size() || frames > state->graphRight.size()) {
    return noErr;
  }

  const std::array<localmixer::engine::SourceBuffer, 1> sources{
    localmixer::engine::SourceBuffer{
      .stripId = state->graphStrip,
      .samples = std::span<const float>(state->graphInput.data(), frames),
      .channels = 1,
    }
  };
  state->runtime.process(sources, {
    .left = std::span<float>(state->graphLeft.data(), frames),
    .right = std::span<float>(state->graphRight.data(), frames),
  });

  for (UInt32 frame = 0; frame < frames; frame += 1) {
    const auto left = std::clamp(state->graphLeft[frame], -kLimitCeiling, kLimitCeiling);
    const auto right = std::clamp(state->graphRight[frame], -kLimitCeiling, kLimitCeiling);
    if (state->mirrorToAllOutputChannels) {
      writeChannel(outputData, 0, frame, left);
      writeChannel(outputData, 1, frame, right);
    } else {
      writeChannel(outputData, state->outputChannel, frame, (left + right) * 0.5f);
    }
  }
  return noErr;
}

void prepareMonitorGraph(PassthroughState& state, const PassthroughMonitorRequest& request) {
  state.runtime = localmixer::engine::MixerRenderRuntime{request.projectSampleRate};
  state.runtime.setFxProgram(localmixer::engine::FxBusId::a, request.fxAProgramId);
  state.runtime.setFxProgram(localmixer::engine::FxBusId::b, request.fxBProgramId);
  auto& graph = state.runtime.graph();
  const auto created = graph.createStrip("Monitor", "#18d6e7");
  state.graphStrip = created.id;
  const auto scratchFrames = static_cast<std::size_t>(std::max<double>(request.projectSampleRate, 512.0));
  state.runtime.prepare(static_cast<std::uint32_t>(scratchFrames));
  graph.setAssignment(created.id, localmixer::engine::SourceAssignment::mono, 0, false);
  graph.setLevel(created.id, 0.0f, request.monitorGainDb, request.monitorPan);
  graph.setInputMonitoring(created.id, true);
  graph.setProcessors(created.id, request.processors);
  graph.setFxUnit(localmixer::engine::FxBusId::a, request.fxA);
  graph.setFxUnit(localmixer::engine::FxBusId::b, request.fxB);
  graph.setFxSend(created.id, localmixer::engine::FxBusId::a, request.sendA);
  graph.setFxSend(created.id, localmixer::engine::FxBusId::b, request.sendB);
  state.graphInput.assign(scratchFrames, 0.0f);
  state.graphLeft.assign(scratchFrames, 0.0f);
  state.graphRight.assign(scratchFrames, 0.0f);
}

DeviceContext prepareDeviceContext(const PassthroughMonitorRequest& request) {
  DeviceContext context;
  context.input = deviceByUid(request.inputUid, kAudioHardwarePropertyDefaultInputDevice);
  if (context.input == kAudioObjectUnknown) {
    context.error = "NO_INPUT_DEVICE";
    return context;
  }
  context.output = deviceByUid(request.outputUid, kAudioHardwarePropertyDefaultOutputDevice);
  if (context.output == kAudioObjectUnknown) {
    context.error = "NO_OUTPUT_DEVICE";
    return context;
  }

  if (!sampleRate(context.input, context.inputRate)) {
    context.error = "INPUT_RATE_UNAVAILABLE";
    return context;
  }
  if (!sampleRate(context.output, context.outputRate)) {
    context.error = "OUTPUT_RATE_UNAVAILABLE";
    return context;
  }
  if (std::fabs(context.inputRate - request.projectSampleRate) > kRateTolerance ||
      std::fabs(context.outputRate - request.projectSampleRate) > kRateTolerance) {
    context.error = "SAMPLE_RATE_MISMATCH";
    return context;
  }

  context.inputChannels = countChannels(context.input, kAudioDevicePropertyScopeInput);
  context.outputChannels = countChannels(context.output, kAudioDevicePropertyScopeOutput);
  if (context.inputChannels == 0) {
    context.error = "NO_INPUT_CHANNELS";
    return context;
  }
  if (context.outputChannels == 0) {
    context.error = "NO_OUTPUT_CHANNELS";
    return context;
  }
  if (request.inputChannel >= context.inputChannels) {
    context.error = "INPUT_CHANNEL_OUT_OF_RANGE";
    return context;
  }
  if (request.outputChannel >= context.outputChannels) {
    context.error = "OUTPUT_CHANNEL_OUT_OF_RANGE";
    return context;
  }
  return context;
}

PersistentMonitorStatus statusFromContext(
  const DeviceContext& context,
  bool running,
  const std::string& error,
  const RingBuffer& ring
) {
  return {
    .running = running,
    .error = error,
    .inputChannels = context.inputChannels,
    .outputChannels = context.outputChannels,
    .inputSampleRate = context.inputRate,
    .outputSampleRate = context.outputRate,
    .inputPeak = static_cast<float>(ring.peakScaled.load(std::memory_order_relaxed)) / 1'000'000.0f,
  };
}

}  // namespace

struct PersistentPassthroughMonitor::Impl {
  ~Impl() { stop(); }

  PersistentMonitorStatus start(const PassthroughMonitorRequest& request) {
    stop();
    context = prepareDeviceContext(request);
    if (!context.error.empty()) return statusFromContext(context, false, context.error, state.ring);

    state.inputChannel = request.inputChannel;
    state.outputChannel = request.outputChannel;
    state.mirrorToAllOutputChannels = request.mirrorToAllOutputChannels;
    state.ring.samples.assign(static_cast<std::size_t>(request.projectSampleRate), 0.0f);
    state.ring.writeFrame.store(0, std::memory_order_relaxed);
    state.ring.readFrame.store(0, std::memory_order_relaxed);
    state.ring.peakScaled.store(0, std::memory_order_relaxed);
    prepareMonitorGraph(state, request);

    auto audioStatus = AudioDeviceCreateIOProcID(context.input, inputCallback, &state, &inputProc);
    if (audioStatus != noErr || inputProc == nullptr) {
      return statusFromContext(context, false, "INPUT_CALLBACK_CREATE_FAILED", state.ring);
    }
    audioStatus = AudioDeviceCreateIOProcID(context.output, outputCallback, &state, &outputProc);
    if (audioStatus != noErr || outputProc == nullptr) {
      AudioDeviceDestroyIOProcID(context.input, inputProc);
      inputProc = nullptr;
      return statusFromContext(context, false, "OUTPUT_CALLBACK_CREATE_FAILED", state.ring);
    }

    audioStatus = AudioDeviceStart(context.input, inputProc);
    if (audioStatus != noErr) {
      destroyCallbacks();
      return statusFromContext(context, false, "INPUT_START_FAILED", state.ring);
    }
    audioStatus = AudioDeviceStart(context.output, outputProc);
    if (audioStatus != noErr) {
      AudioDeviceStop(context.input, inputProc);
      destroyCallbacks();
      return statusFromContext(context, false, "OUTPUT_START_FAILED", state.ring);
    }

    running = true;
    return statusFromContext(context, true, "", state.ring);
  }

  PersistentMonitorStatus stop() {
    if (running) {
      AudioDeviceStop(context.output, outputProc);
      AudioDeviceStop(context.input, inputProc);
    }
    destroyCallbacks();
    running = false;
    return statusFromContext(context, false, "", state.ring);
  }

  PersistentMonitorStatus currentStatus() const {
    return statusFromContext(context, running, "", state.ring);
  }

  void destroyCallbacks() {
    if (outputProc != nullptr && context.output != kAudioObjectUnknown) {
      AudioDeviceDestroyIOProcID(context.output, outputProc);
      outputProc = nullptr;
    }
    if (inputProc != nullptr && context.input != kAudioObjectUnknown) {
      AudioDeviceDestroyIOProcID(context.input, inputProc);
      inputProc = nullptr;
    }
  }

  DeviceContext context;
  PassthroughState state;
  AudioDeviceIOProcID inputProc = nullptr;
  AudioDeviceIOProcID outputProc = nullptr;
  bool running = false;
};

PersistentPassthroughMonitor::PersistentPassthroughMonitor() : impl_(std::make_unique<Impl>()) {}

PersistentPassthroughMonitor::~PersistentPassthroughMonitor() = default;

PersistentMonitorStatus PersistentPassthroughMonitor::start(const PassthroughMonitorRequest& request) {
  return impl_->start(request);
}

PersistentMonitorStatus PersistentPassthroughMonitor::stop() {
  return impl_->stop();
}

PersistentMonitorStatus PersistentPassthroughMonitor::status() const {
  return impl_->currentStatus();
}

PassthroughMonitorResult monitorPassthrough(const PassthroughMonitorRequest& request) {
  const auto context = prepareDeviceContext(request);
  if (!context.error.empty()) {
    return {
      .error = context.error,
      .inputChannels = context.inputChannels,
      .outputChannels = context.outputChannels,
      .inputSampleRate = context.inputRate,
      .outputSampleRate = context.outputRate,
    };
  }

  PassthroughState state;
  state.inputChannel = request.inputChannel;
  state.outputChannel = request.outputChannel;
  state.mirrorToAllOutputChannels = request.mirrorToAllOutputChannels;
  state.ring.samples.assign(static_cast<std::size_t>(request.projectSampleRate), 0.0f);
  prepareMonitorGraph(state, request);

  AudioDeviceIOProcID inputProc = nullptr;
  AudioDeviceIOProcID outputProc = nullptr;
  auto status = AudioDeviceCreateIOProcID(context.input, inputCallback, &state, &inputProc);
  if (status != noErr || inputProc == nullptr) return {.error = "INPUT_CALLBACK_CREATE_FAILED"};
  status = AudioDeviceCreateIOProcID(context.output, outputCallback, &state, &outputProc);
  if (status != noErr || outputProc == nullptr) {
    AudioDeviceDestroyIOProcID(context.input, inputProc);
    return {.error = "OUTPUT_CALLBACK_CREATE_FAILED"};
  }

  status = AudioDeviceStart(context.input, inputProc);
  if (status != noErr) {
    AudioDeviceDestroyIOProcID(context.output, outputProc);
    AudioDeviceDestroyIOProcID(context.input, inputProc);
    return {.error = "INPUT_START_FAILED"};
  }
  status = AudioDeviceStart(context.output, outputProc);
  if (status != noErr) {
    AudioDeviceStop(context.input, inputProc);
    AudioDeviceDestroyIOProcID(context.output, outputProc);
    AudioDeviceDestroyIOProcID(context.input, inputProc);
    return {.error = "OUTPUT_START_FAILED"};
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(request.durationMs));

  AudioDeviceStop(context.output, outputProc);
  AudioDeviceStop(context.input, inputProc);
  AudioDeviceDestroyIOProcID(context.output, outputProc);
  AudioDeviceDestroyIOProcID(context.input, inputProc);
  return {
    .ok = true,
    .inputChannels = context.inputChannels,
    .outputChannels = context.outputChannels,
    .inputSampleRate = context.inputRate,
    .outputSampleRate = context.outputRate,
    .inputPeak = static_cast<float>(state.ring.peakScaled.load(std::memory_order_relaxed)) / 1'000'000.0f,
  };
}

}  // namespace localmixer::platform::macos
