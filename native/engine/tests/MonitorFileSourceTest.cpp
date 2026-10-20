#include "engine/MonitorFileSource.hpp"
#include "engine/MediaResampler.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <vector>

namespace {

void writeInteger(std::ofstream& file, std::uint32_t value, int bytes) {
  for (int index = 0; index < bytes; ++index) file.put(static_cast<char>((value >> (index * 8)) & 0xff));
}

void writeWav(const std::filesystem::path& path, std::uint32_t rate, std::uint32_t frames) {
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  file.write("RIFF", 4);
  writeInteger(file, 36 + frames * 4, 4);
  file.write("WAVEfmt ", 8);
  writeInteger(file, 16, 4);
  writeInteger(file, 1, 2);
  writeInteger(file, 2, 2);
  writeInteger(file, rate, 4);
  writeInteger(file, rate * 4, 4);
  writeInteger(file, 4, 2);
  writeInteger(file, 16, 2);
  file.write("data", 4);
  writeInteger(file, frames * 4, 4);
  for (std::uint32_t frame = 0; frame < frames; ++frame) {
    const auto value = static_cast<std::int16_t>(16000 * std::sin(2.0 * std::numbers::pi * 1000.0 * frame / rate));
    writeInteger(file, static_cast<std::uint16_t>(value), 2);
    writeInteger(file, static_cast<std::uint16_t>(-value), 2);
  }
}

bool check(bool condition, const char* message) {
  if (!condition) std::cerr << message << '\n';
  return condition;
}

}  // namespace

int main() {
  using localmixer::engine::prepareMonitorFile;
  const auto path = std::filesystem::temp_directory_path() / "local-mixer-monitor-rate-test.wav";
  writeWav(path, 48000, 48000);
  bool ok = true;
  for (const auto rate : {44100, 48000, 96000}) {
    const auto file = prepareMonitorFile(path, rate);
    ok &= check(file.error.empty() && file.channels == 2 && file.frameCount == static_cast<std::uint64_t>(rate),
      "monitor file must preserve one-second duration at output rate");
    if (!file.error.empty() || file.samples.empty()) continue;
    int risingCrossings = 0;
    float previous = 0.0f;
    for (std::size_t frame = 0; frame < file.frameCount; ++frame) {
      const auto left = file.samples[frame * 2];
      const auto right = file.samples[frame * 2 + 1];
      ok &= check(std::isfinite(left) && std::fabs(left + right) < 0.0001f,
        "resampling must preserve separate opposing stereo channels");
      if (previous <= 0.0f && left > 0.0f) ++risingCrossings;
      previous = left;
    }
    ok &= check(risingCrossings >= 999 && risingCrossings <= 1001,
      "monitor rate conversion must preserve 1 kHz pitch");
  }
  writeWav(path, 44100, 44100);
  const auto upsampled = prepareMonitorFile(path, 48000);
  ok &= check(upsampled.error.empty() && upsampled.frameCount == 48000,
    "44.1 kHz MUSIC must also play for one second on 48 kHz outputs");

  const std::vector<float> oneFrame{0.25f, -0.25f};
  const auto tail = localmixer::engine::resampleLinear({
    .samples = oneFrame, .channels = 2, .sourceSampleRate = 24000, .targetSampleRate = 192000});
  ok &= check(tail.size() == 16, "single-frame upsampling must produce eight stereo frames");
  for (std::size_t frame = 0; frame < tail.size() / 2; ++frame) {
    ok &= check(tail[frame * 2] == 0.25f && tail[frame * 2 + 1] == -0.25f,
      "upsampler tail must clamp to final valid source frame");
  }

  writeWav(path, 48000, 0);
  ok &= check(!prepareMonitorFile(path, 44100).error.empty(), "empty WAV must fail before callbacks start");
  writeWav(path, 48000, 48000);
  std::filesystem::resize_file(path, 48);
  ok &= check(prepareMonitorFile(path, 44100).error == "MEDIA_FILE_READ_FAILED",
    "truncated WAV must fail before callbacks start");
  std::filesystem::remove(path);
  ok &= check(prepareMonitorFile(path, 44100).error == "MEDIA_FILE_OPEN_FAILED", "missing WAV must fail clearly");
  ok &= check(!prepareMonitorFile(path, 0).error.empty(), "invalid output rate must fail clearly");
  if (!ok) return 1;
  std::cout << "local-mixer-monitor-file-source-tests ok\n";
  return 0;
}
