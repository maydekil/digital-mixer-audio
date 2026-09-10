#include "engine/Export.hpp"
#include "engine/MediaFile.hpp"
#include "engine/Timeline.hpp"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <array>

namespace {

using localmixer::engine::ExportRequest;
using localmixer::engine::ExportGraphSource;
using localmixer::engine::ExportStemRequest;
using localmixer::engine::MediaFileError;
using localmixer::engine::TimelineClip;
using localmixer::engine::TimelineMedia;
using localmixer::engine::TimelineScheduler;
using localmixer::engine::WavStreamReader;
using localmixer::engine::buildExportStemPlan;
using localmixer::engine::exportMixerGraphToWav;
using localmixer::engine::exportTimelineToWav;
using localmixer::engine::partialExportPath;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  const auto directory = std::filesystem::temp_directory_path() / "local-mixer-export-test";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory);

  TimelineScheduler timeline;
  if (!timeline.addMedia("impulse", TimelineMedia{.samples = {0.25f, -0.25f, 0.5f, -0.5f}, .channels = 2})) {
    std::cerr << "timeline media fixture should be accepted\n";
    return 1;
  }
  timeline.setClips({TimelineClip{.mediaId = "impulse", .timelineStartFrame = 1, .durationFrames = 2}});

  const auto exportPath = directory / "mix.wav";
  const auto exported = exportTimelineToWav(timeline, ExportRequest{
    .outputPath = exportPath,
    .sampleRate = 48000,
    .durationFrames = 4,
    .blockFrames = 2,
    .tailFrames = 2,
  });
  if (!exported.success || exported.framesWritten != 6 || exported.path != exportPath) {
    std::cerr << "timeline export should write duration plus explicit tail\n";
    return 1;
  }

  WavStreamReader reader;
  if (reader.open(exportPath) != MediaFileError::none || reader.info().channels != 2 || reader.info().frameCount != 6) {
    std::cerr << "exported WAV metadata mismatch\n";
    return 1;
  }
  const auto read = reader.readFrames(0, 6);
  if (read.samples.size() != 12 || !near(read.samples[2], 0.25f) || !near(read.samples[3], -0.25f) ||
      !near(read.samples[4], 0.5f) || !near(read.samples[5], -0.5f) || !near(read.samples[10], 0.0f)) {
    std::cerr << "exported timeline samples mismatch\n";
    return 1;
  }

  const auto liveRejected = exportTimelineToWav(timeline, ExportRequest{
    .outputPath = directory / "live.wav",
    .durationFrames = 4,
    .liveSourceCount = 1,
  });
  if (liveRejected.success || liveRejected.ignoredLiveSources != 1 || liveRejected.error.empty()) {
    std::cerr << "offline export should reject live sources before rendering\n";
    return 1;
  }

  const auto cancelPath = directory / "cancel.wav";
  const auto canceled = exportTimelineToWav(timeline, ExportRequest{
    .outputPath = cancelPath,
    .durationFrames = 8,
    .blockFrames = 2,
    .cancelAfterFrames = 4,
  });
  if (canceled.success || !canceled.canceled || canceled.path != partialExportPath(cancelPath) ||
      !std::filesystem::exists(canceled.path)) {
    std::cerr << "canceled export should leave a .partial file\n";
    return 1;
  }

  const auto stemPlan = buildExportStemPlan(ExportStemRequest{
    .master = true,
    .fxAReturn = true,
    .fxBReturn = true,
    .includeMonitorVolume = true,
  });
  if (!stemPlan.master || !stemPlan.fxAReturn || !stemPlan.fxBReturn || stemPlan.stemCount != 3 ||
      stemPlan.monitorVolumePrinted) {
    std::cerr << "FX returns should export as separate stems without printing monitor volume\n";
    return 1;
  }

  localmixer::engine::MixerRenderRuntime runtime{48000.0};
  const auto strip = runtime.graph().createStrip("Processed", "#18d6e7");
  if (strip.error != localmixer::engine::MixerError::none) return 1;
  runtime.graph().setLevel(strip.id, 0.0f, -6.0f, 0.0f);
  const std::vector<float> mono{1.0f, 1.0f, 1.0f, 1.0f};
  const std::array<ExportGraphSource, 1> graphSources{
    ExportGraphSource{.stripId = strip.id, .samples = mono, .channels = 1}
  };
  const auto processedPath = directory / "processed.wav";
  const auto processed = exportMixerGraphToWav(runtime, graphSources, ExportRequest{
    .outputPath = processedPath,
    .sampleRate = 48000,
    .durationFrames = 4,
    .blockFrames = 2,
  });
  if (!processed.success || processed.framesWritten != 4) {
    std::cerr << "processed graph export should render through MixerRenderRuntime\n";
    return 1;
  }
  WavStreamReader processedReader;
  if (processedReader.open(processedPath) != MediaFileError::none) return 1;
  const auto processedRead = processedReader.readFrames(0, 4);
  if (processedRead.samples.size() != 8 || !near(processedRead.samples[0], 0.5011872f) ||
      !near(processedRead.samples[1], 0.5011872f)) {
    std::cerr << "processed graph export should print channel graph gain\n";
    return 1;
  }

  std::filesystem::remove_all(directory);
  std::cout << "local-mixer-export-tests ok\n";
  return 0;
}
