#include "engine/Timeline.hpp"

#include <array>
#include <cmath>
#include <iostream>

namespace {

using localmixer::engine::TimelineClip;
using localmixer::engine::TimelineMedia;
using localmixer::engine::TimelineRenderBuffer;
using localmixer::engine::TimelineScheduler;

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001f;
}

}  // namespace

int main() {
  TimelineScheduler timeline;
  if (!timeline.addMedia("impulse-a", TimelineMedia{.samples = {0.0f, 1.0f, 0.0f}, .channels = 1}) ||
      !timeline.addMedia("impulse-b", TimelineMedia{.samples = {0.0f, 0.5f, 0.0f}, .channels = 1}) ||
      !timeline.addMedia("offset", TimelineMedia{.samples = {0.0f, 0.0f, 0.75f, 0.0f}, .channels = 1}) ||
      !timeline.addMedia("fade", TimelineMedia{.samples = {1.0f, 1.0f, 1.0f, 1.0f}, .channels = 1})) {
    std::cerr << "timeline media should validate\n";
    return 1;
  }

  timeline.setClips({
    TimelineClip{.mediaId = "impulse-a", .timelineStartFrame = 4, .durationFrames = 3},
    TimelineClip{.mediaId = "impulse-b", .timelineStartFrame = 4, .durationFrames = 3},
    TimelineClip{.mediaId = "offset", .timelineStartFrame = 8, .sourceOffsetFrame = 2, .durationFrames = 1},
    TimelineClip{.mediaId = "fade", .timelineStartFrame = 9, .durationFrames = 4, .fadeInFrames = 2, .fadeOutFrames = 2},
  });

  std::array<float, 13> left{};
  std::array<float, 13> right{};
  if (!timeline.render(0, TimelineRenderBuffer{.left = left, .right = right})) {
    std::cerr << "timeline render should succeed\n";
    return 1;
  }
  if (!near(left[5], 1.5f) || !near(right[5], 1.5f)) {
    std::cerr << "parallel impulses should align at the same sample frame\n";
    return 1;
  }
  if (!near(left[8], 0.75f) || !near(right[8], 0.75f)) {
    std::cerr << "source offset should render the intended source frame\n";
    return 1;
  }
  if (!near(left[4], 0.0f) || !near(left[6], 0.0f)) {
    std::cerr << "timeline should not leak stale samples around impulses\n";
    return 1;
  }
  if (!near(left[9], 0.5f) || !near(left[10], 1.0f) || !near(left[11], 1.0f) || !near(left[12], 0.5f)) {
    std::cerr << "clip fades should ramp boundary samples\n";
    return 1;
  }

  std::cout << "local-mixer-timeline-tests ok\n";
  return 0;
}
