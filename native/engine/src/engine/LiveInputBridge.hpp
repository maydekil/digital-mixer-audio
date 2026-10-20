#pragma once

#include "engine/AudioFrameRing.hpp"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

struct SRC_STATE_tag;

namespace localmixer::engine {

struct LiveInputBridgeStatus {
  std::uint64_t inputDropped = 0;
  std::uint64_t outputUnderruns = 0;
  std::uint64_t inputQueued = 0;
  std::uint64_t outputQueued = 0;
  double ratio = 1.0;
  bool failed = false;
};

// One native worker owns SRC processing. Input/output callbacks only exchange
// frames through bounded, preallocated queues; neither callback waits.
class LiveInputBridge {
 public:
  LiveInputBridge(double inputRate, double outputRate, std::uint32_t channels);
  ~LiveInputBridge();
  LiveInputBridge(const LiveInputBridge&) = delete;
  LiveInputBridge& operator=(const LiveInputBridge&) = delete;

  bool valid() const noexcept;
  bool start();
  void stop();
  bool push(float left, float right) noexcept;
  bool pop(float& left, float& right) noexcept;
  LiveInputBridgeStatus status() const noexcept;

  // Single worker only. May be driven synchronously, without start(), by
  // deterministic clock-simulation tests. Never call from an audio callback.
  bool processAvailable();

 private:
  struct SrcDeleter { void operator()(SRC_STATE_tag* state) const; };
  std::unique_ptr<SRC_STATE_tag, SrcDeleter> converter_;
  AudioFrameRing input_;
  AudioFrameRing output_;
  std::uint32_t channels_;
  double nominalRatio_;
  double outputRate_;
  std::uint32_t targetFrames_;
  std::vector<float> inputScratch_;
  std::vector<float> outputScratch_;
  std::uint32_t pendingFrames_ = 0;
  std::uint32_t pendingOffset_ = 0;
  bool primed_ = false; // output callback owns this flag
  std::thread worker_;
  std::atomic<bool> stopping_{false};
  std::atomic<bool> failed_{false};
  std::atomic<std::uint64_t> inputDropped_{0};
  std::atomic<std::uint64_t> outputUnderruns_{0};
  std::atomic<double> ratio_{1.0};
};

bool supportedLiveSampleRate(double rate) noexcept;

}  // namespace localmixer::engine
