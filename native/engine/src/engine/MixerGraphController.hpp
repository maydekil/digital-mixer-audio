#pragma once

#include <vector>

#include "engine/MixerGraph.hpp"

namespace localmixer::engine {

class MixerGraphController {
 public:
  MixerGraph& active();
  const MixerGraph& active() const;

  MixerGraph prepare() const;
  void publish(MixerGraph prepared);
  void reclaimRetired();
  std::size_t retiredCount() const;

 private:
  MixerGraph active_;
  std::vector<MixerGraph> retired_;
};

}  // namespace localmixer::engine
