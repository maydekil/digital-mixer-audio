#pragma once

#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class RouteNodeType {
  channel,
  subgroup,
  aux,
  fxReturn,
  main,
  monitor,
};

enum class SendTap {
  preFader,
  postFader,
  pfl,
  afl,
};

struct RouteNode {
  std::string id;
  RouteNodeType type = RouteNodeType::channel;
  std::string name;
};

struct RouteEdge {
  std::string sourceId;
  std::string destinationId;
  SendTap tap = SendTap::postFader;
  float gainDb = 0.0f;
};

struct ChannelRouteState {
  float faderDb = 0.0f;
  float dcaGainDb = 0.0f;
  bool muted = false;
};

class RoutingGraph {
 public:
  bool addNode(RouteNode node);
  bool addEdge(RouteEdge edge);
  bool canAddEdge(const RouteEdge& edge) const;
  bool hasCycle() const;
  std::vector<std::string> topologicalOrder() const;
  std::size_t nodeCount() const;
  std::size_t edgeCount() const;

 private:
  bool containsNode(const std::string& id) const;
  bool reaches(const std::string& sourceId, const std::string& targetId) const;

  std::vector<RouteNode> nodes_;
  std::vector<RouteEdge> edges_;
};

std::optional<float> effectiveSendGainDb(const RouteEdge& edge, const ChannelRouteState& state);
bool routeContributesToExport(const RouteEdge& edge);

}  // namespace localmixer::engine
