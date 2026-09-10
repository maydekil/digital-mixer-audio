#include "engine/RealtimeMetrics.hpp"

#include <iostream>

int main() {
  localmixer::engine::RealtimeMetrics metrics;
  metrics.recordCallback(100, 200);
  metrics.recordCallback(300, 200);
  auto snapshot = metrics.snapshot();
  if (snapshot.callbackCount != 2 || snapshot.deadlineMissCount != 1 || snapshot.maxCallbackNanos != 300) {
    std::cerr << "realtime metrics should count callbacks, deadline misses, and max duration\n";
    return 1;
  }
  metrics.reset();
  snapshot = metrics.snapshot();
  if (snapshot.callbackCount != 0 || snapshot.deadlineMissCount != 0 || snapshot.maxCallbackNanos != 0) {
    std::cerr << "realtime metrics reset should clear counters\n";
    return 1;
  }
  std::cout << "local-mixer-realtime-metrics-tests ok\n";
  return 0;
}
