#include "dsp/fx/EffectRack.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <memory>

namespace {

using localmixer::dsp::fx::AudioBlockView;
using localmixer::dsp::fx::EffectProcessor;
using localmixer::dsp::fx::EffectRack;
using localmixer::dsp::fx::FxError;
using localmixer::dsp::fx::ChannelFormat;
using localmixer::dsp::fx::ProcessContext;
using localmixer::dsp::fx::ProcessSpec;
using localmixer::dsp::fx::RackSlotState;

class TestGain final : public EffectProcessor {
 public:
  explicit TestGain(float gain) : gain_(gain) {}
  void prepare(const ProcessSpec&) override {}
  void reset() noexcept override {}
  void process(AudioBlockView& block, const ProcessContext&) noexcept override {
    for (auto& sample : block.left) sample *= gain_;
    for (auto& sample : block.right) sample *= gain_;
  }
  void applyRealtimeParameter(localmixer::dsp::fx::ParameterId, float) noexcept override {}
  std::uint32_t latencySamples() const noexcept override { return 0; }
  std::uint64_t maximumTailSamples() const noexcept override { return 0; }

 private:
  float gain_;
};

class TestLatency final : public EffectProcessor {
 public:
  void prepare(const ProcessSpec&) override {}
  void reset() noexcept override {}
  void process(AudioBlockView&, const ProcessContext&) noexcept override {}
  void applyRealtimeParameter(localmixer::dsp::fx::ParameterId, float) noexcept override {}
  std::uint32_t latencySamples() const noexcept override { return 1024; }
  std::uint64_t maximumTailSamples() const noexcept override { return 2048; }
};

std::unique_ptr<EffectProcessor> factory(std::string_view type) {
  if (type == "test_gain_2x") return std::make_unique<TestGain>(2.0f);
  if (type == "test_latency") return std::make_unique<TestLatency>();
  return nullptr;
}

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  EffectRack rack(2);
  rack.prepare(ProcessSpec{.maximumBlockFrames = 4, .channels = 2});

  auto result = rack.addSlot(RackSlotState{.instanceId = "gain", .effectType = "test_gain_2x", .mix = 0.5f}, 0, factory);
  if (result.error != FxError::none || result.revision != 1 || rack.slots().size() != 1) {
    std::cerr << "add slot should publish revision 1\n";
    return 1;
  }

  std::array<float, 4> left{1.0f, 1.0f, 1.0f, 1.0f};
  std::array<float, 4> right{1.0f, 1.0f, 1.0f, 1.0f};
  AudioBlockView block{.left = left, .right = right};
  rack.process(block, ProcessContext{});
  if (!near(left[0], 1.5f) || !near(right[0], 1.5f)) {
    std::cerr << "linear 50 percent mix should not use equal-power gain\n";
    return 1;
  }

  result = rack.setBypass("gain", true, rack.revision());
  if (result.error != FxError::none) return 1;
  for (int index = 0; index < 100; index += 1) {
    result = rack.setBypass("gain", index % 2 == 0, rack.revision());
    if (result.error != FxError::none) {
      std::cerr << "rapid bypass changes should be revision-safe\n";
      return 1;
    }
  }
  result = rack.setBypass("gain", true, rack.revision());
  if (result.error != FxError::none) return 1;
  left.fill(1.0f);
  right.fill(1.0f);
  rack.process(block, ProcessContext{});
  if (!near(left[0], 1.0f) || !near(right[0], 1.0f)) {
    std::cerr << "bypassed slot should leave audio unchanged\n";
    return 1;
  }

  result = rack.addSlot(RackSlotState{.instanceId = "latency", .effectType = "test_latency"}, rack.revision(), factory);
  if (result.error != FxError::none || rack.latencySamples() != 1024) {
    std::cerr << "rack should report injected processor latency\n";
    return 1;
  }

  const auto beforeFailureRevision = rack.revision();
  result = rack.replaceAll({RackSlotState{.instanceId = "bad", .effectType = "missing"}}, beforeFailureRevision, factory);
  if (result.error != FxError::factoryFailed || rack.revision() != beforeFailureRevision || rack.slots().size() != 2) {
    std::cerr << "factory failure should preserve old rack all-or-nothing\n";
    return 1;
  }

  result = rack.moveSlot("latency", 0, rack.revision());
  if (result.error != FxError::none || rack.slots()[0].instanceId != "latency") {
    std::cerr << "move should reorder slots with a revision bump\n";
    return 1;
  }

  if (!rack.storeCompare()) return 1;
  result = rack.removeSlot("gain", rack.revision());
  if (result.error != FxError::none || rack.slots().size() != 1) return 1;
  result = rack.recallCompare(rack.revision(), factory);
  if (result.error != FxError::none || rack.slots().size() != 2) {
    std::cerr << "A/B compare recall should restore stored rack snapshot\n";
    return 1;
  }

  result = rack.addSlot(RackSlotState{.instanceId = "overflow", .effectType = "test_gain_2x"}, 0, factory);
  if (result.error != FxError::staleRevision) {
    std::cerr << "stale revision should be rejected before mutation\n";
    return 1;
  }
  result = rack.addSlot(RackSlotState{.instanceId = "overflow", .effectType = "test_gain_2x"}, rack.revision(), factory);
  if (result.error != FxError::slotLimit) {
    std::cerr << "rack should enforce native slot limit\n";
    return 1;
  }

  EffectRack formatRack(3);
  formatRack.prepare(ProcessSpec{.maximumBlockFrames = 4, .channels = 2});
  result = formatRack.replaceAll({
    RackSlotState{
      .instanceId = "widener",
      .effectType = "test_gain_2x",
      .inputFormat = ChannelFormat::monoToStereo,
      .outputFormat = ChannelFormat::stereo,
    },
    RackSlotState{
      .instanceId = "stereo",
      .effectType = "test_gain_2x",
      .inputFormat = ChannelFormat::stereo,
      .outputFormat = ChannelFormat::stereo,
    },
    RackSlotState{
      .instanceId = "mono-only",
      .effectType = "test_gain_2x",
      .inputFormat = ChannelFormat::mono,
      .outputFormat = ChannelFormat::mono,
    },
  }, 0, factory);
  if (result.error != FxError::incompatibleFormat || formatRack.revision() != 0) {
    std::cerr << "replace should reject mono-only processor after stereo widening before publish\n";
    return 1;
  }

  result = formatRack.replaceAll({
    RackSlotState{
      .instanceId = "mono-only",
      .effectType = "test_gain_2x",
      .inputFormat = ChannelFormat::mono,
      .outputFormat = ChannelFormat::mono,
    },
    RackSlotState{
      .instanceId = "widener",
      .effectType = "test_gain_2x",
      .inputFormat = ChannelFormat::monoToStereo,
      .outputFormat = ChannelFormat::stereo,
    },
  }, 0, factory);
  if (result.error != FxError::none) return 1;
  result = formatRack.moveSlot("mono-only", 1, formatRack.revision());
  if (result.error != FxError::incompatibleFormat || formatRack.slots()[0].instanceId != "mono-only") {
    std::cerr << "move should reject incompatible format reorder without changing active rack\n";
    return 1;
  }

  std::cout << "local-mixer-effect-rack-tests ok\n";
  return 0;
}
