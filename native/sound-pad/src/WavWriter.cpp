#include "WavWriter.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <vector>

namespace soundpad {
namespace {

void writeU16(std::ofstream& out, uint16_t value) {
  out.put(static_cast<char>(value & 0xff));
  out.put(static_cast<char>((value >> 8) & 0xff));
}

void writeU32(std::ofstream& out, uint32_t value) {
  out.put(static_cast<char>(value & 0xff));
  out.put(static_cast<char>((value >> 8) & 0xff));
  out.put(static_cast<char>((value >> 16) & 0xff));
  out.put(static_cast<char>((value >> 24) & 0xff));
}

std::vector<int16_t> toPcm16(const RenderedSound& sound) {
  std::vector<int16_t> pcm;
  pcm.reserve(sound.interleaved.size());
  for (const float sample : sound.interleaved) {
    pcm.push_back(static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, sample)) * 32767.0f));
  }
  return pcm;
}

}  // namespace

bool writeWav(const RenderedSound& sound, const std::filesystem::path& path) {
  const auto pcm = toPcm16(sound);
  const uint32_t dataBytes = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;

  out.write("RIFF", 4);
  writeU32(out, 36U + dataBytes);
  out.write("WAVE", 4);
  out.write("fmt ", 4);
  writeU32(out, 16);
  writeU16(out, 1);
  writeU16(out, static_cast<uint16_t>(sound.channels));
  writeU32(out, static_cast<uint32_t>(sound.sampleRate));
  writeU32(out, static_cast<uint32_t>(sound.sampleRate) * static_cast<uint32_t>(sound.channels) * sizeof(int16_t));
  writeU16(out, static_cast<uint16_t>(sound.channels * sizeof(int16_t)));
  writeU16(out, 16);
  out.write("data", 4);
  writeU32(out, dataBytes);
  out.write(reinterpret_cast<const char*>(pcm.data()), dataBytes);
  return out.good();
}

}  // namespace soundpad
