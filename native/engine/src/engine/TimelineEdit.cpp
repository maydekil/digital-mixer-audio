#include "engine/TimelineEdit.hpp"

#include <algorithm>
#include <utility>

namespace localmixer::engine {

bool TimelineEditModel::addClip(EditableClip clip) {
  if (clip.id.empty() || clip.mediaId.empty() || clip.durationFrames == 0 || indexOf(clip.id).has_value()) return false;
  checkpoint();
  clips_.push_back(std::move(clip));
  return true;
}

bool TimelineEditModel::moveClip(const std::string& clipId, std::uint64_t startFrame) {
  const auto index = indexOf(clipId);
  if (!index.has_value()) return false;
  checkpoint();
  clips_[*index].timelineStartFrame = startFrame;
  return true;
}

bool TimelineEditModel::trimClip(
  const std::string& clipId, std::uint64_t sourceOffsetFrame, std::uint64_t durationFrames) {
  const auto index = indexOf(clipId);
  if (!index.has_value() || durationFrames == 0) return false;
  checkpoint();
  clips_[*index].sourceOffsetFrame = sourceOffsetFrame;
  clips_[*index].durationFrames = durationFrames;
  return true;
}

bool TimelineEditModel::splitClip(const std::string& clipId, std::uint64_t splitFrame, std::string newClipId) {
  const auto index = indexOf(clipId);
  if (!index.has_value() || newClipId.empty() || indexOf(newClipId).has_value()) return false;
  const auto clip = clips_[*index];
  if (splitFrame <= clip.timelineStartFrame || splitFrame >= clip.timelineStartFrame + clip.durationFrames) return false;

  checkpoint();
  const auto leftDuration = splitFrame - clip.timelineStartFrame;
  EditableClip right = clip;
  right.id = std::move(newClipId);
  right.timelineStartFrame = splitFrame;
  right.sourceOffsetFrame += leftDuration;
  right.durationFrames -= leftDuration;
  clips_[*index].durationFrames = leftDuration;
  clips_.insert(clips_.begin() + static_cast<std::ptrdiff_t>(*index + 1), std::move(right));
  return true;
}

bool TimelineEditModel::joinAdjacent(const std::string& firstClipId, const std::string& secondClipId) {
  const auto first = indexOf(firstClipId);
  const auto second = indexOf(secondClipId);
  if (!first.has_value() || !second.has_value() || *first == *second) return false;
  const auto& left = clips_[*first];
  const auto& right = clips_[*second];
  const auto contiguousTimeline = left.timelineStartFrame + left.durationFrames == right.timelineStartFrame;
  const auto contiguousSource = left.sourceOffsetFrame + left.durationFrames == right.sourceOffsetFrame;
  if (left.mediaId != right.mediaId || !contiguousTimeline || !contiguousSource) return false;

  checkpoint();
  clips_[*first].durationFrames += right.durationFrames;
  clips_.erase(clips_.begin() + static_cast<std::ptrdiff_t>(*second));
  return true;
}

bool TimelineEditModel::undo() {
  if (undoStack_.empty()) return false;
  redoStack_.push_back(clips_);
  clips_ = std::move(undoStack_.back());
  undoStack_.pop_back();
  return true;
}

bool TimelineEditModel::redo() {
  if (redoStack_.empty()) return false;
  undoStack_.push_back(clips_);
  clips_ = std::move(redoStack_.back());
  redoStack_.pop_back();
  return true;
}

std::optional<EditableClip> TimelineEditModel::clip(const std::string& clipId) const {
  const auto index = indexOf(clipId);
  if (!index.has_value()) return std::nullopt;
  return clips_[*index];
}

const std::vector<EditableClip>& TimelineEditModel::clips() const {
  return clips_;
}

void TimelineEditModel::checkpoint() {
  undoStack_.push_back(clips_);
  redoStack_.clear();
}

std::optional<std::size_t> TimelineEditModel::indexOf(const std::string& clipId) const {
  const auto it = std::find_if(clips_.begin(), clips_.end(), [&](const auto& clip) {
    return clip.id == clipId;
  });
  if (it == clips_.end()) return std::nullopt;
  return static_cast<std::size_t>(std::distance(clips_.begin(), it));
}

std::uint64_t snapFrame(std::uint64_t frame, std::uint64_t gridFrames, bool enabled) {
  if (!enabled || gridFrames == 0) return frame;
  const auto half = gridFrames / 2;
  return ((frame + half) / gridFrames) * gridFrames;
}

}  // namespace localmixer::engine
