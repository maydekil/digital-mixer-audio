#include "engine/LiveInputBridge.hpp"

#include <chrono>
#include <cmath>
#include <iostream>
#include <numbers>
#include <thread>

namespace {
bool check(bool condition, const char* message) {
  if (!condition) std::cerr << message << '\n';
  return condition;
}

bool simulateClocks(double inputRate, double outputRate, double drift, std::uint32_t channels) {
  localmixer::engine::LiveInputBridge bridge(inputRate, outputRate, channels);
  if (!bridge.valid()) return false;
  double inputFraction = 0;
  std::uint64_t inputFrame = 0;
  std::uint64_t measuredFrames = 0;
  int crossings = 0;
  float previous = 0;
  bool stereoCorrect = true;
  const auto actualInputRate = inputRate * (1.0 + drift);
  const auto blocks = static_cast<std::uint32_t>(60 * outputRate / 256);
  for (std::uint32_t block = 0; block < blocks; ++block) {
    inputFraction += 256 * actualInputRate / outputRate;
    while (inputFraction >= 1) {
      const auto value = static_cast<float>(0.25 * std::sin(2 * std::numbers::pi * 1000 * inputFrame / actualInputRate));
      bridge.push(value, -value);
      ++inputFrame;
      inputFraction -= 1;
    }
    while (bridge.processAvailable()) {}
    for (int frame = 0; frame < 256; ++frame) {
      float left = 0, right = 0;
      const auto available = bridge.pop(left, right);
      stereoCorrect &= std::isfinite(left) && std::isfinite(right) &&
        std::fabs(channels == 2 ? left + right : left - right) < 0.0001f;
      if (block > outputRate / 256 && available) {
        if (previous <= 0 && left > 0) ++crossings;
        previous = left;
        ++measuredFrames;
      }
    }
  }
  const auto status = bridge.status();
  const auto frequency = measuredFrames > 0 ? crossings * outputRate / measuredFrames : 0;
  const auto nominal = outputRate / inputRate;
  std::cout << inputRate << " -> " << outputRate << " drift=" << drift << " ratio=" << status.ratio
    << " queued=" << status.inputQueued << "/" << status.outputQueued << " underruns=" << status.outputUnderruns
    << " frequency=" << frequency << '\n';
  return check(!status.failed && status.inputDropped == 0 && status.outputUnderruns == 0,
      "independent clock simulation must not overflow or underrun after priming") &&
    check(status.inputQueued < inputRate * 0.1 && status.outputQueued < outputRate * 0.05,
      "clock drift must not accumulate an unbounded backlog") &&
    check(std::fabs(status.ratio / (nominal / (1 + drift)) - 1.0) < 0.0005,
      "adaptive ratio must track source clock drift") &&
    check(std::fabs(frequency - 1000) < 2, "rate bridge must preserve source pitch") &&
    check(stereoCorrect, "rate bridge must preserve mono/stereo channel separation");
}
}

int main() {
  bool ok = true;
  localmixer::engine::AudioFrameRing ring(4, 2);
  for (int frame = 0; frame < 4; ++frame) ok &= ring.push(frame, -frame);
  ok &= check(!ring.push(99, 99), "full SPSC ring must reject new frames without overwriting unread data");
  for (int frame = 0; frame < 4; ++frame) {
    float left = 0, right = 0;
    ok &= ring.pop(left, right) && left == frame && right == -frame;
  }
  float left = 1, right = 1;
  ok &= check(!ring.pop(left, right) && left == 0 && right == 0, "empty ring must return silence");
  ok &= check(ring.push(0.5f, -0.5f) && ring.pop(left, right) && left == 0.5f && right == -0.5f,
    "SPSC ring must wrap without replacing channel data");

  ok &= simulateClocks(48000, 44100, 0.001, 2);
  ok &= simulateClocks(48000, 44100, -0.001, 2);
  ok &= simulateClocks(44100, 48000, 0.0, 2);
  ok &= simulateClocks(16000, 44100, 0.001, 1);
  ok &= simulateClocks(48000, 48000, -0.001, 1);

  localmixer::engine::LiveInputBridge worker(48000, 44100, 2);
  ok &= check(worker.start(), "native rate worker must start");
  for (int frame = 0; frame < 2048; ++frame) worker.push(0.1f, -0.1f);
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  while (worker.status().outputQueued < 882 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  ok &= check(worker.pop(left, right) && std::isfinite(left) && std::fabs(left + right) < 0.0001f,
    "native worker must deliver converted stereo frames through its output queue");
  worker.stop();

  localmixer::engine::LiveInputBridge stalled(48000, 44100, 1);
  for (int frame = 0; frame < 32768; ++frame) stalled.push(0.25f, 0.25f);
  ok &= check(!stalled.push(1, 1) && stalled.status().inputDropped == 1,
    "a stalled worker must remain bounded and count dropped input");
  localmixer::engine::LiveInputBridge invalid(0, 44100, 2);
  ok &= check(!invalid.valid() && !invalid.start(), "invalid rates must never start a converter");
  if (!ok) return 1;
  std::cout << "local-mixer-live-input-bridge-tests ok\n";
  return 0;
}
