#include "engine/RealtimeMetrics.hpp"

namespace localmixer::engine {

void RealtimeMetrics::reset() noexcept {
  callbackCount_.store(0, std::memory_order_relaxed);
  deadlineMissCount_.store(0, std::memory_order_relaxed);
  maxCallbackNanos_.store(0, std::memory_order_relaxed);
}

void RealtimeMetrics::recordCallback(std::uint64_t elapsedNanos, std::uint64_t deadlineNanos) noexcept {
  callbackCount_.fetch_add(1, std::memory_order_relaxed);
  if (deadlineNanos > 0 && elapsedNanos > deadlineNanos) {
    deadlineMissCount_.fetch_add(1, std::memory_order_relaxed);
  }
  auto current = maxCallbackNanos_.load(std::memory_order_relaxed);
  while (elapsedNanos > current &&
         !maxCallbackNanos_.compare_exchange_weak(current, elapsedNanos, std::memory_order_relaxed)) {
  }
}

RealtimeMetricsSnapshot RealtimeMetrics::snapshot() const noexcept {
  return {
    .callbackCount = callbackCount_.load(std::memory_order_relaxed),
    .deadlineMissCount = deadlineMissCount_.load(std::memory_order_relaxed),
    .maxCallbackNanos = maxCallbackNanos_.load(std::memory_order_relaxed),
  };
}

}  // namespace localmixer::engine
