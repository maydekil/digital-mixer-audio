#include "engine/FxProgramController.hpp"

#include <cassert>
#include <iostream>

int main() {
  localmixer::engine::FxProgramController controller(480);

  assert(controller.snapshot(localmixer::engine::FxBusId::a).programId == 12);
  assert(controller.snapshot(localmixer::engine::FxBusId::b).programId == 50);

  for (std::uint32_t index = 0; index < 100; index += 1) {
    const auto programId = 1 + (index % 99);
    const auto ack = controller.requestProgram(localmixer::engine::FxBusId::a, programId, 0);
    assert(ack.accepted);
  }
  assert(controller.pendingRequestCount(localmixer::engine::FxBusId::a) == 1);
  assert(controller.snapshot(localmixer::engine::FxBusId::a).programId == 12);

  auto ack = controller.processPending(localmixer::engine::FxBusId::a, [](const auto& program) {
    return program.programId == 1;
  });
  assert(ack.applied);
  assert(ack.snapshot.programId == 1);
  assert(ack.snapshot.previousProgramId == 12);
  assert(ack.snapshot.crossfadeFramesRemaining == 480);

  controller.advance(240);
  assert(controller.snapshot(localmixer::engine::FxBusId::a).transitionActive);
  controller.advance(240);
  assert(!controller.snapshot(localmixer::engine::FxBusId::a).transitionActive);

  ack = controller.requestProgram(localmixer::engine::FxBusId::a, 12, 999);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::FxProgramError::revisionConflict);

  const auto revision = controller.snapshot(localmixer::engine::FxBusId::a).revision;
  ack = controller.setMacroOverride(localmixer::engine::FxBusId::a, revision);
  assert(ack.applied);
  assert(ack.snapshot.modified);
  ack = controller.resetMacros(localmixer::engine::FxBusId::a, ack.snapshot.revision);
  assert(ack.applied);
  assert(!ack.snapshot.modified);

  ack = controller.requestProgram(localmixer::engine::FxBusId::b, 12, 0);
  assert(ack.accepted);
  ack = controller.processPending(localmixer::engine::FxBusId::b, [](const auto&) {
    return false;
  });
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::FxProgramError::prepareFailed);
  assert(controller.snapshot(localmixer::engine::FxBusId::b).programId == 50);

  ack = controller.requestProgram(localmixer::engine::FxBusId::a, 100, 0);
  assert(!ack.accepted);
  assert(ack.error == localmixer::engine::FxProgramError::invalidProgram);

  std::cout << "fx program controller ok\n";
  return 0;
}
