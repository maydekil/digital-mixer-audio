#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

namespace localmixer::engine {

// Bounded SPSC queue. Producer never overwrites unread samples; consumer
// never changes the producer cursor. Construct/reset only while stopped.
class AudioFrameRing {
 public:
  AudioFrameRing(std::uint32_t capacity, std::uint32_t channels)
    : samples_(static_cast<std::size_t>(capacity) * channels), capacity_(capacity), channels_(channels) {}

  bool push(float left, float right) noexcept {
    const auto write = write_.load(std::memory_order_relaxed);
    if (write - read_.load(std::memory_order_acquire) >= capacity_) return false;
    const auto offset = static_cast<std::size_t>(write % capacity_) * channels_;
    samples_[offset] = left;
    if (channels_ == 2) samples_[offset + 1] = right;
    write_.store(write + 1, std::memory_order_release);
    return true;
  }

  bool pop(float& left, float& right) noexcept {
    const auto read = read_.load(std::memory_order_relaxed);
    if (read == write_.load(std::memory_order_acquire)) {
      left = right = 0.0f;
      return false;
    }
    const auto offset = static_cast<std::size_t>(read % capacity_) * channels_;
    left = samples_[offset];
    right = channels_ == 2 ? samples_[offset + 1] : left;
    read_.store(read + 1, std::memory_order_release);
    return true;
  }

  // Each queue endpoint may call this; other observers get a bounded snapshot.
  std::uint64_t available() const noexcept {
    const auto read = read_.load(std::memory_order_acquire);
    const auto write = write_.load(std::memory_order_acquire);
    return write >= read ? std::min<std::uint64_t>(write - read, capacity_) : 0;
  }

 private:
  static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
  std::vector<float> samples_;
  std::uint32_t capacity_;
  std::uint32_t channels_;
  alignas(64) std::atomic<std::uint64_t> write_{0};
  alignas(64) std::atomic<std::uint64_t> read_{0};
};

}  // namespace localmixer::engine
