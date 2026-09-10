#include "engine/Automation.hpp"

#include <cmath>
#include <iostream>

namespace {

using localmixer::engine::AutomationEditHistory;
using localmixer::engine::AutomationLane;
using localmixer::engine::AutomationParameter;
using localmixer::engine::AutomationPoint;
using localmixer::engine::AutomationValueMode;
using localmixer::engine::MidiEvent;
using localmixer::engine::MidiEventType;
using localmixer::engine::MidiMapping;
using localmixer::engine::automationParameterId;
using localmixer::engine::midiControlValue;
using localmixer::engine::softTakeoverAllowsUpdate;

bool near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.0001f;
}

}  // namespace

int main() {
  AutomationLane fader;
  fader.setPoints({AutomationPoint{.frame = 0, .value = 0.0f}, AutomationPoint{.frame = 100, .value = 1.0f}});
  if (!near(fader.valueAt(50, 0.5f), 0.5f) || !near(fader.valueAt(200, 0.0f), 1.0f)) {
    std::cerr << "continuous automation should interpolate and seek deterministically\n";
    return 1;
  }

  AutomationLane mute(AutomationValueMode::discrete);
  mute.setPoints({AutomationPoint{.frame = 0, .value = 0.0f}, AutomationPoint{.frame = 100, .value = 1.0f}});
  if (!near(mute.valueAt(50, 0.0f), 0.0f) || !near(mute.valueAt(100, 0.0f), 1.0f) ||
      !near(mute.valueAt(101, 0.0f), 1.0f)) {
    std::cerr << "mute automation should use discrete event values\n";
    return 1;
  }

  AutomationEditHistory history;
  history.beginGesture(fader);
  fader.addPoint(AutomationPoint{.frame = 200, .value = 0.25f});
  history.commitGesture(fader);
  const auto undone = history.undo();
  if (!undone.has_value() || undone->points().size() != 2) {
    std::cerr << "undo should restore one gesture as a single action\n";
    return 1;
  }

  MidiMapping mapping{
    .deviceUid = "midi-controller",
    .channel = 1,
    .controller = 7,
    .parameter = AutomationParameter::fader,
    .deviceConnected = false,
  };
  const auto value = midiControlValue(MidiEvent{
    .timestampFrame = 128,
    .channel = 1,
    .type = MidiEventType::controlChange,
    .number = 7,
    .value = 64,
  }, mapping);
  if (!value.has_value() || !near(*value, 64.0f / 127.0f) || mapping.deviceUid != "midi-controller") {
    std::cerr << "MIDI mapping should survive disconnect and map injected CC values\n";
    return 1;
  }

  const auto ignored = midiControlValue(MidiEvent{.channel = 2, .number = 7, .value = 64}, mapping);
  if (ignored.has_value()) {
    std::cerr << "MIDI mapping should ignore mismatched channels\n";
    return 1;
  }

  if (softTakeoverAllowsUpdate(0.8f, 0.1f, 0.05f) || !softTakeoverAllowsUpdate(0.8f, 0.82f, 0.05f)) {
    std::cerr << "soft takeover should prevent fader jumps until pickup\n";
    return 1;
  }

  if (automationParameterId(AutomationParameter::send, "voice.fx-a") != "fx.send.voice.fx-a" ||
      automationParameterId(AutomationParameter::fxReturn, "fx-a") != "fx.return.fx-a" ||
      automationParameterId(AutomationParameter::fxMacro, "fx-a.macro1") != "fx.macro.fx-a.macro1" ||
      automationParameterId(AutomationParameter::harmonyEnabled, "voice") != "harmony.enabled.voice" ||
      automationParameterId(AutomationParameter::harmonyLevel, "voice") != "harmony.level.voice") {
    std::cerr << "FX automation parameter IDs should be stable and explicit\n";
    return 1;
  }

  MidiMapping fxReturnMapping{
    .deviceUid = "midi-controller",
    .channel = 1,
    .controller = 91,
    .parameter = AutomationParameter::fxReturn,
    .targetId = "fx-a",
    .deviceConnected = false,
  };
  const auto fxReturnValue = midiControlValue(MidiEvent{.channel = 1, .number = 91, .value = 100}, fxReturnMapping);
  if (!fxReturnValue.has_value() || !near(*fxReturnValue, 100.0f / 127.0f)) {
    std::cerr << "FX return MIDI mapping should survive disconnect and map CC values\n";
    return 1;
  }

  std::cout << "local-mixer-automation-tests ok\n";
  return 0;
}
