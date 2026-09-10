#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

struct EditableClip {
  std::string id;
  std::string mediaId;
  std::uint64_t timelineStartFrame = 0;
  std::uint64_t sourceOffsetFrame = 0;
  std::uint64_t durationFrames = 0;
  float gain = 1.0f;
};

class TimelineEditModel {
 public:
  bool addClip(EditableClip clip);
  bool moveClip(const std::string& clipId, std::uint64_t startFrame);
  bool trimClip(const std::string& clipId, std::uint64_t sourceOffsetFrame, std::uint64_t durationFrames);
  bool splitClip(const std::string& clipId, std::uint64_t splitFrame, std::string newClipId);
  bool joinAdjacent(const std::string& firstClipId, const std::string& secondClipId);
  bool undo();
  bool redo();
  std::optional<EditableClip> clip(const std::string& clipId) const;
  const std::vector<EditableClip>& clips() const;

 private:
  void checkpoint();
  std::optional<std::size_t> indexOf(const std::string& clipId) const;

  std::vector<EditableClip> clips_;
  std::vector<std::vector<EditableClip>> undoStack_;
  std::vector<std::vector<EditableClip>> redoStack_;
};

std::uint64_t snapFrame(std::uint64_t frame, std::uint64_t gridFrames, bool enabled);

}  // namespace localmixer::engine
