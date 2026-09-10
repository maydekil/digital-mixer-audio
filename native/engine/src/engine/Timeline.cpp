#include "engine/Timeline.hpp"

#include <algorithm>

namespace localmixer::engine {

bool TimelineScheduler::addMedia(std::string mediaId, TimelineMedia media) {
  if (mediaId.empty() || media.channels == 0 || media.samples.empty()) return false;
  if (media.samples.size() % media.channels != 0) return false;
  media_[std::move(mediaId)] = std::move(media);
  return true;
}

void TimelineScheduler::clearMedia() {
  media_.clear();
}

void TimelineScheduler::setClips(std::vector<TimelineClip> clips) {
  clips_ = std::move(clips);
}

bool TimelineScheduler::render(std::uint64_t startFrame, TimelineRenderBuffer output) const {
  if (output.left.size() != output.right.size()) return false;
  std::fill(output.left.begin(), output.left.end(), 0.0f);
  std::fill(output.right.begin(), output.right.end(), 0.0f);

  const auto endFrame = startFrame + output.left.size();
  for (const auto& clip : clips_) {
    const auto clipEnd = clip.timelineStartFrame + clip.durationFrames;
    if (clipEnd <= startFrame || clip.timelineStartFrame >= endFrame) continue;

    const auto renderStart = std::max(startFrame, clip.timelineStartFrame);
    const auto renderEnd = std::min<std::uint64_t>(endFrame, clipEnd);
    const auto media = media_.find(clip.mediaId);
    if (media == media_.end()) return false;
    const auto& source = media->second;
    const auto sourceFrames = source.samples.size() / source.channels;

    for (auto frame = renderStart; frame < renderEnd; frame += 1) {
      const auto sourceFrame = clip.sourceOffsetFrame + (frame - clip.timelineStartFrame);
      if (sourceFrame >= sourceFrames) continue;
      const auto outputIndex = static_cast<std::size_t>(frame - startFrame);
      const auto sourceIndex = static_cast<std::size_t>(sourceFrame * source.channels);
      const auto left = source.samples[sourceIndex] * clip.gain;
      const auto right = source.channels > 1 ? source.samples[sourceIndex + 1] * clip.gain : left;
      output.left[outputIndex] += left;
      output.right[outputIndex] += right;
    }
  }
  return true;
}

}  // namespace localmixer::engine
