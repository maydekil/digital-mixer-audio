#include "engine/MixerGraph.hpp"
#include "engine/MixerGraphController.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::engine::MixerError;
using localmixer::engine::MixerCommand;
using localmixer::engine::MixerCommandType;
using localmixer::engine::MixerControlQueue;
using localmixer::engine::MixerGraph;
using localmixer::engine::MixerGraphController;
using localmixer::engine::SourceAssignment;
using localmixer::engine::SourceBuffer;
using localmixer::engine::StereoOutput;
using localmixer::engine::StripId;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

bool expect(MixerError actual, MixerError expected, const char* message) {
  if (actual == expected) return true;
  std::cerr << message << "\n";
  return false;
}

}  // namespace

int main() {
  MixerGraph graph;
  const auto vocal = graph.createStrip("Voice", "#18d6e7");
  const auto music = graph.createStrip("Music", "#f3c842");
  if (!expect(vocal.error, MixerError::none, "vocal create failed") ||
      !expect(music.error, MixerError::none, "music create failed")) {
    return 1;
  }

  graph.renameStrip(vocal.id, "Lead Voice");
  graph.setColor(vocal.id, "#58f28a");
  graph.setAssignment(vocal.id, SourceAssignment::mono, 0, false);
  graph.setAssignment(music.id, SourceAssignment::stereo, 0, true);
  graph.setLevel(vocal.id, 0.0f, -6.0f, 0.0f);
  graph.setLevel(music.id, 0.0f, 0.0f, 0.0f);
  graph.setInputMonitoring(vocal.id, true);

  const auto config = graph.strip(vocal.id);
  if (!config.has_value() || config->name != "Lead Voice" || config->color != "#58f28a" || !config->inputMonitoring) {
    std::cerr << "strip metadata/state mismatch\n";
    return 1;
  }

  std::array<float, 4> voiceSamples{1.0f, 1.0f, 1.0f, 1.0f};
  std::array<float, 8> musicSamples{0.25f, 0.5f, 0.25f, 0.5f, 0.25f, 0.5f, 0.25f, 0.5f};
  std::array<float, 4> left{};
  std::array<float, 4> right{};
  std::array<SourceBuffer, 2> sources{
    SourceBuffer{.stripId = vocal.id, .samples = voiceSamples, .channels = 1},
    SourceBuffer{.stripId = music.id, .samples = musicSamples, .channels = 2},
  };

  if (!expect(graph.process(sources, StereoOutput{.left = left, .right = right}), MixerError::none, "process failed")) return 1;
  for (std::size_t index = 0; index < left.size(); index += 1) {
    if (!near(left[index], 0.7511872f) || !near(right[index], 1.0011872f)) {
      std::cerr << "known-amplitude sum mismatch\n";
      return 1;
    }
  }

  graph.setMute(music.id, true);
  if (!expect(graph.process(sources, StereoOutput{.left = left, .right = right}), MixerError::none, "mute process failed")) return 1;
  if (!near(left[0], 0.5011872f) || !near(right[0], 0.5011872f)) {
    std::cerr << "mute should silence only music source\n";
    return 1;
  }

  graph.setMute(music.id, false);
  graph.setSolo(music.id, true);
  if (!expect(graph.process(sources, StereoOutput{.left = left, .right = right}), MixerError::none, "solo process failed")) return 1;
  if (!near(left[0], 0.25f) || !near(right[0], 0.5f)) {
    std::cerr << "solo should isolate music source\n";
    return 1;
  }

  graph.setSolo(music.id, false);
  graph.setEnabled(vocal.id, false);
  if (!expect(graph.process(sources, StereoOutput{.left = left, .right = right}), MixerError::none, "disable process failed")) return 1;
  if (!near(left[0], 0.25f) || !near(right[0], 0.5f)) {
    std::cerr << "disabled vocal should not affect music\n";
    return 1;
  }

  if (!expect(graph.removeStrip(vocal.id), MixerError::none, "remove failed")) return 1;
  if (!expect(graph.removeStrip(vocal.id), MixerError::staleStripId, "stale remove should be rejected")) return 1;
  if (!expect(graph.process(sources, StereoOutput{.left = left, .right = right}), MixerError::staleStripId, "stale source should be rejected")) return 1;

  MixerGraph capacityGraph;
  std::vector<float> silent(16, 0.0f);
  std::array<float, 8> silentLeft{};
  std::array<float, 8> silentRight{};
  std::vector<SourceBuffer> silentSources;
  silentSources.reserve(localmixer::engine::kMaxMixerStrips);
  for (std::size_t index = 0; index < localmixer::engine::kMaxMixerStrips; index += 1) {
    const auto created = capacityGraph.createStrip("Silent", "#000000");
    if (created.error != MixerError::none) {
      std::cerr << "32 silent strips should fit\n";
      return 1;
    }
    silentSources.push_back(SourceBuffer{.stripId = created.id, .samples = silent, .channels = 2});
  }
  if (capacityGraph.createStrip("Too Many", "#ffffff").error != MixerError::graphFull) {
    std::cerr << "33rd strip should be rejected\n";
    return 1;
  }
  if (!expect(capacityGraph.process(silentSources, StereoOutput{.left = silentLeft, .right = silentRight}), MixerError::none, "silent process failed")) return 1;

  MixerControlQueue queue(2);
  const StripId first{101};
  const StripId second{102};
  if (!expect(queue.push(MixerCommand{.type = MixerCommandType::setMute, .stripId = first, .boolValue = true}),
              MixerError::none,
              "first control push failed")) {
    return 1;
  }
  if (!expect(queue.push(MixerCommand{.type = MixerCommandType::setMute, .stripId = second, .boolValue = true}),
              MixerError::none,
              "second control push failed")) {
    return 1;
  }
  if (!expect(queue.push(MixerCommand{.type = MixerCommandType::setMute, .stripId = second, .boolValue = false}),
              MixerError::queueFull,
              "bounded queue should reject overflow")) {
    return 1;
  }
  const auto popped = queue.pop();
  if (!popped.has_value() || popped->stripId != first || queue.size() != 1) {
    std::cerr << "bounded queue should preserve FIFO order\n";
    return 1;
  }

  MixerGraph rampGraph;
  const auto rampStrip = rampGraph.createStrip("Ramp", "#18d6e7");
  if (rampStrip.error != MixerError::none) return 1;
  std::array<float, 5> rampSource{1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
  std::array<float, 5> rampLeft{};
  std::array<float, 5> rampRight{};
  std::array<SourceBuffer, 1> rampSources{SourceBuffer{.stripId = rampStrip.id, .samples = rampSource, .channels = 1}};
  if (!expect(rampGraph.enqueueControl(MixerCommand{.type = MixerCommandType::setLevel,
                                                   .stripId = rampStrip.id,
                                                   .trimDb = 0.0f,
                                                   .faderDb = -6.0f,
                                                   .pan = 0.0f}),
              MixerError::none,
              "ramp enqueue failed")) {
    return 1;
  }
  if (!expect(rampGraph.applyQueuedControls(4), MixerError::none, "ramp command apply failed")) return 1;
  if (!expect(rampGraph.process(rampSources, StereoOutput{.left = rampLeft, .right = rampRight}),
              MixerError::none,
              "ramp process failed")) {
    return 1;
  }
  if (!(rampLeft[0] > rampLeft[1] && rampLeft[1] > rampLeft[2] && rampLeft[2] > rampLeft[3] &&
        rampLeft[3] > rampLeft[4] && near(rampLeft[4], 0.5011872f))) {
    std::cerr << "queued level change should ramp over process frames\n";
    return 1;
  }

  if (!expect(rampGraph.enqueueControl(MixerCommand{.type = MixerCommandType::setMute, .stripId = StripId{999}, .boolValue = true}),
              MixerError::none,
              "stale control enqueue failed")) {
    return 1;
  }
  if (!expect(rampGraph.applyQueuedControls(4), MixerError::staleStripId, "stale queued control should be rejected")) return 1;

  rampGraph.setMute(rampStrip.id, true);
  if (!expect(rampGraph.process(rampSources, StereoOutput{.left = rampLeft, .right = rampRight}),
              MixerError::none,
              "muted meter process failed")) {
    return 1;
  }
  const auto mutedMeters = rampGraph.meters(rampStrip.id);
  if (!mutedMeters.has_value() || !near(mutedMeters->inputPeak, 1.0f) || !near(mutedMeters->outputPeakLeft, 0.0f)) {
    std::cerr << "input meter should remain independent from muted output\n";
    return 1;
  }

  MixerGraphController controller;
  const auto controlled = controller.active().createStrip("Voice", "#18d6e7");
  auto prepared = controller.prepare();
  prepared.renameStrip(controlled.id, "Lead Voice");
  if (controller.active().strip(controlled.id)->name != "Voice") {
    std::cerr << "prepared graph should not mutate active graph before publish\n";
    return 1;
  }
  controller.publish(std::move(prepared));
  if (controller.active().strip(controlled.id)->name != "Lead Voice" || controller.retiredCount() != 1) {
    std::cerr << "published graph should replace active graph and retire old graph\n";
    return 1;
  }
  controller.reclaimRetired();
  if (controller.retiredCount() != 0) {
    std::cerr << "retired graphs should be reclaimed off the callback path\n";
    return 1;
  }

  std::cout << "local-mixer-graph-tests ok\n";
  return 0;
}
