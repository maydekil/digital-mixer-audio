#include "engine/FxSendReturnBus.hpp"

#include "dsp/Gain.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {
namespace {

void updatePeak(float& peak, float sample) {
  peak = std::max(peak, std::fabs(sample));
}

float sendGain(const FxSendState& send) {
  return send.enabled ? localmixer::dsp::decibelsToLinear(std::clamp(send.gainDb, -90.0f, 10.0f)) : 0.0f;
}

}  // namespace

void FxSendReturnBus::prepare(std::uint32_t maximumFrames) {
  sendA_.assign(maximumFrames, 0.0f);
  sendB_.assign(maximumFrames, 0.0f);
  wetLeft_.assign(maximumFrames, 0.0f);
  wetRight_.assign(maximumFrames, 0.0f);
}

void FxSendReturnBus::setUnit(FxBusId bus, FxUnitRuntime unit) noexcept {
  if (bus == FxBusId::a) {
    unitA_ = unit;
  } else {
    unitB_ = unit;
  }
}

void FxSendReturnBus::setChannels(std::vector<FxBusChannel> channels) {
  channels_ = std::move(channels);
}

void FxSendReturnBus::process(FxBusFrame frame, const FxWetProcessor& processorA, const FxWetProcessor& processorB) noexcept {
  const auto frames = std::min(frame.mainLeft.size(), std::min(frame.mainRight.size(), sendA_.size()));
  if (frames == 0 || frame.mainLeft.size() != frame.mainRight.size()) return;
  std::fill(sendA_.begin(), sendA_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  std::fill(sendB_.begin(), sendB_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  metersA_ = {};
  metersB_ = {};

  for (const auto& source : frame.sources) {
    const auto count = std::min(frames, source.samples.size());
    const auto channel = std::find_if(channels_.begin(), channels_.end(), [&](const auto& item) {
      return item.stripId == source.stripId;
    });
    if (channel == channels_.end() || !channel->enabled || channel->mute) continue;
    const auto dryGain = localmixer::dsp::decibelsToLinear(channel->faderDb);
    for (std::size_t index = 0; index < count; index += 1) {
      const auto dry = source.samples[index] * dryGain;
      frame.mainLeft[index] += dry;
      frame.mainRight[index] += dry;
    }
  }

  sumSend(FxBusId::a, frame.sources, std::span<float>(sendA_.data(), frames));
  sumSend(FxBusId::b, frame.sources, std::span<float>(sendB_.data(), frames));
  renderReturn(FxBusId::a, processorA, frame.mainLeft.first(frames), frame.mainRight.first(frames));
  renderReturn(FxBusId::b, processorB, frame.mainLeft.first(frames), frame.mainRight.first(frames));
}

FxBusMeters FxSendReturnBus::meters(FxBusId bus) const noexcept {
  return bus == FxBusId::a ? metersA_ : metersB_;
}

void FxSendReturnBus::sumSend(FxBusId bus, std::span<const FxBusSource> sources, std::span<float> output) noexcept {
  auto& meter = bus == FxBusId::a ? metersA_ : metersB_;
  for (const auto& source : sources) {
    const auto channel = std::find_if(channels_.begin(), channels_.end(), [&](const auto& item) {
      return item.stripId == source.stripId;
    });
    if (channel == channels_.end() || !channel->enabled || channel->mute) continue;
    const auto gain = localmixer::dsp::decibelsToLinear(channel->faderDb) * sendGain(bus == FxBusId::a ? channel->sendA : channel->sendB);
    if (gain == 0.0f) continue;
    const auto count = std::min(source.samples.size(), output.size());
    for (std::size_t index = 0; index < count; index += 1) {
      const auto sample = source.samples[index] * gain;
      output[index] += sample;
      updatePeak(meter.inputPeak, sample);
    }
  }
}

void FxSendReturnBus::renderReturn(FxBusId bus, const FxWetProcessor& processor, std::span<float> mainLeft, std::span<float> mainRight) noexcept {
  const auto& unit = bus == FxBusId::a ? unitA_ : unitB_;
  auto& meter = bus == FxBusId::a ? metersA_ : metersB_;
  const auto frames = mainLeft.size();
  std::fill(wetLeft_.begin(), wetLeft_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  std::fill(wetRight_.begin(), wetRight_.begin() + static_cast<std::ptrdiff_t>(frames), 0.0f);
  if (!unit.enabled || unit.mute || !processor) return;

  processor(bus == FxBusId::a ? std::span<const float>(sendA_.data(), frames) : std::span<const float>(sendB_.data(), frames),
    std::span<float>(wetLeft_.data(), frames),
    std::span<float>(wetRight_.data(), frames));
  const auto returnGain = localmixer::dsp::decibelsToLinear(std::clamp(unit.returnDb, -90.0f, 10.0f));
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto left = wetLeft_[index] * returnGain;
    const auto right = wetRight_[index] * returnGain;
    mainLeft[index] += left;
    mainRight[index] += right;
    updatePeak(meter.returnPeakLeft, left);
    updatePeak(meter.returnPeakRight, right);
  }
}

}  // namespace localmixer::engine
