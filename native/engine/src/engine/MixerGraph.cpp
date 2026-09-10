#include "engine/MixerGraph.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {
namespace {

float clamp(float value, float min, float max) {
  return std::max(min, std::min(max, std::isfinite(value) ? value : min));
}

float sampleAt(const SourceBuffer& source, std::uint32_t frame, std::uint32_t channel) {
  if (source.channels == 0) return 0.0f;
  const auto index = static_cast<std::size_t>(frame) * source.channels + channel;
  if (index >= source.samples.size()) return 0.0f;
  return source.samples[index];
}

void updatePeak(float& peak, float sample) {
  peak = std::max(peak, std::fabs(sample));
}

float sendGain(const FxSendState& send) {
  return send.enabled ? dsp::decibelsToLinear(clamp(send.gainDb, -90.0f, 10.0f)) : 0.0f;
}

}  // namespace

MixerGraph::MixerGraph() : controlQueue_(128) {
  strips_.reserve(kMaxMixerStrips);
  meters_.reserve(kMaxMixerStrips);
  runtimes_.reserve(kMaxMixerStrips);
  processors_.reserve(kMaxMixerStrips);
  prepare(kDefaultMixerMaxBlockFrames);
}

CreateStripResult MixerGraph::createStrip(std::string name, std::string color) {
  if (strips_.size() >= kMaxMixerStrips) return {.error = MixerError::graphFull};
  const StripId id{nextId_++};
  strips_.push_back(StripConfig{.id = id, .name = std::move(name), .color = std::move(color)});
  meters_.push_back({});
  runtimes_.push_back({});
  processors_.push_back({});
  return {.id = id};
}

MixerError MixerGraph::removeStrip(StripId id) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_.erase(strips_.begin() + static_cast<std::ptrdiff_t>(*index));
  meters_.erase(meters_.begin() + static_cast<std::ptrdiff_t>(*index));
  runtimes_.erase(runtimes_.begin() + static_cast<std::ptrdiff_t>(*index));
  processors_.erase(processors_.begin() + static_cast<std::ptrdiff_t>(*index));
  return MixerError::none;
}

MixerError MixerGraph::renameStrip(StripId id, std::string name) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].name = std::move(name);
  return MixerError::none;
}

MixerError MixerGraph::setColor(StripId id, std::string color) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].color = std::move(color);
  return MixerError::none;
}

MixerError MixerGraph::setSourceUid(StripId id, std::string sourceUid) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].sourceUid = std::move(sourceUid);
  return MixerError::none;
}

MixerError MixerGraph::setAssignment(StripId id, SourceAssignment assignment, std::uint32_t inputChannel, bool stereoLinked) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].assignment = assignment;
  strips_[*index].inputChannel = inputChannel;
  strips_[*index].stereoLinked = stereoLinked;
  return MixerError::none;
}

MixerError MixerGraph::setLevel(StripId id, float trimDb, float faderDb, float pan) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  return setLevelAt(*index, trimDb, faderDb, pan, 0);
}

MixerError MixerGraph::setMute(StripId id, bool mute) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].mute = mute;
  return MixerError::none;
}

MixerError MixerGraph::setSolo(StripId id, bool solo) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].solo = solo;
  return MixerError::none;
}

MixerError MixerGraph::setEnabled(StripId id, bool enabled) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].enabled = enabled;
  return MixerError::none;
}

MixerError MixerGraph::setInputMonitoring(StripId id, bool enabled) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].inputMonitoring = enabled;
  return MixerError::none;
}

MixerError MixerGraph::setProcessors(StripId id, ChannelProcessorConfig config) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_[*index].processors = config;
  processors_[*index].configure(config);
  return MixerError::none;
}

MixerError MixerGraph::setFxSend(StripId id, FxBusId bus, FxSendState send) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  if (bus == FxBusId::a) {
    strips_[*index].sendA = send;
  } else {
    strips_[*index].sendB = send;
  }
  return MixerError::none;
}

void MixerGraph::prepare(std::uint32_t maximumFrames) {
  maximumFrames_ = std::max<std::uint32_t>(1, maximumFrames);
  sendA_.assign(maximumFrames_, 0.0f);
  sendB_.assign(maximumFrames_, 0.0f);
  wetLeft_.assign(maximumFrames_, 0.0f);
  wetRight_.assign(maximumFrames_, 0.0f);
}

void MixerGraph::setFxUnit(FxBusId bus, FxUnitRuntime unit) noexcept {
  if (bus == FxBusId::a) {
    unitA_ = unit;
  } else {
    unitB_ = unit;
  }
}

