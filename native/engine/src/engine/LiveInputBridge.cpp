#include "engine/LiveInputBridge.hpp"

#include <samplerate.h>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace localmixer::engine {
namespace {
constexpr std::uint32_t kInputFrames = 256;
constexpr std::uint32_t kQueueFrames = 32768;
}

bool supportedLiveSampleRate(double rate) noexcept {
  return std::isfinite(rate) && rate >= 8000.0 && rate <= 192000.0;
}

void LiveInputBridge::SrcDeleter::operator()(SRC_STATE_tag* state) const { src_delete(state); }

LiveInputBridge::LiveInputBridge(double inputRate, double outputRate, std::uint32_t channels)
  : input_(kQueueFrames, channels == 2 ? 2 : 1), output_(kQueueFrames, channels == 2 ? 2 : 1),
    channels_(channels), nominalRatio_(1.0), outputRate_(outputRate), targetFrames_(0) {
  if (!supportedLiveSampleRate(inputRate) || !supportedLiveSampleRate(outputRate) ||
      (channels != 1 && channels != 2)) return;
  nominalRatio_ = outputRate / inputRate;
  targetFrames_ = static_cast<std::uint32_t>(std::ceil(outputRate * 0.02));
  int error = 0;
  converter_.reset(src_new(SRC_SINC_FASTEST, static_cast<int>(channels), &error));
  if (!converter_ || error != 0) return;
  ratio_.store(nominalRatio_, std::memory_order_relaxed);
  src_set_ratio(converter_.get(), nominalRatio_);
  inputScratch_.resize(kInputFrames * channels);
  const auto maxOutput = static_cast<std::size_t>(std::ceil(kInputFrames * nominalRatio_ * 1.01)) + 256;
  outputScratch_.resize(maxOutput * channels);
}

LiveInputBridge::~LiveInputBridge() { stop(); }

bool LiveInputBridge::valid() const noexcept { return converter_ && !failed_.load(std::memory_order_relaxed); }

bool LiveInputBridge::start() {
  if (!valid() || worker_.joinable()) return false;
  stopping_.store(false, std::memory_order_relaxed);
  try {
    worker_ = std::thread([this] {
      while (!stopping_.load(std::memory_order_relaxed) && valid()) {
        if (!processAvailable()) std::this_thread::sleep_for(std::chrono::microseconds(250));
      }
    });
  } catch (...) { return false; }
  return true;
}

void LiveInputBridge::stop() {
  stopping_.store(true, std::memory_order_relaxed);
  if (worker_.joinable()) worker_.join();
}

bool LiveInputBridge::push(float left, float right) noexcept {
  if (input_.push(left, right)) return true;
  inputDropped_.fetch_add(1, std::memory_order_relaxed);
  return false;
}

bool LiveInputBridge::pop(float& left, float& right) noexcept {
  if (!primed_) {
    if (output_.available() < targetFrames_) { left = right = 0.0f; return false; }
    primed_ = true;
  }
  if (output_.pop(left, right)) return true;
  primed_ = false;
  outputUnderruns_.fetch_add(1, std::memory_order_relaxed);
  return false;
}

bool LiveInputBridge::processAvailable() {
  if (!valid() || output_.available() >= targetFrames_) return false;
  if (pendingFrames_ == 0) {
    if (input_.available() < kInputFrames) return false;
    for (std::uint32_t frame = 0; frame < kInputFrames; ++frame) {
      float left = 0, right = 0;
      input_.pop(left, right);
      inputScratch_[frame * channels_] = left;
      if (channels_ == 2) inputScratch_[frame * channels_ + 1] = right;
    }
    pendingFrames_ = kInputFrames;
    pendingOffset_ = 0;
  }
  // Queue-depth feedback tracks independent device clocks. Bound correction
  // to +/-2000 ppm and smooth it; libsamplerate also interpolates ratio changes.
  const auto queued = (input_.available() + pendingFrames_) * nominalRatio_ + output_.available();
  const auto correction = std::clamp((queued - targetFrames_) / outputRate_ * 0.2, -0.002, 0.002);
  const auto previous = ratio_.load(std::memory_order_relaxed);
  const auto ratio = previous + 0.005 * (nominalRatio_ * (1.0 - correction) - previous);
  SRC_DATA data{};
  data.data_in = inputScratch_.data() + pendingOffset_ * channels_;
  data.input_frames = pendingFrames_;
  data.data_out = outputScratch_.data();
  data.output_frames = static_cast<long>(outputScratch_.size() / channels_);
  data.src_ratio = ratio;
  if (src_process(converter_.get(), &data) != 0 ||
      (data.input_frames_used == 0 && data.output_frames_gen == 0)) {
    failed_.store(true, std::memory_order_relaxed);
    return false;
  }
  pendingOffset_ += static_cast<std::uint32_t>(data.input_frames_used);
  pendingFrames_ -= static_cast<std::uint32_t>(data.input_frames_used);
  for (long frame = 0; frame < data.output_frames_gen; ++frame) {
    const auto left = outputScratch_[static_cast<std::size_t>(frame) * channels_];
    const auto right = channels_ == 2 ? outputScratch_[static_cast<std::size_t>(frame) * channels_ + 1] : left;
    if (!output_.push(left, right)) { failed_.store(true, std::memory_order_relaxed); return false; }
  }
  ratio_.store(ratio, std::memory_order_relaxed);
  return true;
}

LiveInputBridgeStatus LiveInputBridge::status() const noexcept {
  return {.inputDropped = inputDropped_.load(std::memory_order_relaxed),
    .outputUnderruns = outputUnderruns_.load(std::memory_order_relaxed),
    .inputQueued = input_.available(), .outputQueued = output_.available(),
    .ratio = ratio_.load(std::memory_order_relaxed), .failed = failed_.load(std::memory_order_relaxed)};
}

}  // namespace localmixer::engine
