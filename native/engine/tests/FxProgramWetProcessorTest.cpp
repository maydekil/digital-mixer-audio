#include "engine/FxProgramWetProcessor.hpp"
#include "engine/MixerGraph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

namespace {

using localmixer::engine::FxBusId;
using localmixer::engine::FxProgramWetProcessor;
using localmixer::engine::FxSendState;
using localmixer::engine::FxUnitRuntime;
using localmixer::engine::MixerError;
using localmixer::engine::MixerGraph;
using localmixer::engine::SourceBuffer;
using localmixer::engine::StereoOutput;

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
  FxProgramWetProcessor plate{48000.0};
  plate.prepare(4096);
  if (!expect(plate.configure(12), "program 12 should configure Vocal Plate")) return 1;

  std::vector<float> impulse(4096, 0.0f);
  impulse[0] = 1.0f;
  std::vector<float> wetLeft(4096, 0.0f);
  std::vector<float> wetRight(4096, 0.0f);
  plate.process(impulse, wetLeft, wetRight);
  if (!expect(peak(wetLeft) > 0.01f && peak(wetRight) > 0.01f, "Vocal Plate should produce wet tail")) return 1;

  FxProgramWetProcessor stereoDelay{48000.0};
  stereoDelay.prepare(20000);
  if (!expect(stereoDelay.configure(50), "program 50 should configure Stereo 320")) return 1;
  std::vector<float> delayInput(20000, 0.0f);
  delayInput[0] = 1.0f;
  std::vector<float> delayLeft(20000, 0.0f);
  std::vector<float> delayRight(20000, 0.0f);
  stereoDelay.process(delayInput, delayLeft, delayRight);
  if (!expect(peak(delayLeft) > 0.01f && peak(delayRight) > 0.01f, "Stereo 320 should produce delayed wet signal")) return 1;
  if (!expect(!stereoDelay.configure(1000), "invalid program should be rejected")) return 1;

  MixerGraph graph;
  graph.prepare(4096);
  const auto strip = graph.createStrip("FX Source", "#18d6e7");
  if (!expect(strip.error == MixerError::none, "graph strip should create")) return 1;
  graph.setFxSend(strip.id, FxBusId::a, FxSendState{.enabled = true, .gainDb = 0.0f});
  graph.setFxUnit(FxBusId::a, FxUnitRuntime{.enabled = true, .returnDb = 0.0f});
  std::array<SourceBuffer, 1> sources{SourceBuffer{.stripId = strip.id, .samples = impulse, .channels = 1}};
  std::vector<float> mainLeft(4096, 0.0f);
  std::vector<float> mainRight(4096, 0.0f);
  if (!expect(graph.processWithFx(sources, StereoOutput{.left = mainLeft, .right = mainRight}, plate.callback(), {}) == MixerError::none,
              "graph should process FX program callback")) {
    return 1;
  }
  if (!expect(peak(mainLeft) >= 1.0f && graph.fxMeters(FxBusId::a).returnPeakLeft > 0.01f,
              "graph should add factory program wet return to dry signal")) {
    return 1;
  }

  std::cout << "local-mixer-fx-program-wet-processor-tests ok\n";
  return 0;
}
