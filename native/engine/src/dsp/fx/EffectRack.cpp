#include "dsp/fx/EffectRack.hpp"

#include <algorithm>

namespace localmixer::dsp::fx {

EffectRack::EffectRack(std::size_t slotLimit) : slotLimit_(slotLimit) {}

void EffectRack::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  ensureScratch(spec.maximumBlockFrames);
  for (auto& slot : slots_) slot.processor->prepare(spec_);
}

RackActionResult EffectRack::addSlot(RackSlotState state, std::uint64_t expectedRevision, const EffectFactory& factory) {
  if (const auto stale = staleIfNeeded(expectedRevision); stale.error != FxError::none) return stale;
  if (slots_.size() >= slotLimit_) return {.error = FxError::slotLimit, .revision = revision_};
  auto next = slotStates_;
  next.push_back(std::move(state));
  return replaceAll(std::move(next), expectedRevision, factory);
}

RackActionResult EffectRack::removeSlot(std::string_view instanceId, std::uint64_t expectedRevision) {
  if (const auto stale = staleIfNeeded(expectedRevision); stale.error != FxError::none) return stale;
  const auto index = indexOf(instanceId);
  if (!index.has_value()) return {.error = FxError::unknownEffect, .revision = revision_};
  auto next = std::move(slots_);
  next.erase(next.begin() + static_cast<std::ptrdiff_t>(*index));
  publish(std::move(next));
  return {.revision = revision_};
}

RackActionResult EffectRack::moveSlot(
  std::string_view instanceId, std::size_t destinationIndex, std::uint64_t expectedRevision) {
  if (const auto stale = staleIfNeeded(expectedRevision); stale.error != FxError::none) return stale;
  const auto index = indexOf(instanceId);
  if (!index.has_value() || destinationIndex >= slots_.size()) return {.error = FxError::unknownEffect, .revision = revision_};
  auto stateOrder = slotStates_;
  auto state = std::move(stateOrder[*index]);
  stateOrder.erase(stateOrder.begin() + static_cast<std::ptrdiff_t>(*index));
  stateOrder.insert(stateOrder.begin() + static_cast<std::ptrdiff_t>(destinationIndex), std::move(state));
  if (!formatsCompatible(stateOrder)) return {.error = FxError::incompatibleFormat, .revision = revision_};

  auto slot = std::move(slots_[*index]);
  slots_.erase(slots_.begin() + static_cast<std::ptrdiff_t>(*index));
  slots_.insert(slots_.begin() + static_cast<std::ptrdiff_t>(destinationIndex), std::move(slot));
  publish(std::move(slots_));
  return {.revision = revision_};
}

RackActionResult EffectRack::setBypass(std::string_view instanceId, bool bypassed, std::uint64_t expectedRevision) {
  if (const auto stale = staleIfNeeded(expectedRevision); stale.error != FxError::none) return stale;
  const auto index = indexOf(instanceId);
  if (!index.has_value()) return {.error = FxError::unknownEffect, .revision = revision_};
  slots_[*index].state.bypassed = bypassed;
  publish(std::move(slots_));
  return {.revision = revision_};
}

RackActionResult EffectRack::replaceAll(
  std::vector<RackSlotState> states, std::uint64_t expectedRevision, const EffectFactory& factory) {
  if (const auto stale = staleIfNeeded(expectedRevision); stale.error != FxError::none) return stale;
  if (states.size() > slotLimit_) return {.error = FxError::slotLimit, .revision = revision_};
  if (!formatsCompatible(states)) return {.error = FxError::incompatibleFormat, .revision = revision_};
  std::vector<Slot> replacement;
  if (!rebuildSlots(std::move(states), factory, replacement)) return {.error = FxError::factoryFailed, .revision = revision_};
  publish(std::move(replacement));
  return {.revision = revision_};
}

bool EffectRack::storeCompare() {
  compareSnapshot_ = slotStates_;
  return true;
}

RackActionResult EffectRack::recallCompare(std::uint64_t expectedRevision, const EffectFactory& factory) {
  if (compareSnapshot_.empty()) return {.error = FxError::unknownEffect, .revision = revision_};
  return replaceAll(compareSnapshot_, expectedRevision, factory);
}

void EffectRack::process(AudioBlockView& block, const ProcessContext& context) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  if (frames == 0) return;
  for (auto& slot : slots_) {
    if (slot.state.bypassed) continue;
    const auto mix = std::clamp(slot.state.mix, 0.0f, 1.0f);
    std::copy(block.left.begin(), block.left.begin() + static_cast<std::ptrdiff_t>(frames), dryLeft_.begin());
    std::copy(block.right.begin(), block.right.begin() + static_cast<std::ptrdiff_t>(frames), dryRight_.begin());
    slot.processor->process(block, context);
    for (std::size_t index = 0; index < frames; index += 1) {
      block.left[index] = dryLeft_[index] * (1.0f - mix) + block.left[index] * mix;
      block.right[index] = dryRight_[index] * (1.0f - mix) + block.right[index] * mix;
    }
  }
}

std::uint32_t EffectRack::latencySamples() const noexcept {
  std::uint32_t latency = 0;
  for (const auto& slot : slots_) latency += slot.processor->latencySamples();
  return latency;
}

std::uint64_t EffectRack::revision() const {
  return revision_;
}

const std::vector<RackSlotState>& EffectRack::slots() const {
  return slotStates_;
}

RackActionResult EffectRack::staleIfNeeded(std::uint64_t expectedRevision) const {
  if (expectedRevision != revision_) return {.error = FxError::staleRevision, .revision = revision_};
  return {.revision = revision_};
}

std::optional<std::size_t> EffectRack::indexOf(std::string_view instanceId) const {
  const auto it = std::find_if(slots_.begin(), slots_.end(), [&](const auto& slot) {
    return slot.state.instanceId == instanceId;
  });
  if (it == slots_.end()) return std::nullopt;
  return static_cast<std::size_t>(std::distance(slots_.begin(), it));
}

bool EffectRack::formatsCompatible(const std::vector<RackSlotState>& states) const {
  auto current = ChannelFormat::mono;
  for (const auto& state : states) {
    const bool acceptsCurrent = state.inputFormat == current || state.inputFormat == ChannelFormat::monoToStereo;
    if (!acceptsCurrent) return false;
    current = state.outputFormat;
  }
  return true;
}

bool EffectRack::rebuildSlots(
  std::vector<RackSlotState> states, const EffectFactory& factory, std::vector<Slot>& output) const {
  output.clear();
  output.reserve(states.size());
  for (auto& state : states) {
    if (state.instanceId.empty() || state.effectType.empty()) return false;
    auto processor = factory(state.effectType);
    if (!processor) return false;
    processor->prepare(spec_);
    output.push_back(Slot{.state = std::move(state), .processor = std::move(processor)});
  }
  return true;
}

void EffectRack::publish(std::vector<Slot> slots) {
  slots_ = std::move(slots);
  slotStates_.clear();
  slotStates_.reserve(slots_.size());
  for (const auto& slot : slots_) slotStates_.push_back(slot.state);
  revision_ += 1;
}

void EffectRack::ensureScratch(std::size_t frames) {
  dryLeft_.assign(frames, 0.0f);
  dryRight_.assign(frames, 0.0f);
}

}  // namespace localmixer::dsp::fx
