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

}  // namespace

MixerGraph::MixerGraph() : controlQueue_(128) {
  strips_.reserve(kMaxMixerStrips);
  meters_.reserve(kMaxMixerStrips);
  runtimes_.reserve(kMaxMixerStrips);
}

CreateStripResult MixerGraph::createStrip(std::string name, std::string color) {
  if (strips_.size() >= kMaxMixerStrips) return {.error = MixerError::graphFull};
  const StripId id{nextId_++};
  strips_.push_back(StripConfig{.id = id, .name = std::move(name), .color = std::move(color)});
  meters_.push_back({});
  runtimes_.push_back({});
  return {.id = id};
}

MixerError MixerGraph::removeStrip(StripId id) {
  const auto index = indexOf(id);
  if (!index.has_value()) return MixerError::staleStripId;
  strips_.erase(strips_.begin() + static_cast<std::ptrdiff_t>(*index));
  meters_.erase(meters_.begin() + static_cast<std::ptrdiff_t>(*index));
  runtimes_.erase(runtimes_.begin() + static_cast<std::ptrdiff_t>(*index));
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
  if (output.left.size() != output.right.size()) return MixerError::invalidBuffer;
  std::fill(output.left.begin(), output.left.end(), 0.0f);
  std::fill(output.right.begin(), output.right.end(), 0.0f);
  std::fill(meters_.begin(), meters_.end(), StripMeters{});

  const bool soloActive = anySolo();
  for (const auto& source : sources) {
    const auto index = indexOf(source.stripId);
    if (!index.has_value()) return MixerError::staleStripId;
    if (source.channels == 0 || source.samples.empty()) return MixerError::invalidSource;

    const auto& strip = strips_[*index];
    auto& meter = meters_[*index];
    auto& runtime = runtimes_[*index];
    const bool active = strip.enabled && !strip.mute && (!soloActive || strip.solo);

    for (std::uint32_t frame = 0; frame < output.left.size(); frame += 1) {
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

std::size_t MixerGraph::pendingControlCount() const {
  return controlQueue_.size();
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
