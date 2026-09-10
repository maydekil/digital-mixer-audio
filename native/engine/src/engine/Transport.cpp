#include "engine/Transport.hpp"

namespace localmixer::engine {

void TransportClock::setStartFrame(std::uint64_t frame) {
  snapshot_.startFrame = frame;
  if (snapshot_.state == TransportState::stopped) snapshot_.positionFrame = frame;
}

void TransportClock::setLoop(TransportLoop loop) {
  if (loop.enabled && loop.endFrame <= loop.startFrame) loop.enabled = false;
  snapshot_.loop = loop;
}

void TransportClock::play() {
  snapshot_.state = TransportState::playing;
}

void TransportClock::pause() {
  if (snapshot_.state == TransportState::playing) snapshot_.state = TransportState::paused;
}

void TransportClock::stop() {
  snapshot_.state = TransportState::stopped;
  snapshot_.positionFrame = snapshot_.startFrame;
  snapshot_.bufferGeneration += 1;
}

void TransportClock::seek(std::uint64_t frame) {
  snapshot_.positionFrame = frame;
  snapshot_.bufferGeneration += 1;
}

std::uint64_t TransportClock::advance(std::uint32_t frames) {
  const auto begin = snapshot_.positionFrame;
  if (snapshot_.state != TransportState::playing || frames == 0) return begin;

  snapshot_.positionFrame += frames;
  if (snapshot_.loop.enabled && snapshot_.positionFrame >= snapshot_.loop.endFrame) {
    const auto loopLength = snapshot_.loop.endFrame - snapshot_.loop.startFrame;
    const auto overflow = snapshot_.positionFrame - snapshot_.loop.startFrame;
    snapshot_.positionFrame = snapshot_.loop.startFrame + (overflow % loopLength);
  }
  return begin;
}

TransportSnapshot TransportClock::snapshot() const {
  return snapshot_;
}

const char* transportStateName(TransportState state) {
  switch (state) {
    case TransportState::stopped: return "stopped";
    case TransportState::playing: return "playing";
    case TransportState::paused: return "paused";
  }
  return "unknown";
}

}  // namespace localmixer::engine
