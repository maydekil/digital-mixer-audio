#include "platform/macos/CoreAudioPassthrough.hpp"

#include "dsp/Gain.hpp"
#include "engine/MediaFile.hpp"
#include "engine/MixerRenderRuntime.hpp"
#include "engine/VocalFxRackRuntime.hpp"

#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <span>
#include <thread>
#include <utility>
#include <vector>

namespace localmixer::platform::macos {
namespace {

constexpr double kRateTolerance = 0.01;
constexpr float kLimitCeiling = 0.98f;

struct RingBuffer {
  RingBuffer() = default;
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;
  RingBuffer(RingBuffer&& other) noexcept
    : samples(std::move(other.samples)) {
    writeFrame.store(other.writeFrame.load(std::memory_order_relaxed), std::memory_order_relaxed);
    readFrame.store(other.readFrame.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaled.store(other.peakScaled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaledLeft.store(other.peakScaledLeft.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaledRight.store(other.peakScaledRight.load(std::memory_order_relaxed), std::memory_order_relaxed);
  }
  RingBuffer& operator=(RingBuffer&& other) noexcept {
    if (this == &other) return *this;
    samples = std::move(other.samples);
    writeFrame.store(other.writeFrame.load(std::memory_order_relaxed), std::memory_order_relaxed);
    readFrame.store(other.readFrame.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaled.store(other.peakScaled.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaledLeft.store(other.peakScaledLeft.load(std::memory_order_relaxed), std::memory_order_relaxed);
    peakScaledRight.store(other.peakScaledRight.load(std::memory_order_relaxed), std::memory_order_relaxed);
    return *this;
  }

  std::vector<float> samples;
  std::atomic<std::uint64_t> writeFrame{0};
  std::atomic<std::uint64_t> readFrame{0};
  std::atomic<int> peakScaled{0};
  std::atomic<int> peakScaledLeft{0};
  std::atomic<int> peakScaledRight{0};
};

struct MonitorSourceState {
  RingBuffer ring;
  localmixer::engine::VocalFxRackRuntime vocalFx;
  localmixer::engine::StripId graphStrip;
  std::vector<float> graphInput;
  std::vector<float> graphStereoInput;
  std::vector<float> graphFxLeft;
  std::vector<float> graphFxRight;
  std::vector<float> fileSamples;
  std::uint64_t fileFrame = 0;
  std::uint64_t fileFrameCount = 0;
  std::uint32_t fileChannels = 0;
  std::uint32_t inputChannel = 0;
  std::uint32_t ringChannels = 1;
  bool fileSource = false;
};

struct PassthroughState {
  std::vector<MonitorSourceState> sources;
  std::vector<std::size_t> deviceSourceIndices;
  localmixer::engine::MixerRenderRuntime runtime;
  std::vector<localmixer::engine::SourceBuffer> renderSources;
  std::vector<float> graphLeft;
  std::vector<float> graphRight;
  std::uint32_t outputChannel = 0;
  double outputSampleRate = 48000.0;
  bool mirrorToAllOutputChannels = true;
  localmixer::engine::RealtimeMetrics metrics;
};

struct InputCallbackClient {
  PassthroughState* state = nullptr;
  std::size_t sourceIndex = 0;
};

struct InputDeviceContext {
  AudioObjectID device = kAudioObjectUnknown;
  std::uint32_t channels = 0;
  double rate = 0.0;
};

struct DeviceContext {
  std::vector<InputDeviceContext> inputs;
  AudioObjectID output = kAudioObjectUnknown;
  std::uint32_t outputChannels = 0;
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

void updatePeak(std::atomic<int>& peakScaled, float sample) {
  const auto scaled = static_cast<int>(std::clamp(std::fabs(sample), 0.0f, 1.0f) * 1'000'000.0f);
  auto current = peakScaled.load(std::memory_order_relaxed);
  while (scaled > current &&
         !peakScaled.compare_exchange_weak(current, scaled, std::memory_order_relaxed)) {
  }
}

void updateStereoPeak(RingBuffer& ring, float left, float right) {
  updatePeak(ring.peakScaledLeft, left);
  updatePeak(ring.peakScaledRight, right);
  updatePeak(ring.peakScaled, std::max(std::fabs(left), std::fabs(right)));
}

std::vector<PassthroughMonitorSource> monitorSources(const PassthroughMonitorRequest& request) {
  if (!request.sources.empty()) return request.sources;
  return {PassthroughMonitorSource{
    .inputUid = request.inputUid,
    .label = "Monitor",
    .fileSource = false,
    .inputChannel = request.inputChannel,
    .stereoInput = request.stereoInput,
    .monitorGainDb = request.monitorGainDb,
    .monitorPan = request.monitorPan,
    .processors = request.processors,
    .sendA = request.sendA,
    .sendB = request.sendB,
    .insertFxEnabled = request.insertFxEnabled,
    .vocalFxSlots = request.vocalFxSlots,
  }};
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
  auto* client = static_cast<InputCallbackClient*>(clientData);
  if (client == nullptr || client->state == nullptr || client->sourceIndex >= client->state->sources.size()) return noErr;
  auto& source = client->state->sources[client->sourceIndex];
  const auto frames = frameCountFor(inputData);
  const auto ringChannels = std::max<std::uint32_t>(1, source.ringChannels);
  const auto capacity = source.ring.samples.size() / ringChannels;
  if (capacity == 0) return noErr;

  for (UInt32 frame = 0; frame < frames; frame += 1) {
    const auto left = readChannel(inputData, source.inputChannel, frame);
    const auto right = ringChannels > 1 ? readChannel(inputData, source.inputChannel + 1, frame) : left;
    const auto write = source.ring.writeFrame.load(std::memory_order_relaxed);
    const auto base = (write % capacity) * ringChannels;
    source.ring.samples[base] = left;
    if (ringChannels > 1) source.ring.samples[base + 1] = right;
    source.ring.writeFrame.store(write + 1, std::memory_order_release);
    updateStereoPeak(source.ring, left, right);
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
  const auto callbackStart = std::chrono::steady_clock::now();
  auto* state = static_cast<PassthroughState*>(clientData);
  const auto frames = frameCountFor(outputData);

  for (UInt32 bufferIndex = 0; bufferIndex < outputData->mNumberBuffers; bufferIndex += 1) {
    auto& buffer = outputData->mBuffers[bufferIndex];
    auto* samples = static_cast<float*>(buffer.mData);
    if (samples != nullptr) std::fill(samples, samples + (buffer.mDataByteSize / sizeof(float)), 0.0f);
  }

  if (frames > state->graphLeft.size() || frames > state->graphRight.size() ||
      state->renderSources.size() != state->sources.size()) {
    return noErr;
  }

  for (std::size_t sourceIndex = 0; sourceIndex < state->sources.size(); sourceIndex += 1) {
    auto& source = state->sources[sourceIndex];
    const auto ringChannels = std::max<std::uint32_t>(1, source.ringChannels);
    const auto capacity = source.ring.samples.size() / ringChannels;
    if ((!source.fileSource && capacity == 0) || frames > source.graphInput.size()) return noErr;
    if (ringChannels > 1 && frames * 2 > source.graphStereoInput.size()) return noErr;

    for (UInt32 frame = 0; frame < frames; frame += 1) {
      float left = 0.0f;
      float right = 0.0f;
      if (source.fileSource && source.fileFrameCount > 0 && source.fileChannels > 0) {
        const auto fileBase = (source.fileFrame % source.fileFrameCount) * source.fileChannels;
        left = source.fileSamples[static_cast<std::size_t>(fileBase)];
        right = source.fileChannels > 1 ? source.fileSamples[static_cast<std::size_t>(fileBase + 1)] : left;
        source.fileFrame = (source.fileFrame + 1) % source.fileFrameCount;
        updateStereoPeak(source.ring, left, right);
      } else {
        const auto read = source.ring.readFrame.load(std::memory_order_relaxed);
        const auto write = source.ring.writeFrame.load(std::memory_order_acquire);
        if (read >= write) {
          source.graphInput[frame] = 0.0f;
          if (ringChannels > 1) {
            source.graphStereoInput[frame * 2] = 0.0f;
            source.graphStereoInput[frame * 2 + 1] = 0.0f;
          }
          continue;
        }
        const auto base = (read % capacity) * ringChannels;
        left = source.ring.samples[base];
        right = ringChannels > 1 ? source.ring.samples[base + 1] : left;
        source.ring.readFrame.store(read + 1, std::memory_order_release);
      }
      source.graphInput[frame] = (left + right) * 0.5f;
      if (ringChannels > 1) {
        source.graphStereoInput[frame * 2] = left;
        source.graphStereoInput[frame * 2 + 1] = right;
      }
    }

    std::span<const float> graphSource = std::span<const float>(source.graphInput.data(), frames);
    std::uint32_t graphChannels = 1;
    if (ringChannels > 1) {
      graphSource = std::span<const float>(source.graphStereoInput.data(), frames * 2);
      graphChannels = 2;
    } else if (source.vocalFx.active() && frames * 2 <= source.graphStereoInput.size()) {
      source.vocalFx.processMonoToStereo(
        graphSource,
        std::span<float>(source.graphFxLeft.data(), frames),
        std::span<float>(source.graphFxRight.data(), frames));
      for (std::size_t frame = 0; frame < frames; frame += 1) {
        source.graphStereoInput[frame * 2] = source.graphFxLeft[frame];
        source.graphStereoInput[frame * 2 + 1] = source.graphFxRight[frame];
      }
      graphSource = std::span<const float>(source.graphStereoInput.data(), frames * 2);
      graphChannels = 2;
    }
    state->renderSources[sourceIndex] = localmixer::engine::SourceBuffer{
      .stripId = source.graphStrip,
      .samples = graphSource,
      .channels = graphChannels
    };
  }

  state->runtime.process(state->renderSources, {
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
  const auto callbackEnd = std::chrono::steady_clock::now();
  const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(callbackEnd - callbackStart).count();
  const auto deadline = static_cast<std::uint64_t>(
    (static_cast<double>(frames) / std::max(1.0, state->outputSampleRate)) * 1'000'000'000.0);
  state->metrics.recordCallback(static_cast<std::uint64_t>(std::max<std::int64_t>(0, elapsed)), deadline);
  return noErr;
}

void prepareMonitorGraph(PassthroughState& state, const PassthroughMonitorRequest& request) {
  const auto requestSources = monitorSources(request);
  state.runtime = localmixer::engine::MixerRenderRuntime{request.projectSampleRate};
  state.runtime.setFxProgram(localmixer::engine::FxBusId::a, request.fxAProgramId);
  state.runtime.setFxProgram(localmixer::engine::FxBusId::b, request.fxBProgramId);
  auto& graph = state.runtime.graph();
  const auto scratchFrames = static_cast<std::size_t>(std::max<double>(request.projectSampleRate, 512.0));
  state.sources.clear();
  state.sources.resize(requestSources.size());
  state.deviceSourceIndices.clear();
  state.renderSources.resize(requestSources.size());
  state.runtime.prepare(static_cast<std::uint32_t>(scratchFrames));
  graph.setFxUnit(localmixer::engine::FxBusId::a, request.fxA);
  graph.setFxUnit(localmixer::engine::FxBusId::b, request.fxB);

  for (std::size_t index = 0; index < requestSources.size(); index += 1) {
    const auto& requestSource = requestSources[index];
    auto& source = state.sources[index];
    const auto created = graph.createStrip(requestSource.label.empty() ? "Monitor" : requestSource.label, "#18d6e7");
    source.graphStrip = created.id;
    source.inputChannel = requestSource.inputChannel;
    source.ringChannels = requestSource.stereoInput ? 2 : 1;
    source.fileSource = requestSource.fileSource;
    if (source.fileSource) {
      localmixer::engine::WavStreamReader reader;
      if (reader.open(requestSource.inputUid) == localmixer::engine::MediaFileError::none) {
        const auto read = reader.readFrames(0, static_cast<std::uint32_t>(
          std::min<std::uint64_t>(reader.info().frameCount, std::numeric_limits<std::uint32_t>::max())));
        if (read.error == localmixer::engine::MediaFileError::none && read.framesRead > 0) {
          source.fileSamples = read.samples;
          source.fileFrameCount = read.framesRead;
          source.fileChannels = reader.info().channels;
          source.ringChannels = source.fileChannels > 1 ? 2 : 1;
        }
      }
    } else {
      state.deviceSourceIndices.push_back(index);
    }
    source.vocalFx.prepare(request.projectSampleRate, static_cast<std::uint32_t>(scratchFrames));
    source.vocalFx.configure(
      requestSource.insertFxEnabled ? requestSource.vocalFxSlots : std::vector<localmixer::dsp::fx::RackSlotState>{});
    const auto graphStereo = requestSource.stereoInput || source.vocalFx.active();
    graph.setAssignment(
      created.id,
      graphStereo ? localmixer::engine::SourceAssignment::stereo : localmixer::engine::SourceAssignment::mono,
      0,
      graphStereo);
    graph.setLevel(created.id, 0.0f, requestSource.monitorGainDb, requestSource.monitorPan);
    graph.setInputMonitoring(created.id, true);
    graph.setProcessors(created.id, requestSource.processors);
    graph.setFxSend(created.id, localmixer::engine::FxBusId::a, requestSource.sendA);
    graph.setFxSend(created.id, localmixer::engine::FxBusId::b, requestSource.sendB);
    source.graphInput.assign(scratchFrames, 0.0f);
    source.graphStereoInput.assign(scratchFrames * 2, 0.0f);
    source.graphFxLeft.assign(scratchFrames, 0.0f);
    source.graphFxRight.assign(scratchFrames, 0.0f);
  }
  state.graphLeft.assign(scratchFrames, 0.0f);
  state.graphRight.assign(scratchFrames, 0.0f);
}

DeviceContext prepareDeviceContext(const PassthroughMonitorRequest& request) {
  DeviceContext context;
  const auto requestSources = monitorSources(request);
  context.inputs.reserve(requestSources.size());
  context.output = deviceByUid(request.outputUid, kAudioHardwarePropertyDefaultOutputDevice);
  if (context.output == kAudioObjectUnknown) {
    context.error = "NO_OUTPUT_DEVICE";
    return context;
  }

  if (!sampleRate(context.output, context.outputRate)) {
    context.error = "OUTPUT_RATE_UNAVAILABLE";
    return context;
  }
  if (std::fabs(context.outputRate - request.projectSampleRate) > kRateTolerance) {
    context.error = "SAMPLE_RATE_MISMATCH";
    return context;
  }

  for (const auto& source : requestSources) {
    if (source.fileSource) {
      localmixer::engine::WavStreamReader reader;
      const auto error = reader.open(source.inputUid);
      if (error != localmixer::engine::MediaFileError::none) {
        context.error = "MEDIA_FILE_OPEN_FAILED";
        return context;
      }
      if (std::fabs(static_cast<double>(reader.info().sampleRate) - request.projectSampleRate) > kRateTolerance) {
        context.error = "SAMPLE_RATE_MISMATCH";
        return context;
      }
      continue;
    }
    InputDeviceContext input;
    input.device = deviceByUid(source.inputUid, kAudioHardwarePropertyDefaultInputDevice);
    if (input.device == kAudioObjectUnknown) {
      context.error = "NO_INPUT_DEVICE";
      return context;
    }
    if (!sampleRate(input.device, input.rate)) {
      context.error = "INPUT_RATE_UNAVAILABLE";
      return context;
    }
    if (std::fabs(input.rate - request.projectSampleRate) > kRateTolerance) {
      context.error = "SAMPLE_RATE_MISMATCH";
      return context;
    }
    input.channels = countChannels(input.device, kAudioDevicePropertyScopeInput);
    if (input.channels == 0) {
      context.error = "NO_INPUT_CHANNELS";
      return context;
    }
    if (source.inputChannel >= input.channels) {
      context.error = "INPUT_CHANNEL_OUT_OF_RANGE";
      return context;
    }
    if (source.stereoInput && source.inputChannel + 1 >= input.channels) {
      context.error = "INPUT_STEREO_CHANNEL_OUT_OF_RANGE";
      return context;
    }
    context.inputs.push_back(input);
  }

  context.outputChannels = countChannels(context.output, kAudioDevicePropertyScopeOutput);
  if (context.outputChannels == 0) {
    context.error = "NO_OUTPUT_CHANNELS";
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
  PassthroughState& state
) {
  int peak = 0;
  int peakLeft = 0;
  int peakRight = 0;
  for (auto& source : state.sources) {
    peak = std::max(peak, source.ring.peakScaled.exchange(0, std::memory_order_relaxed));
    peakLeft = std::max(peakLeft, source.ring.peakScaledLeft.exchange(0, std::memory_order_relaxed));
    peakRight = std::max(peakRight, source.ring.peakScaledRight.exchange(0, std::memory_order_relaxed));
  }
  std::uint32_t inputChannels = 0;
  double inputRate = 0.0;
  for (const auto& input : context.inputs) {
    inputChannels += input.channels;
    if (inputRate == 0.0) inputRate = input.rate;
  }
  return {
    .running = running,
    .error = error,
    .inputChannels = inputChannels,
    .outputChannels = context.outputChannels,
    .inputSampleRate = inputRate,
    .outputSampleRate = context.outputRate,
    .inputPeak = static_cast<float>(peak) / 1'000'000.0f,
    .inputPeakLeft = static_cast<float>(peakLeft) / 1'000'000.0f,
    .inputPeakRight = static_cast<float>(peakRight) / 1'000'000.0f,
  };
}

PersistentMonitorStatus statusFromContext(
  const DeviceContext& context,
  bool running,
  const std::string& error,
  PassthroughState& state,
  const localmixer::engine::RealtimeMetrics& metrics
) {
  auto status = statusFromContext(context, running, error, state);
  status.metrics = metrics.snapshot();
  return status;
}

}  // namespace

struct PersistentPassthroughMonitor::Impl {
  ~Impl() { stop(); }

  PersistentMonitorStatus start(const PassthroughMonitorRequest& request) {
    stop();
    context = prepareDeviceContext(request);
    if (!context.error.empty()) return statusFromContext(context, false, context.error, state);

    state.outputChannel = request.outputChannel;
    state.outputSampleRate = context.outputRate;
    state.mirrorToAllOutputChannels = request.mirrorToAllOutputChannels;
    state.metrics.reset();
    prepareMonitorGraph(state, request);
    for (auto& source : state.sources) {
      source.ring.samples.assign(static_cast<std::size_t>(request.projectSampleRate) * source.ringChannels, 0.0f);
      source.ring.writeFrame.store(0, std::memory_order_relaxed);
      source.ring.readFrame.store(0, std::memory_order_relaxed);
      source.ring.peakScaled.store(0, std::memory_order_relaxed);
      source.ring.peakScaledLeft.store(0, std::memory_order_relaxed);
      source.ring.peakScaledRight.store(0, std::memory_order_relaxed);
    }

    inputProcs.assign(state.deviceSourceIndices.size(), nullptr);
    inputClients.assign(state.deviceSourceIndices.size(), {});
    for (std::size_t index = 0; index < state.deviceSourceIndices.size(); index += 1) {
      inputClients[index] = InputCallbackClient{.state = &state, .sourceIndex = state.deviceSourceIndices[index]};
      auto audioStatus = AudioDeviceCreateIOProcID(
        context.inputs[index].device, inputCallback, &inputClients[index], &inputProcs[index]);
      if (audioStatus != noErr || inputProcs[index] == nullptr) {
        destroyCallbacks();
        return statusFromContext(context, false, "INPUT_CALLBACK_CREATE_FAILED", state, state.metrics);
      }
    }
    auto audioStatus = AudioDeviceCreateIOProcID(context.output, outputCallback, &state, &outputProc);
    if (audioStatus != noErr || outputProc == nullptr) {
      destroyCallbacks();
      return statusFromContext(context, false, "OUTPUT_CALLBACK_CREATE_FAILED", state, state.metrics);
    }

    for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
      audioStatus = AudioDeviceStart(context.inputs[index].device, inputProcs[index]);
      if (audioStatus != noErr) {
        for (std::size_t stopIndex = 0; stopIndex < index; stopIndex += 1) {
          AudioDeviceStop(context.inputs[stopIndex].device, inputProcs[stopIndex]);
        }
        destroyCallbacks();
        return statusFromContext(context, false, "INPUT_START_FAILED", state, state.metrics);
      }
    }
    audioStatus = AudioDeviceStart(context.output, outputProc);
    if (audioStatus != noErr) {
      for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
        AudioDeviceStop(context.inputs[index].device, inputProcs[index]);
      }
      destroyCallbacks();
      return statusFromContext(context, false, "OUTPUT_START_FAILED", state, state.metrics);
    }

    running = true;
    return statusFromContext(context, true, "", state, state.metrics);
  }

  PersistentMonitorStatus stop() {
    if (running) {
      AudioDeviceStop(context.output, outputProc);
      for (std::size_t index = 0; index < inputProcs.size() && index < context.inputs.size(); index += 1) {
        AudioDeviceStop(context.inputs[index].device, inputProcs[index]);
      }
    }
    destroyCallbacks();
    running = false;
    return statusFromContext(context, false, "", state, state.metrics);
  }

  PersistentMonitorStatus currentStatus() {
    return statusFromContext(context, running, "", state, state.metrics);
  }

  void destroyCallbacks() {
    if (outputProc != nullptr && context.output != kAudioObjectUnknown) {
      AudioDeviceDestroyIOProcID(context.output, outputProc);
      outputProc = nullptr;
    }
    for (std::size_t index = 0; index < inputProcs.size() && index < context.inputs.size(); index += 1) {
      if (inputProcs[index] != nullptr && context.inputs[index].device != kAudioObjectUnknown) {
        AudioDeviceDestroyIOProcID(context.inputs[index].device, inputProcs[index]);
        inputProcs[index] = nullptr;
      }
    }
  }

  DeviceContext context;
  PassthroughState state;
  std::vector<AudioDeviceIOProcID> inputProcs;
  std::vector<InputCallbackClient> inputClients;
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

PersistentMonitorStatus PersistentPassthroughMonitor::status() {
  return impl_->currentStatus();
}

PassthroughMonitorResult monitorPassthrough(const PassthroughMonitorRequest& request) {
  const auto context = prepareDeviceContext(request);
  PassthroughState state;
  if (!context.error.empty()) {
    std::uint32_t inputChannels = 0;
    double inputRate = 0.0;
    for (const auto& input : context.inputs) {
      inputChannels += input.channels;
      if (inputRate == 0.0) inputRate = input.rate;
    }
    return {
      .error = context.error,
      .inputChannels = inputChannels,
      .outputChannels = context.outputChannels,
      .inputSampleRate = inputRate,
      .outputSampleRate = context.outputRate,
    };
  }

  state.outputChannel = request.outputChannel;
  state.mirrorToAllOutputChannels = request.mirrorToAllOutputChannels;
  prepareMonitorGraph(state, request);
  for (auto& source : state.sources) {
    source.ring.samples.assign(static_cast<std::size_t>(request.projectSampleRate) * source.ringChannels, 0.0f);
  }

  std::vector<AudioDeviceIOProcID> inputProcs(state.deviceSourceIndices.size(), nullptr);
  std::vector<InputCallbackClient> inputClients(state.deviceSourceIndices.size());
  AudioDeviceIOProcID outputProc = nullptr;
  for (std::size_t index = 0; index < state.deviceSourceIndices.size(); index += 1) {
    inputClients[index] = InputCallbackClient{.state = &state, .sourceIndex = state.deviceSourceIndices[index]};
    auto status = AudioDeviceCreateIOProcID(
      context.inputs[index].device, inputCallback, &inputClients[index], &inputProcs[index]);
    if (status != noErr || inputProcs[index] == nullptr) {
      for (std::size_t destroyIndex = 0; destroyIndex < index; destroyIndex += 1) {
        AudioDeviceDestroyIOProcID(context.inputs[destroyIndex].device, inputProcs[destroyIndex]);
      }
      return {.error = "INPUT_CALLBACK_CREATE_FAILED"};
    }
  }
  auto status = AudioDeviceCreateIOProcID(context.output, outputCallback, &state, &outputProc);
  if (status != noErr || outputProc == nullptr) {
    for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
      AudioDeviceDestroyIOProcID(context.inputs[index].device, inputProcs[index]);
    }
    return {.error = "OUTPUT_CALLBACK_CREATE_FAILED"};
  }

  for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
    status = AudioDeviceStart(context.inputs[index].device, inputProcs[index]);
    if (status != noErr) {
      for (std::size_t stopIndex = 0; stopIndex < index; stopIndex += 1) {
        AudioDeviceStop(context.inputs[stopIndex].device, inputProcs[stopIndex]);
      }
      AudioDeviceDestroyIOProcID(context.output, outputProc);
      for (std::size_t destroyIndex = 0; destroyIndex < inputProcs.size(); destroyIndex += 1) {
        AudioDeviceDestroyIOProcID(context.inputs[destroyIndex].device, inputProcs[destroyIndex]);
      }
      return {.error = "INPUT_START_FAILED"};
    }
  }
  status = AudioDeviceStart(context.output, outputProc);
  if (status != noErr) {
    AudioDeviceDestroyIOProcID(context.output, outputProc);
    for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
      AudioDeviceStop(context.inputs[index].device, inputProcs[index]);
      AudioDeviceDestroyIOProcID(context.inputs[index].device, inputProcs[index]);
    }
    return {.error = "OUTPUT_START_FAILED"};
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(request.durationMs));

  AudioDeviceStop(context.output, outputProc);
  for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
    AudioDeviceStop(context.inputs[index].device, inputProcs[index]);
  }
  AudioDeviceDestroyIOProcID(context.output, outputProc);
  for (std::size_t index = 0; index < inputProcs.size(); index += 1) {
    AudioDeviceDestroyIOProcID(context.inputs[index].device, inputProcs[index]);
  }
  std::uint32_t inputChannels = 0;
  double inputRate = 0.0;
  for (const auto& input : context.inputs) {
    inputChannels += input.channels;
    if (inputRate == 0.0) inputRate = input.rate;
  }
  int peak = 0;
  int peakLeft = 0;
  int peakRight = 0;
  for (auto& source : state.sources) {
    peak = std::max(peak, source.ring.peakScaled.load(std::memory_order_relaxed));
    peakLeft = std::max(peakLeft, source.ring.peakScaledLeft.load(std::memory_order_relaxed));
    peakRight = std::max(peakRight, source.ring.peakScaledRight.load(std::memory_order_relaxed));
  }
  return {
    .ok = true,
    .inputChannels = inputChannels,
    .outputChannels = context.outputChannels,
    .inputSampleRate = inputRate,
    .outputSampleRate = context.outputRate,
    .inputPeak = static_cast<float>(peak) / 1'000'000.0f,
    .inputPeakLeft = static_cast<float>(peakLeft) / 1'000'000.0f,
    .inputPeakRight = static_cast<float>(peakRight) / 1'000'000.0f,
  };
}

}  // namespace localmixer::platform::macos
