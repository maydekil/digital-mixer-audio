#include "engine/MixerGraphController.hpp"

namespace localmixer::engine {

MixerGraph& MixerGraphController::active() {
  return active_;
}

const MixerGraph& MixerGraphController::active() const {
  return active_;
}

MixerGraph MixerGraphController::prepare() const {
  return active_;
}

void MixerGraphController::publish(MixerGraph prepared) {
  retired_.push_back(active_);
  active_ = std::move(prepared);
}

void MixerGraphController::reclaimRetired() {
  retired_.clear();
}

std::size_t MixerGraphController::retiredCount() const {
  return retired_.size();
}

}  // namespace localmixer::engine
