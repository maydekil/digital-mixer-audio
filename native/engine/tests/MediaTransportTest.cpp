#include "engine/MediaFile.hpp"
#include "engine/Transport.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

using localmixer::engine::MediaFileError;
using localmixer::engine::TransportClock;
using localmixer::engine::TransportLoop;
using localmixer::engine::TransportState;
using localmixer::engine::WavStreamReader;

void writeU16(std::ofstream& file, std::uint16_t value) {
  file.put(static_cast<char>(value & 0xFF));
  file.put(static_cast<char>((value >> 8) & 0xFF));
}

void writeU32(std::ofstream& file, std::uint32_t value) {
  file.put(static_cast<char>(value & 0xFF));
  file.put(static_cast<char>((value >> 8) & 0xFF));
  file.put(static_cast<char>((value >> 16) & 0xFF));
  file.put(static_cast<char>((value >> 24) & 0xFF));
}

std::filesystem::path writeFixtureWav() {
  const auto path = std::filesystem::temp_directory_path() / "local-mixer-media-transport-fixture.wav";
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  const std::uint16_t channels = 2;
  const std::uint32_t sampleRate = 48000;
  const std::uint16_t bits = 16;
  const std::uint32_t frames = 4;
  const std::uint32_t dataBytes = frames * channels * (bits / 8);

  file.write("RIFF", 4);
  writeU32(file, 36 + dataBytes);
  file.write("WAVE", 4);
  file.write("fmt ", 4);
  writeU32(file, 16);
  writeU16(file, 1);
  writeU16(file, channels);
  writeU32(file, sampleRate);
  writeU32(file, sampleRate * channels * (bits / 8));
  writeU16(file, channels * (bits / 8));
  writeU16(file, bits);
  file.write("data", 4);
  writeU32(file, dataBytes);
  const std::int16_t samples[] = {0, 32767, -32768, 16384, 8192, -8192, 0, 0};
  for (const auto sample : samples) writeU16(file, static_cast<std::uint16_t>(sample));
  return path;
}

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001f;
}

}  // namespace

int main() {
  const auto fixture = writeFixtureWav();
  WavStreamReader reader;
  if (reader.open(fixture) != MediaFileError::none) {
    std::cerr << "fixture wav should open\n";
    return 1;
  }
  if (reader.info().channels != 2 || reader.info().sampleRate != 48000 || reader.info().frameCount != 4) {
    std::cerr << "fixture wav metadata mismatch\n";
    return 1;
  }

  const auto chunk = reader.readFrames(1, 2);
  if (chunk.error != MediaFileError::none || chunk.framesRead != 2 || chunk.samples.size() != 4) {
    std::cerr << "streaming read window failed\n";
    return 1;
  }
  if (!near(chunk.samples[0], -1.0f) || !near(chunk.samples[1], 0.5f) ||
      !near(chunk.samples[2], 0.25f) || !near(chunk.samples[3], -0.25f)) {
    std::cerr << "streaming read samples were not normalized correctly\n";
    return 1;
  }

  TransportClock clock;
  clock.setStartFrame(10);
  clock.play();
  if (clock.advance(5) != 10 || clock.snapshot().positionFrame != 15) {
    std::cerr << "transport should advance while playing\n";
    return 1;
  }
  clock.pause();
  clock.advance(7);
  if (clock.snapshot().positionFrame != 15) {
    std::cerr << "pause should retain position\n";
    return 1;
  }
  clock.seek(20);
  clock.setLoop(TransportLoop{.enabled = true, .startFrame = 20, .endFrame = 24});
  clock.play();
  clock.advance(6);
  if (clock.snapshot().positionFrame != 22) {
    std::cerr << "loop should wrap position without stale advance\n";
    return 1;
  }
  clock.stop();
  if (clock.snapshot().state != TransportState::stopped || clock.snapshot().positionFrame != 10) {
    std::cerr << "stop should return to playback start\n";
    return 1;
  }

  std::filesystem::remove(fixture);
  std::cout << "local-mixer-media-transport-tests ok\n";
  return 0;
}
