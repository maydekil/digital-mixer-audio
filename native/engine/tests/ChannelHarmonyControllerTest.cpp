#include "engine/ChannelHarmonyController.hpp"

#include <cassert>
#include <iostream>

int main() {
  localmixer::engine::ChannelHarmonyController controller;
  controller.registerChannel("voice", localmixer::engine::ChannelContentRole::vocal);
  controller.registerChannel("music", localmixer::engine::ChannelContentRole::music);
  controller.registerChannel("full", localmixer::engine::ChannelContentRole::vocal, {}, 8);
  controller.registerChannel("multi", localmixer::engine::ChannelContentRole::vocal, {"h-1", "h-2"}, 2);

  auto ack = controller.setEnabled("voice", true, 0);
  assert(ack.accepted);
  assert(ack.state.primaryInstanceId == "harmony:voice:primary");
  assert(ack.state.desiredEnabled);
  assert(ack.state.effectiveEnabled);
  assert(ack.state.params.key == "C");
  assert(ack.state.params.scale == "Major");
  assert(ack.state.params.voice1 == "+3rd");
  assert(ack.state.params.voice2 == "+5th");
  assert(ack.state.params.levelDb == 0.0f);

  ack = controller.setEnabled("voice", false, ack.state.revision);
  assert(ack.accepted);
  assert(!ack.state.effectiveEnabled);
  assert(!ack.state.primaryInstanceId.empty());

  ack = controller.setEnabled("voice", true, 0);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::HarmonyCommandError::revisionConflict);

  ack = controller.setEnabled("music", true, 0);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::HarmonyCommandError::roleUnsupported);

  ack = controller.setEnabled("full", true, 0);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::HarmonyCommandError::rackFull);

  ack = controller.setEnabled("multi", true, 0);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::HarmonyCommandError::multipleCandidates);

  auto snapshot = controller.snapshot("voice");
  assert(snapshot.has_value());
  localmixer::engine::HarmonyQuickParams params;
  params.key = "D";
  params.scale = "Natural Minor";
  params.mode = "Diatonic";
  params.voice1 = "+3rd";
  params.voice2 = "+5th";
  params.levelDb = -3.0f;
  ack = controller.configure("voice", params, snapshot->revision);
  assert(ack.accepted);
  assert(ack.state.params.key == "D");
  assert(ack.state.params.levelDb == -3.0f);

  std::cout << "channel harmony controller ok\n";
  return 0;
}
