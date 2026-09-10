#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace localmixer::engine {

enum class AutomationParameter {
  fader,
  pan,
  mute,
  send,
  fxReturn,
  fxMacro,
  harmonyEnabled,
  harmonyLevel,
};

enum class AutomationValueMode {
  continuous,
  discrete,
};

struct AutomationPoint {
  std::uint64_t frame = 0;
  float value = 0.0f;
};

class AutomationLane {
 public:
  explicit AutomationLane(AutomationValueMode mode = AutomationValueMode::continuous);
  void setPoints(std::vector<AutomationPoint> points);
  void addPoint(AutomationPoint point);
  float valueAt(std::uint64_t frame, float defaultValue) const;
  const std::vector<AutomationPoint>& points() const;

 private:
  AutomationValueMode mode_;
  std::vector<AutomationPoint> points_;
};

class AutomationEditHistory {
 public:
  void beginGesture(const AutomationLane& lane);
  void commitGesture(const AutomationLane& lane);
  std::optional<AutomationLane> undo();

 private:
  std::optional<AutomationLane> beforeGesture_;
  std::optional<AutomationLane> undoLane_;
};

enum class MidiEventType {
  noteOn,
  noteOff,
  controlChange,
};

struct MidiEvent {
  std::uint64_t timestampFrame = 0;
  std::uint8_t channel = 1;
  MidiEventType type = MidiEventType::controlChange;
  std::uint8_t number = 0;
  std::uint8_t value = 0;
};

struct MidiMapping {
  std::string deviceUid;
  std::uint8_t channel = 1;
  std::uint8_t controller = 0;
  AutomationParameter parameter = AutomationParameter::fader;
  std::string targetId;
  bool deviceConnected = true;
};

std::string automationParameterId(AutomationParameter parameter, const std::string& targetId);
std::optional<float> midiControlValue(const MidiEvent& event, const MidiMapping& mapping);
bool softTakeoverAllowsUpdate(float currentValue, float incomingValue, float tolerance);

}  // namespace localmixer::engine