MixerError MixerGraph::enqueueControl(MixerCommand command) {
  return controlQueue_.push(command);
}

MixerError MixerGraph::applyQueuedControls(std::uint32_t rampFrames) {
  while (const auto command = controlQueue_.pop()) {
    const auto index = indexOf(command->stripId);
    if (!index.has_value()) return MixerError::staleStripId;

    switch (command->type) {
      case MixerCommandType::setLevel:
        if (const auto error = setLevelAt(*index, command->trimDb, command->faderDb, command->pan, rampFrames);
            error != MixerError::none) {
          return error;
        }
        break;
      case MixerCommandType::setMute:
        strips_[*index].mute = command->boolValue;
        break;
      case MixerCommandType::setSolo:
        strips_[*index].solo = command->boolValue;
        break;
      case MixerCommandType::setEnabled:
        strips_[*index].enabled = command->boolValue;
        break;
      case MixerCommandType::setInputMonitoring:
        strips_[*index].inputMonitoring = command->boolValue;
        break;
    }
  }
  return MixerError::none;
}

MixerError MixerGraph::process(std::span<const SourceBuffer> sources, StereoOutput output) {
  return processWithFx(sources, output, {}, {});
}

MixerError MixerGraph::processWithFx(
  std::span<const SourceBuffer> sources,
  StereoOutput output,
  const FxWetProcessor& processorA,
  const FxWetProcessor& processorB
) {
  if (output.left.size() != output.right.size()) return MixerError::invalidBuffer;
  if (output.left.size() > maximumFrames_) return MixerError::invalidBuffer;
  const auto frames = output.left.size();
  std::fill(output.left.begin(), output.left.end(), 0.0f);
  std::fill(output.right.begin(), output.right.end(), 0.0f);
  std::fill(sendA_.begin(), sendA_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  std::fill(sendB_.begin(), sendB_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  std::fill(meters_.begin(), meters_.end(), StripMeters{});
  fxMetersA_ = {};
  fxMetersB_ = {};

  const bool soloActive = anySolo();
  for (const auto& source : sources) {
    const auto index = indexOf(source.stripId);
    if (!index.has_value()) return MixerError::staleStripId;
    if (source.channels == 0 || source.samples.empty()) return MixerError::invalidSource;

    const auto& strip = strips_[*index];
    auto& meter = meters_[*index];
    auto& runtime = runtimes_[*index];
    auto& processor = processors_[*index];
    const bool active = strip.enabled && !strip.mute && (!soloActive || strip.solo);

    for (std::uint32_t frame = 0; frame < frames; frame += 1) {
      float left = 0.0f;
      float right = 0.0f;
      if (strip.assignment == SourceAssignment::stereo && source.channels > strip.inputChannel + 1) {
        left = sampleAt(source, frame, strip.inputChannel);
        right = sampleAt(source, frame, strip.inputChannel + 1);
      } else if (source.channels > strip.inputChannel) {
        left = sampleAt(source, frame, strip.inputChannel);
        right = left;
      }

      updatePeak(meter.inputPeak, left);
      updatePeak(meter.inputPeak, right);
      if (!active) {
        advanceRuntime(runtime);
        continue;
      }

      processor.processFrame(left, right);
      const auto sendMono = (left + right) * 0.5f;
      const auto fxAGain = sendGain(strip.sendA);
      const auto fxBGain = sendGain(strip.sendB);
      if (fxAGain > 0.0f) {
        const auto sample = sendMono * fxAGain;
        sendA_[frame] += sample;
        updatePeak(fxMetersA_.inputPeak, sample);
      }
      if (fxBGain > 0.0f) {
        const auto sample = sendMono * fxBGain;
        sendB_[frame] += sample;
        updatePeak(fxMetersB_.inputPeak, sample);
      }
      const auto gain = runtime.currentGain;
      const auto pan = runtime.currentPan;
      const auto leftPan = strip.assignment == SourceAssignment::mono ? std::min(1.0f, 1.0f - pan) : 1.0f;
      const auto rightPan = strip.assignment == SourceAssignment::mono ? std::min(1.0f, 1.0f + pan) : 1.0f;
      left *= gain * leftPan;
      right *= gain * rightPan;
      output.left[frame] += left;
      output.right[frame] += right;
      updatePeak(meter.outputPeakLeft, left);
      updatePeak(meter.outputPeakRight, right);
      advanceRuntime(runtime);
    }
  }

  renderFxReturn(FxBusId::a, processorA, output.left.first(frames), output.right.first(frames));
  renderFxReturn(FxBusId::b, processorB, output.left.first(frames), output.right.first(frames));
  return MixerError::none;
}

std::optional<StripConfig> MixerGraph::strip(StripId id) const {
  const auto index = indexOf(id);
  if (!index.has_value()) return std::nullopt;
  return strips_[*index];
}

std::optional<StripMeters> MixerGraph::meters(StripId id) const {
  const auto index = indexOf(id);
  if (!index.has_value()) return std::nullopt;
  return meters_[*index];
}

FxBusMeters MixerGraph::fxMeters(FxBusId bus) const noexcept {
  return bus == FxBusId::a ? fxMetersA_ : fxMetersB_;
}

std::size_t MixerGraph::pendingControlCount() const {
  return controlQueue_.size();
}

void MixerGraph::renderFxReturn(
  FxBusId bus,
  const FxWetProcessor& processor,
  std::span<float> mainLeft,
  std::span<float> mainRight
) noexcept {
  const auto frames = mainLeft.size();
  auto& meter = bus == FxBusId::a ? fxMetersA_ : fxMetersB_;
  const auto& unit = bus == FxBusId::a ? unitA_ : unitB_;
  std::fill(wetLeft_.begin(), wetLeft_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  std::fill(wetRight_.begin(), wetRight_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  if (!unit.enabled || unit.mute || !processor) return;

  const auto input = bus == FxBusId::a ? std::span<const float>(sendA_.data(), frames)
                                      : std::span<const float>(sendB_.data(), frames);
  processor(input, std::span<float>(wetLeft_.data(), frames), std::span<float>(wetRight_.data(), frames));
  const auto returnGain = dsp::decibelsToLinear(clamp(unit.returnDb, -90.0f, 10.0f));
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto left = wetLeft_[index] * returnGain;
    const auto right = wetRight_[index] * returnGain;
    mainLeft[index] += left;
    mainRight[index] += right;
    updatePeak(meter.returnPeakLeft, left);
    updatePeak(meter.returnPeakRight, right);
  }
}

std::size_t MixerGraph::stripCount() const {
  return strips_.size();
}

MixerError MixerGraph::setLevelAt(std::size_t index, float trimDb, float faderDb, float pan, std::uint32_t rampFrames) {
  strips_[index].trimDb = clamp(trimDb, -24.0f, 24.0f);
  strips_[index].faderDb = clamp(faderDb, -60.0f, 10.0f);
  strips_[index].pan = clamp(pan, -1.0f, 1.0f);

  auto& runtime = runtimes_[index];
  runtime.targetGain = dsp::decibelsToLinear(strips_[index].trimDb + strips_[index].faderDb);
  runtime.targetPan = strips_[index].pan;
  if (rampFrames == 0) {
    runtime.currentGain = runtime.targetGain;
    runtime.currentPan = runtime.targetPan;
    runtime.gainStep = 0.0f;
    runtime.panStep = 0.0f;
    runtime.remainingRampFrames = 0;
    return MixerError::none;
  }

  runtime.gainStep = (runtime.targetGain - runtime.currentGain) / static_cast<float>(rampFrames);
  runtime.panStep = (runtime.targetPan - runtime.currentPan) / static_cast<float>(rampFrames);
  runtime.remainingRampFrames = rampFrames;
  return MixerError::none;
}

std::optional<std::size_t> MixerGraph::indexOf(StripId id) const {
  const auto it = std::find_if(strips_.begin(), strips_.end(), [&](const auto& strip) {
    return strip.id == id;
  });
  if (it == strips_.end()) return std::nullopt;
  return static_cast<std::size_t>(std::distance(strips_.begin(), it));
}

bool MixerGraph::anySolo() const {
  return std::any_of(strips_.begin(), strips_.end(), [](const auto& strip) {
    return strip.enabled && strip.solo;
  });
}

void MixerGraph::advanceRuntime(StripRuntime& runtime) {
  if (runtime.remainingRampFrames == 0) return;
  runtime.currentGain += runtime.gainStep;
  runtime.currentPan += runtime.panStep;
  runtime.remainingRampFrames -= 1;
  if (runtime.remainingRampFrames == 0) {
    runtime.currentGain = runtime.targetGain;
    runtime.currentPan = runtime.targetPan;
    runtime.gainStep = 0.0f;
    runtime.panStep = 0.0f;
  }
}

const char* mixerErrorName(MixerError error) {
  switch (error) {
    case MixerError::none: return "NONE";
    case MixerError::graphFull: return "GRAPH_FULL";
    case MixerError::staleStripId: return "STALE_STRIP_ID";
    case MixerError::invalidSource: return "INVALID_SOURCE";
    case MixerError::invalidBuffer: return "INVALID_BUFFER";
    case MixerError::queueFull: return "QUEUE_FULL";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::engine
