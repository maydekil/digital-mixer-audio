#include "engine/RoutingGraph.hpp"

#include <cmath>
#include <iostream>

namespace {

using localmixer::engine::ChannelRouteState;
using localmixer::engine::RouteEdge;
using localmixer::engine::RouteNode;
using localmixer::engine::RouteNodeType;
using localmixer::engine::RoutingGraph;
using localmixer::engine::SendTap;
using localmixer::engine::effectiveSendGainDb;
using localmixer::engine::routeContributesToExport;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  RoutingGraph graph;
  if (!graph.addNode(RouteNode{.id = "voice", .type = RouteNodeType::channel, .name = "Voice"}) ||
      !graph.addNode(RouteNode{.id = "group-1", .type = RouteNodeType::subgroup, .name = "Group 1"}) ||
      !graph.addNode(RouteNode{.id = "fx-a", .type = RouteNodeType::aux, .name = "FX A"}) ||
      !graph.addNode(RouteNode{.id = "main", .type = RouteNodeType::main, .name = "Main"}) ||
      !graph.addNode(RouteNode{.id = "monitor", .type = RouteNodeType::monitor, .name = "Monitor"})) {
    std::cerr << "routing nodes should be accepted once\n";
    return 1;
  }

  if (graph.addNode(RouteNode{.id = "voice", .type = RouteNodeType::channel, .name = "Duplicate"})) {
    std::cerr << "duplicate route node ids should be rejected\n";
    return 1;
  }

  if (!graph.addEdge(RouteEdge{.sourceId = "voice", .destinationId = "group-1"}) ||
      !graph.addEdge(RouteEdge{.sourceId = "group-1", .destinationId = "main"}) ||
      !graph.addEdge(RouteEdge{.sourceId = "voice", .destinationId = "fx-a", .tap = SendTap::preFader}) ||
      !graph.addEdge(RouteEdge{.sourceId = "voice", .destinationId = "monitor", .tap = SendTap::pfl})) {
    std::cerr << "valid route edges should be accepted\n";
    return 1;
  }

  if (graph.addEdge(RouteEdge{.sourceId = "main", .destinationId = "voice"}) || graph.hasCycle()) {
    std::cerr << "cycle route should be rejected before graph mutation\n";
    return 1;
  }
  if (graph.edgeCount() != 4) {
    std::cerr << "rejected cycle should not change active route graph\n";
    return 1;
  }

  const auto order = graph.topologicalOrder();
  if (order.size() != graph.nodeCount() || order.front() != "voice" || order.back() != "main") {
    std::cerr << "route graph should provide deterministic topological order\n";
    return 1;
  }

  const ChannelRouteState state{.faderDb = -12.0f, .dcaGainDb = -3.0f};
  const RouteEdge pre{.sourceId = "voice", .destinationId = "fx-a", .tap = SendTap::preFader, .gainDb = -6.0f};
  const RouteEdge post{.sourceId = "voice", .destinationId = "main", .tap = SendTap::postFader, .gainDb = -6.0f};
  const auto preGain = effectiveSendGainDb(pre, state);
  const auto postGain = effectiveSendGainDb(post, state);
  if (!preGain.has_value() || !postGain.has_value() || !near(*preGain, -6.0f) || !near(*postGain, -21.0f)) {
    std::cerr << "pre/post send effective gain mismatch\n";
    return 1;
  }

  const auto mutedGain = effectiveSendGainDb(pre, ChannelRouteState{.muted = true});
  if (mutedGain.has_value()) {
    std::cerr << "muted channels should not feed sends\n";
    return 1;
  }

  const RouteEdge pfl{.sourceId = "voice", .destinationId = "monitor", .tap = SendTap::pfl};
  const RouteEdge afl{.sourceId = "voice", .destinationId = "monitor", .tap = SendTap::afl};
  if (routeContributesToExport(pfl) || routeContributesToExport(afl) || !routeContributesToExport(post)) {
    std::cerr << "PFL/AFL should stay out of export routes\n";
    return 1;
  }

  std::cout << "local-mixer-routing-graph-tests ok\n";
  return 0;
}
