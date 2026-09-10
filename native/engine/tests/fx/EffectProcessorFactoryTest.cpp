#include "dsp/fx/EffectProcessorFactory.hpp"
#include "dsp/fx/EffectRegistry.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

bool expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << "\n";
  return false;
}

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  constexpr std::array<std::string_view, 10> constructible{
    "reverb",
    "delay",
    "chorus",
    "doubler",
    "pitch_correct",
    "harmony",
    "saturation",
    "flanger",
    "phaser",
    "vocoder",
  };

  const auto factory = localmixer::dsp::fx::nativeEffectFactory();
  for (const auto effectType : constructible) {
    auto processor = factory(effectType);
    if (!expect(processor != nullptr, "constructible native effect should have a production factory")) return 1;
    processor->prepare(localmixer::dsp::fx::ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = 64, .channels = 2});
    std::array<float, 64> left{};
    std::array<float, 64> right{};
    left[0] = 0.25f;
    right[0] = 0.25f;
    auto block = localmixer::dsp::fx::AudioBlockView{.left = left, .right = right};
    processor->process(block, localmixer::dsp::fx::ProcessContext{.sampleRate = 48000.0});
  }

  if (!expect(factory("pitch_shift") == nullptr, "pitch_shift wrapper should remain explicit pending work")) return 1;
  if (!expect(factory("formant_shift") == nullptr, "formant_shift wrapper should remain explicit pending work")) return 1;

  localmixer::dsp::fx::EffectRack rack(2);
  rack.prepare(localmixer::dsp::fx::ProcessSpec{.sampleRate = 48000.0, .maximumBlockFrames = 16, .channels = 2});
  auto result = rack.replaceAll({
    localmixer::dsp::fx::RackSlotState{
      .instanceId = "drive",
      .effectType = "saturation",
      .mix = 1.0f,
    },
  }, 0, factory);
  if (!expect(result.error == localmixer::dsp::fx::FxError::none, "rack should accept production factory slots")) return 1;

  std::array<float, 16> left{};
  std::array<float, 16> right{};
  left.fill(0.2f);
  right.fill(0.2f);
  auto block = localmixer::dsp::fx::AudioBlockView{.left = left, .right = right};
  rack.process(block, localmixer::dsp::fx::ProcessContext{.sampleRate = 48000.0});
  if (!expect(!near(left[0], 0.2f), "production rack processor should affect audio")) return 1;

  std::cout << "local-mixer-effect-processor-factory-tests ok\n";
  return 0;
}
