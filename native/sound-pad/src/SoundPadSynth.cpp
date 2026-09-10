#include "SoundPadSynth.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

namespace soundpad {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kSampleRate = 48000.0;
constexpr int kChannels = 2;

float clamp(float value) {
  return std::max(-0.95f, std::min(0.95f, value));
}

void addStereo(std::vector<float>& data, int frame, float value, float pan) {
  if (frame < 0) return;
  const auto index = static_cast<size_t>(frame) * 2U;
  if (index + 1 >= data.size()) return;
  const float left = std::sqrt(std::max(0.0f, 1.0f - pan));
  const float right = std::sqrt(std::max(0.0f, pan));
  data[index] = clamp(data[index] + value * left);
  data[index + 1] = clamp(data[index + 1] + value * right);
}

RenderedSound makeSound(double seconds) {
  RenderedSound sound;
  sound.sampleRate = kSampleRate;
  sound.channels = kChannels;
  sound.interleaved.assign(static_cast<size_t>(seconds * kSampleRate) * kChannels, 0.0f);
  return sound;
}

void addTone(RenderedSound& sound, double start, double duration, double frequency, float gain) {
  const int startFrame = static_cast<int>(start * sound.sampleRate);
  const int frameCount = static_cast<int>(duration * sound.sampleRate);
  for (int i = 0; i < frameCount; ++i) {
    const double t = static_cast<double>(i) / sound.sampleRate;
    const double env = std::exp(-5.5 * t / duration);
    const float sample = static_cast<float>(std::sin(2.0 * kPi * frequency * t) * env * gain);
    addStereo(sound.interleaved, startFrame + i, sample, 0.5f);
  }
}

void addNoiseBurst(RenderedSound& sound, double start, double duration, float gain, float pan, uint32_t seed) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
  const int startFrame = static_cast<int>(start * sound.sampleRate);
  const int frameCount = static_cast<int>(duration * sound.sampleRate);
  float last = 0.0f;
  for (int i = 0; i < frameCount; ++i) {
    const double phase = static_cast<double>(i) / std::max(1, frameCount - 1);
    const double attack = std::min(1.0, phase * 16.0);
    const double decay = std::pow(1.0 - phase, 2.1);
    last = last * 0.45f + noise(rng) * 0.55f;
    addStereo(sound.interleaved, startFrame + i, last * static_cast<float>(attack * decay) * gain, pan);
  }
}

RenderedSound renderApplause() {
  auto sound = makeSound(1.25);
  for (int i = 0; i < 34; ++i) {
    const double start = 0.03 + (i % 11) * 0.052 + (i / 11) * 0.13;
    const float pan = static_cast<float>((i * 37 % 100) / 100.0);
    addNoiseBurst(sound, start, 0.075, 0.34f, pan, 300U + static_cast<uint32_t>(i));
  }
  return sound;
}

RenderedSound renderLaugh() {
  auto sound = makeSound(1.1);
  const double starts[] = {0.04, 0.25, 0.46, 0.69};
  const double freqs[] = {210.0, 245.0, 220.0, 265.0};
  for (int syllable = 0; syllable < 4; ++syllable) {
    const int startFrame = static_cast<int>(starts[syllable] * sound.sampleRate);
    const int frameCount = static_cast<int>(0.18 * sound.sampleRate);
    for (int i = 0; i < frameCount; ++i) {
      const double t = static_cast<double>(i) / sound.sampleRate;
      const double env = std::sin(kPi * static_cast<double>(i) / frameCount);
      const double wobble = 1.0 + 0.08 * std::sin(2.0 * kPi * 7.0 * t);
      const double f = freqs[syllable] * wobble;
      const float sample = static_cast<float>((std::sin(2.0 * kPi * f * t) + 0.35 * std::sin(4.0 * kPi * f * t)) * env * 0.31);
      addStereo(sound.interleaved, startFrame + i, sample, 0.5f);
    }
  }
  return sound;
}

RenderedSound renderCheer() {
  auto sound = makeSound(1.35);
  for (int voice = 0; voice < 7; ++voice) {
    addTone(sound, 0.08 + voice * 0.035, 0.9, 320.0 + voice * 42.0, 0.08f);
  }
  for (int i = 0; i < 14; ++i) {
    addNoiseBurst(sound, 0.07 + i * 0.07, 0.12, 0.13f, static_cast<float>((i % 5) / 4.0), 800U + static_cast<uint32_t>(i));
  }
  return sound;
}

RenderedSound renderDrumroll() {
  auto sound = makeSound(1.2);
  for (int hit = 0; hit < 28; ++hit) {
    const double start = 0.03 + hit * 0.035;
    addNoiseBurst(sound, start, 0.035, 0.22f, hit % 2 == 0 ? 0.35f : 0.65f, 1200U + static_cast<uint32_t>(hit));
    addTone(sound, start, 0.06, 135.0, 0.12f);
  }
  addNoiseBurst(sound, 1.03, 0.14, 0.42f, 0.5f, 1400U);
  return sound;
}

RenderedSound renderDing() {
  auto sound = makeSound(0.9);
  addTone(sound, 0.0, 0.85, 880.0, 0.32f);
  addTone(sound, 0.0, 0.75, 1760.0, 0.12f);
  addTone(sound, 0.0, 0.5, 2637.0, 0.07f);
  return sound;
}

RenderedSound renderWhoosh() {
  auto sound = makeSound(0.95);
  std::mt19937 rng(1800U);
  std::uniform_real_distribution<float> noise(-1.0f, 1.0f);
  float low = 0.0f;
  for (size_t frame = 0; frame < sound.interleaved.size() / 2U; ++frame) {
    const double phase = static_cast<double>(frame) / (sound.interleaved.size() / 2U);
    const double env = std::sin(kPi * phase);
    low = low * static_cast<float>(0.93 - phase * 0.28) + noise(rng) * static_cast<float>(0.07 + phase * 0.28);
    const float sweep = static_cast<float>(std::sin(2.0 * kPi * (160.0 + 1100.0 * phase) * phase) * 0.08);
    addStereo(sound.interleaved, static_cast<int>(frame), (low + sweep) * static_cast<float>(env) * 0.38f, static_cast<float>(phase));
  }
  return sound;
}

}  // namespace

bool isSupportedPad(const std::string& id) {
  const auto pads = supportedPads();
  return std::find(pads.begin(), pads.end(), id) != pads.end();
}

std::vector<std::string> supportedPads() {
  return {"applause", "laugh", "cheer", "drumroll", "ding", "whoosh"};
}

RenderedSound renderPad(const std::string& id) {
  if (id == "applause") return renderApplause();
  if (id == "laugh") return renderLaugh();
  if (id == "cheer") return renderCheer();
  if (id == "drumroll") return renderDrumroll();
  if (id == "ding") return renderDing();
  if (id == "whoosh") return renderWhoosh();
  throw std::invalid_argument("unsupported sound pad: " + id);
}

}  // namespace soundpad
