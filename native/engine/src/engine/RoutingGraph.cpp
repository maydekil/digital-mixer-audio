#include "engine/RoutingGraph.hpp"

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace localmixer::engine {

bool RoutingGraph::addNode(RouteNode node) {
  if (node.id.empty() || containsNode(node.id)) return false;
  nodes_.push_back(std::move(node));
  return true;
}

bool RoutingGraph::addEdge(RouteEdge edge) {
  if (!canAddEdge(edge)) return false;
  edges_.push_back(std::move(edge));
  return true;
}

bool RoutingGraph::canAddEdge(const RouteEdge& edge) const {
  if (edge.sourceId.empty() || edge.destinationId.empty() || edge.sourceId == edge.destinationId) return false;
  if (!containsNode(edge.sourceId) || !containsNode(edge.destinationId)) return false;
  return !reaches(edge.destinationId, edge.sourceId);
}

bool RoutingGraph::hasCycle() const {
  return topologicalOrder().size() != nodes_.size();
}

std::vector<std::string> RoutingGraph::topologicalOrder() const {
  std::unordered_map<std::string, std::size_t> indegree;
  std::unordered_map<std::string, std::vector<std::string>> outgoing;
  for (const auto& node : nodes_) indegree[node.id] = 0;
  for (const auto& edge : edges_) {
    outgoing[edge.sourceId].push_back(edge.destinationId);
    indegree[edge.destinationId] += 1;
  }

  std::deque<std::string> ready;
  for (const auto& node : nodes_) {
    if (indegree[node.id] == 0) ready.push_back(node.id);
  }

  std::vector<std::string> order;
  while (!ready.empty()) {
    const auto id = ready.front();
    ready.pop_front();
    order.push_back(id);
    for (const auto& next : outgoing[id]) {
      indegree[next] -= 1;
      if (indegree[next] == 0) ready.push_back(next);
    }
  }
  return order;
}

std::size_t RoutingGraph::nodeCount() const {
  return nodes_.size();
}

std::size_t RoutingGraph::edgeCount() const {
  return edges_.size();
}

bool RoutingGraph::containsNode(const std::string& id) const {
  return std::any_of(nodes_.begin(), nodes_.end(), [&](const auto& node) {
    return node.id == id;
  });
}

bool RoutingGraph::reaches(const std::string& sourceId, const std::string& targetId) const {
  std::deque<std::string> pending{sourceId};
  std::unordered_set<std::string> visited;
  while (!pending.empty()) {
    const auto current = pending.front();
    pending.pop_front();
    if (current == targetId) return true;
    if (!visited.insert(current).second) continue;
    for (const auto& edge : edges_) {
      if (edge.sourceId == current) pending.push_back(edge.destinationId);
    }
  }
  return false;
}

std::optional<float> effectiveSendGainDb(const RouteEdge& edge, const ChannelRouteState& state) {
  if (state.muted) return std::nullopt;
  switch (edge.tap) {
    case SendTap::preFader:
    case SendTap::pfl:
      return edge.gainDb;
    case SendTap::postFader:
    case SendTap::afl:
      return edge.gainDb + state.faderDb + state.dcaGainDb;
  }
  return std::nullopt;
}

bool routeContributesToExport(const RouteEdge& edge) {
  return edge.tap != SendTap::pfl && edge.tap != SendTap::afl;
}

}  // namespace localmixer::engine
