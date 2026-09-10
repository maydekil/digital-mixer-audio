#include "engine/MixerRenderRuntime.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

namespace {

float peak(std::span<const float> samples) {
  float value = 0.0f;
  for (const auto sample : samples) value = std::max(value, std::fabs(sample));
  return value;
}

bool expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << "\n";
  return false;
}

}  // namespace

int main() {
  localmixer::engine::MixerRenderRuntime runtime{48000.0};
  runtime.prepare(4096);
  if (!expect(runtime.fxProgramId(localmixer::engine::FxBusId::a) == 12, "FX A should default to Vocal Plate")) return 1;
  if (!expect(runtime.fxProgramId(localmixer::engine::FxBusId::b) == 50, "FX B should default to Stereo 320")) return 1;
  if (!expect(!runtime.setFxProgram(localmixer::engine::FxBusId::a, 1000), "invalid FX program should reject")) return 1;
  if (!expect(runtime.fxProgramId(localmixer::engine::FxBusId::a) == 12, "invalid FX program should not replace active program")) return 1;

  auto& graph = runtime.graph();
  const auto strip = graph.createStrip("Voice", "#18d6e7");
  if (!expect(strip.error == localmixer::engine::MixerError::none, "strip should create")) return 1;
  graph.setFxSend(strip.id, localmixer::engine::FxBusId::a, localmixer::engine::FxSendState{.enabled = true, .gainDb = 0.0f});
  graph.setFxUnit(localmixer::engine::FxBusId::a, localmixer::engine::FxUnitRuntime{.enabled = true, .returnDb = 0.0f});

  std::vector<float> input(4096, 0.0f);
  input[0] = 1.0f;
  std::vector<float> left(4096, 0.0f);
  std::vector<float> right(4096, 0.0f);
  const std::array<localmixer::engine::SourceBuffer, 1> sources{
    localmixer::engine::SourceBuffer{.stripId = strip.id, .samples = input, .channels = 1}
  };
  if (!expect(runtime.process(sources, localmixer::engine::StereoOutput{.left = left, .right = right}) ==
                localmixer::engine::MixerError::none,
              "runtime should process graph with FX program callbacks")) {
    return 1;
  }
  if (!expect(peak(left) >= 1.0f && graph.fxMeters(localmixer::engine::FxBusId::a).returnPeakLeft > 0.01f,
              "runtime should add FX A factory wet return")) {
    return 1;
  }

  std::cout << "local-mixer-render-runtime-tests ok\n";
  return 0;
}
