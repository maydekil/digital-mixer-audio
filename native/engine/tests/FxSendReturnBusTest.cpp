#include "engine/FxSendReturnBus.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

namespace {

using localmixer::engine::FxBusChannel;
using localmixer::engine::FxBusFrame;
using localmixer::engine::FxBusId;
using localmixer::engine::FxBusSource;
using localmixer::engine::FxSendReturnBus;
using localmixer::engine::FxSendState;
using localmixer::engine::FxUnitRuntime;
using localmixer::engine::StripId;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

void identityWet(std::span<const float> input, std::span<float> left, std::span<float> right) {
  for (std::size_t index = 0; index < input.size(); index += 1) {
    left[index] = input[index];
    right[index] = input[index];
  }
}

void doubleWet(std::span<const float> input, std::span<float> left, std::span<float> right) {
  for (std::size_t index = 0; index < input.size(); index += 1) {
    left[index] = input[index] * 2.0f;
    right[index] = input[index] * 2.0f;
  }
}

}  // namespace

int main() {
  FxSendReturnBus bus;
  bus.prepare(4);
  const StripId voice{1};
  const StripId music{2};
  bus.setChannels({
    FxBusChannel{.stripId = voice, .sendA = FxSendState{.enabled = true, .gainDb = 0.0f}, .sendB = FxSendState{.enabled = true, .gainDb = 0.0f}},
    FxBusChannel{.stripId = music, .sendA = FxSendState{.enabled = false, .gainDb = 0.0f}, .sendB = FxSendState{.enabled = false, .gainDb = 0.0f}},
  });
  bus.setUnit(FxBusId::a, FxUnitRuntime{.enabled = true, .returnDb = 0.0f});
  bus.setUnit(FxBusId::b, FxUnitRuntime{.enabled = false, .returnDb = 0.0f});

  std::array<float, 4> voiceSamples{1.0f, 1.0f, 1.0f, 1.0f};
  std::array<float, 4> musicSamples{0.5f, 0.5f, 0.5f, 0.5f};
  std::array<FxBusSource, 2> sources{FxBusSource{voice, voiceSamples}, FxBusSource{music, musicSamples}};
  std::array<float, 4> left{};
  std::array<float, 4> right{};

  bus.process(FxBusFrame{.sources = sources, .mainLeft = left, .mainRight = right}, identityWet, doubleWet);
  if (!near(left[0], 2.5f) || !near(right[0], 2.5f)) {
    std::cerr << "FX A should add only voice wet return while music send stays off\n";
    return 1;
  }
  if (!near(bus.meters(FxBusId::a).inputPeak, 1.0f) || !near(bus.meters(FxBusId::b).returnPeakLeft, 0.0f)) {
    std::cerr << "FX meters should reflect independent A/B input and disabled return\n";
    return 1;
  }

  bus.setUnit(FxBusId::a, FxUnitRuntime{.enabled = false, .returnDb = 0.0f});
  std::fill(left.begin(), left.end(), 0.0f);
  std::fill(right.begin(), right.end(), 0.0f);
  bus.process(FxBusFrame{.sources = sources, .mainLeft = left, .mainRight = right}, identityWet, doubleWet);
  if (!near(left[0], 1.5f) || !near(right[0], 1.5f)) {
    std::cerr << "FX unit OFF should silence wet return without dry leak or dry loss\n";
    return 1;
  }

  bus.setUnit(FxBusId::b, FxUnitRuntime{.enabled = true, .returnDb = 0.0f});
  std::fill(left.begin(), left.end(), 0.0f);
  std::fill(right.begin(), right.end(), 0.0f);
  bus.process(FxBusFrame{.sources = sources, .mainLeft = left, .mainRight = right}, identityWet, doubleWet);
  if (!near(left[0], 3.5f) || !near(bus.meters(FxBusId::b).returnPeakLeft, 2.0f)) {
    std::cerr << "FX B send should be independent from FX A unit state\n";
    return 1;
  }

  bus.setChannels({
    FxBusChannel{.stripId = voice, .mute = true, .sendA = FxSendState{.enabled = true, .gainDb = 0.0f}},
  });
  std::fill(left.begin(), left.end(), 0.0f);
  std::fill(right.begin(), right.end(), 0.0f);
  bus.process(FxBusFrame{.sources = sources, .mainLeft = left, .mainRight = right}, identityWet, doubleWet);
  if (!near(left[0], 0.0f) || !near(bus.meters(FxBusId::a).inputPeak, 0.0f)) {
    std::cerr << "muted channel should feed neither main nor FX sends\n";
    return 1;
  }

  std::cout << "local-mixer-fx-send-return-bus-tests ok\n";
  return 0;
}
