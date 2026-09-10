#pragma once

#include <cstdint>

namespace localmixer::engine {

enum class TransportState {
  stopped,
  playing,
  paused,
};

struct TransportLoop {
  bool enabled = false;
  std::uint64_t startFrame = 0;
  std::uint64_t endFrame = 0;
};

struct TransportSnapshot {
  TransportState state = TransportState::stopped;
  std::uint64_t positionFrame = 0;
  std::uint64_t startFrame = 0;
  std::uint64_t bufferGeneration = 0;
  TransportLoop loop;
};

class TransportClock {
 public:
  void setStartFrame(std::uint64_t frame);
  void setLoop(TransportLoop loop);
  void play();
  void pause();
  void stop();
  void seek(std::uint64_t frame);
  std::uint64_t advance(std::uint32_t frames);
  TransportSnapshot snapshot() const;

 private:
  TransportSnapshot snapshot_;
};

const char* transportStateName(TransportState state);

}  // namespace localmixer::engine
