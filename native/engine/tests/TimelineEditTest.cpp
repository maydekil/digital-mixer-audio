#include "engine/Metronome.hpp"
#include "engine/TimelineEdit.hpp"

#include <iostream>

namespace {

using localmixer::engine::EditableClip;
using localmixer::engine::MetronomeConfig;
using localmixer::engine::TimelineEditModel;
using localmixer::engine::metronomeClicks;
using localmixer::engine::metronomeContributesToExport;
using localmixer::engine::snapFrame;

}  // namespace

int main() {
  TimelineEditModel model;
  if (!model.addClip(EditableClip{
        .id = "clip-a",
        .mediaId = "media-1",
        .timelineStartFrame = 10,
        .sourceOffsetFrame = 100,
        .durationFrames = 10,
      })) {
    std::cerr << "clip add should succeed\n";
    return 1;
  }

  if (!model.splitClip("clip-a", 14, "clip-b")) {
    std::cerr << "valid split should succeed\n";
    return 1;
  }
  const auto left = model.clip("clip-a");
  const auto right = model.clip("clip-b");
  if (!left.has_value() || !right.has_value() || left->durationFrames != 4 ||
      right->timelineStartFrame != 14 || right->sourceOffsetFrame != 104 || right->durationFrames != 6) {
    std::cerr << "split should preserve sample positions\n";
    return 1;
  }

  if (!model.joinAdjacent("clip-a", "clip-b")) {
    std::cerr << "join after split should succeed\n";
    return 1;
  }
  const auto joined = model.clip("clip-a");
  if (!joined.has_value() || model.clips().size() != 1 || joined->sourceOffsetFrame != 100 ||
      joined->durationFrames != 10) {
    std::cerr << "join should restore the original clip reference\n";
    return 1;
  }

  if (!model.undo() || model.clips().size() != 2 || !model.redo() || model.clips().size() != 1) {
    std::cerr << "undo/redo should restore edit snapshots\n";
    return 1;
  }

  if (!model.addClip(EditableClip{
        .id = "overlap",
        .mediaId = "media-2",
        .timelineStartFrame = 12,
        .sourceOffsetFrame = 0,
        .durationFrames = 8,
      }) ||
      model.clips().size() != 2) {
    std::cerr << "overlap clips should be retained instead of replacing existing clips\n";
    return 1;
  }

  if (snapFrame(119, 100, true) != 100 || snapFrame(151, 100, true) != 200 || snapFrame(151, 100, false) != 151) {
    std::cerr << "snap frame behavior mismatch\n";
    return 1;
  }

  const MetronomeConfig metronome{.enabled = true, .sampleRate = 48000.0, .bpm = 120.0, .beatsPerBar = 4};
  const auto clicks = metronomeClicks(metronome, 0, 96000);
  if (clicks.size() != 4 || clicks[0].frame != 0 || clicks[1].frame != 24000 ||
      clicks[2].frame != 48000 || clicks[3].frame != 72000 || !clicks[0].accented || clicks[1].accented) {
    std::cerr << "metronome tempo should produce stable beat frames\n";
    return 1;
  }
  if (metronomeContributesToExport(metronome)) {
    std::cerr << "metronome should be monitor-only by default\n";
    return 1;
  }
  if (!metronomeContributesToExport(MetronomeConfig{.enabled = true, .printToMaster = true})) {
    std::cerr << "metronome print click should be explicit\n";
    return 1;
  }

  std::cout << "local-mixer-timeline-edit-tests ok\n";
  return 0;
}
