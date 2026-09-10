#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace localmixer::engine {

struct TimelineClip {
  std::string mediaId;
  std::uint64_t timelineStartFrame = 0;
  std::uint64_t sourceOffsetFrame = 0;
  std::uint64_t durationFrames = 0;
  std::uint32_t fadeInFrames = 0;
  std::uint32_t fadeOutFrames = 0;
  float gain = 1.0f;
};

struct TimelineMedia {
  std::vector<float> samples;
  std::uint16_t channels = 1;
};

struct TimelineRenderBuffer {
  std::span<float> left;
  std::span<float> right;
};

class TimelineScheduler {
 public:
  bool addMedia(std::string mediaId, TimelineMedia media);
  void clearMedia();
  void setClips(std::vector<TimelineClip> clips);
  bool render(std::uint64_t startFrame, TimelineRenderBuffer output) const;

 private:
  std::unordered_map<std::string, TimelineMedia> media_;
  std::vector<TimelineClip> clips_;
};

}  // namespace localmixer::engine
