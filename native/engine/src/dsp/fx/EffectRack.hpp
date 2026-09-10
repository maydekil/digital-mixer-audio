#pragma once

#include "dsp/fx/EffectProcessor.hpp"
#include "dsp/fx/EffectRegistry.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::dsp::fx {

constexpr std::size_t kMaxNativeChannelFxSlots = 8;
constexpr std::size_t kMaxNativeBusFxSlots = 4;

struct RackSlotState {
  std::string instanceId;
  std::string effectType;
  ChannelFormat inputFormat = ChannelFormat::mono;
  ChannelFormat outputFormat = ChannelFormat::mono;
  float mix = 1.0f;
  bool bypassed = false;
};

struct RackActionResult {
  FxError error = FxError::none;
  std::uint64_t revision = 0;
};

using EffectFactory = std::function<std::unique_ptr<EffectProcessor>(std::string_view effectType)>;

class EffectRack {
 public:
  explicit EffectRack(std::size_t slotLimit = kMaxNativeChannelFxSlots);
  void prepare(const ProcessSpec& spec);
  RackActionResult addSlot(RackSlotState state, std::uint64_t expectedRevision, const EffectFactory& factory);
  RackActionResult removeSlot(std::string_view instanceId, std::uint64_t expectedRevision);
  RackActionResult moveSlot(std::string_view instanceId, std::size_t destinationIndex, std::uint64_t expectedRevision);
  RackActionResult setBypass(std::string_view instanceId, bool bypassed, std::uint64_t expectedRevision);
  RackActionResult replaceAll(
    std::vector<RackSlotState> states, std::uint64_t expectedRevision, const EffectFactory& factory);
  bool storeCompare();
  RackActionResult recallCompare(std::uint64_t expectedRevision, const EffectFactory& factory);
  void process(AudioBlockView& block, const ProcessContext& context) noexcept;
  std::uint32_t latencySamples() const noexcept;
  std::uint64_t revision() const;
  const std::vector<RackSlotState>& slots() const;

 private:
  struct Slot {
    RackSlotState state;
    std::unique_ptr<EffectProcessor> processor;
  };

  RackActionResult staleIfNeeded(std::uint64_t expectedRevision) const;
  std::optional<std::size_t> indexOf(std::string_view instanceId) const;
  bool formatsCompatible(const std::vector<RackSlotState>& states) const;
  bool rebuildSlots(std::vector<RackSlotState> states, const EffectFactory& factory, std::vector<Slot>& output) const;
  void publish(std::vector<Slot> slots);
  void ensureScratch(std::size_t frames);

  std::size_t slotLimit_;
  ProcessSpec spec_;
  std::uint64_t revision_ = 0;
  std::vector<Slot> slots_;
  std::vector<RackSlotState> slotStates_;
  std::vector<RackSlotState> compareSnapshot_;
  std::vector<float> dryLeft_;
  std::vector<float> dryRight_;
};

}  // namespace localmixer::dsp::fx
