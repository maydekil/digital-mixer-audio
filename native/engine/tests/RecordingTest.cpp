#include "engine/MediaFile.hpp"
#include "engine/Recording.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace {

using localmixer::engine::MediaFileError;
using localmixer::engine::RecordingConfig;
using localmixer::engine::RecordingSession;
using localmixer::engine::RecordingState;
using localmixer::engine::WavStreamReader;
using localmixer::engine::makeCollisionSafeTakePath;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  const auto directory = std::filesystem::temp_directory_path() / "local-mixer-recording-test";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory);

  {
    std::ofstream existing(directory / "take.wav");
    existing << "occupied";
  }
  const auto nextTake = makeCollisionSafeTakePath(directory, "take", ".wav");
  if (nextTake.filename() != "take-001.wav") {
    std::cerr << "collision-safe take naming mismatch\n";
    return 1;
  }

  RecordingSession session;
  if (!session.arm(RecordingConfig{.directory = directory, .baseName = "voice", .sampleRate = 48000, .channels = 2}) ||
      session.state() != RecordingState::armed || !session.start() || session.state() != RecordingState::recording) {
    std::cerr << "recording state machine should arm and start\n";
    return 1;
  }

  const std::vector<float> samples{0.25f, -0.25f, 2.0f, -2.0f};
  if (!session.write(samples) || session.framesWritten() != 2 || !session.stop() ||
      session.state() != RecordingState::saved) {
    std::cerr << "recording session should write and save frames\n";
    return 1;
  }

  WavStreamReader reader;
  if (reader.open(session.path()) != MediaFileError::none || reader.info().channels != 2 ||
      reader.info().sampleRate != 48000 || reader.info().bitsPerSample != 32 || reader.info().frameCount != 2) {
    std::cerr << "recorded WAV metadata mismatch\n";
    return 1;
  }
  const auto read = reader.readFrames(0, 2);
  if (read.error != MediaFileError::none || read.samples.size() != 4 || !near(read.samples[0], 0.25f) ||
      !near(read.samples[1], -0.25f) || !near(read.samples[2], 1.0f) || !near(read.samples[3], -1.0f)) {
    std::cerr << "recorded WAV samples should round-trip as clamped float32\n";
    return 1;
  }

  RecordingSession overrun;
  const std::vector<float> overrunSamples{0.5f};
  if (!overrun.arm(RecordingConfig{.directory = directory, .baseName = "overrun", .sampleRate = 48000, .channels = 1}) ||
      !overrun.start() || !overrun.write(overrunSamples)) {
    std::cerr << "overrun fixture setup failed\n";
    return 1;
  }
  overrun.markOverrun();
  if (!overrun.stop() || overrun.state() != RecordingState::overrun || overrun.framesWritten() != 1) {
    std::cerr << "overrun should finalize valid data and retain overrun status\n";
    return 1;
  }

  std::filesystem::remove_all(directory);
  std::cout << "local-mixer-recording-tests ok\n";
  return 0;
}
