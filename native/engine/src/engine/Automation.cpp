#include "engine/Automation.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace localmixer::engine {

AutomationLane::AutomationLane(AutomationValueMode mode) : mode_(mode) {}

void AutomationLane::setPoints(std::vector<AutomationPoint> points) {
  points_ = std::move(points);
  std::sort(points_.begin(), points_.end(), [](const auto& left, const auto& right) {
    return left.frame < right.frame;
  });
}

void AutomationLane::addPoint(AutomationPoint point) {
  points_.push_back(point);
  std::sort(points_.begin(), points_.end(), [](const auto& left, const auto& right) {
    return left.frame < right.frame;
  });
}

float AutomationLane::valueAt(std::uint64_t frame, float defaultValue) const {
  if (points_.empty()) return defaultValue;
  if (frame <= points_.front().frame) return points_.front().value;
  for (std::size_t index = 1; index < points_.size(); index += 1) {
    const auto& previous = points_[index - 1];
    const auto& next = points_[index];
    if (frame <= next.frame) {
      if (mode_ == AutomationValueMode::discrete && frame == next.frame) return next.value;
      if (mode_ == AutomationValueMode::discrete || next.frame == previous.frame) return previous.value;
      const auto position = static_cast<float>(frame - previous.frame) / static_cast<float>(next.frame - previous.frame);
      return previous.value + (next.value - previous.value) * position;
    }
  }
  return points_.back().value;
}

const std::vector<AutomationPoint>& AutomationLane::points() const {
  return points_;
}

void AutomationEditHistory::beginGesture(const AutomationLane& lane) {
  beforeGesture_ = lane;
}

void AutomationEditHistory::commitGesture(const AutomationLane& lane) {
  if (!beforeGesture_.has_value()) return;
  undoLane_ = *beforeGesture_;
  beforeGesture_.reset();
  (void)lane;
}

std::string automationParameterId(AutomationParameter parameter, const std::string& targetId) {
  switch (parameter) {
    case AutomationParameter::fader: return "channel." + targetId + ".fader";
    case AutomationParameter::pan: return "channel." + targetId + ".pan";
    case AutomationParameter::mute: return "channel." + targetId + ".mute";
    case AutomationParameter::send: return "fx.send." + targetId;
    case AutomationParameter::fxReturn: return "fx.return." + targetId;
    case AutomationParameter::fxMacro: return "fx.macro." + targetId;
  }
  return "unknown." + targetId;
}

std::optional<AutomationLane> AutomationEditHistory::undo() {
  auto lane = undoLane_;
  undoLane_.reset();
  return lane;
}

std::optional<float> midiControlValue(const MidiEvent& event, const MidiMapping& mapping) {
  if (event.type != MidiEventType::controlChange) return std::nullopt;
  if (event.channel != mapping.channel || event.number != mapping.controller) return std::nullopt;
  return static_cast<float>(event.value) / 127.0f;
}

bool softTakeoverAllowsUpdate(float currentValue, float incomingValue, float tolerance) {
  return std::fabs(currentValue - incomingValue) <= std::max(0.0f, tolerance);
}

}  // namespace localmixer::engine
