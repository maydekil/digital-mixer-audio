#pragma once

#include <atomic>
#include <cstdint>

namespace localmixer::engine {

struct RealtimeMetricsSnapshot {
  std::uint64_t callbackCount = 0;
  std::uint64_t deadlineMissCount = 0;
  std::uint64_t maxCallbackNanos = 0;
};

class RealtimeMetrics {
 public:
  void reset() noexcept;
  void recordCallback(std::uint64_t elapsedNanos, std::uint64_t deadlineNanos) noexcept;
  RealtimeMetricsSnapshot snapshot() const noexcept;

 private:
  std::atomic<std::uint64_t> callbackCount_{0};
  std::atomic<std::uint64_t> deadlineMissCount_{0};
  std::atomic<std::uint64_t> maxCallbackNanos_{0};
};

}  // namespace localmixer::engine
