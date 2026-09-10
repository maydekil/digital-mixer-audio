#include "engine/FxProgramController.hpp"

#include <algorithm>

namespace localmixer::engine {
namespace {

FxProgramUnitSnapshot makeDefault(std::uint32_t programId) {
  return {.programId = programId};
}

}  // namespace

FxProgramController::FxProgramController(std::uint32_t crossfadeFrames)
  : unitA_{.snapshot = makeDefault(12)},
    unitB_{.snapshot = makeDefault(50)},
    crossfadeFrames_(crossfadeFrames) {}

FxProgramAck FxProgramController::requestProgram(FxBusId unit, std::uint32_t programId, std::uint64_t expectedRevision) {
  auto& current = state(unit);
  if (!findFxFactoryProgram(programId).has_value()) return reject(unit, programId, FxProgramError::invalidProgram);
  if (!revisionMatches(current, expectedRevision)) return reject(unit, programId, FxProgramError::revisionConflict);
  current.desiredProgramId = programId;
  current.snapshot.pending = true;
  return {.accepted = true, .applied = false, .unit = unit, .requestedProgramId = programId, .snapshot = current.snapshot};
}

FxProgramAck FxProgramController::processPending(FxBusId unit, const FxProgramPrepare& prepare) {
  auto& current = state(unit);
  if (!current.desiredProgramId.has_value()) {
    return {.accepted = true, .applied = false, .unit = unit, .requestedProgramId = current.snapshot.programId, .snapshot = current.snapshot};
  }

  const auto programId = *current.desiredProgramId;
  const auto program = findFxFactoryProgram(programId);
  if (!program.has_value()) {
    current.desiredProgramId.reset();
    current.snapshot.pending = false;
    return reject(unit, programId, FxProgramError::invalidProgram);
  }

  if (prepare && !prepare(*program)) {
    current.desiredProgramId.reset();
    current.snapshot.pending = false;
    return reject(unit, programId, FxProgramError::prepareFailed);
  }

  current.desiredProgramId.reset();
  current.snapshot.previousProgramId = current.snapshot.programId;
  current.snapshot.programId = programId;
  current.snapshot.revision += 1;
  current.snapshot.modified = false;
  current.snapshot.pending = false;
  current.snapshot.transitionActive = current.snapshot.previousProgramId != current.snapshot.programId;
  current.snapshot.crossfadeFramesRemaining = current.snapshot.transitionActive ? crossfadeFrames_ : 0;
  return {.accepted = true, .applied = true, .unit = unit, .requestedProgramId = programId, .snapshot = current.snapshot};
}

FxProgramAck FxProgramController::setMacroOverride(FxBusId unit, std::uint64_t expectedRevision) {
  auto& current = state(unit);
  if (!revisionMatches(current, expectedRevision)) return reject(unit, current.snapshot.programId, FxProgramError::revisionConflict);
  current.snapshot.modified = true;
  current.snapshot.revision += 1;
  return {.accepted = true, .applied = true, .unit = unit, .requestedProgramId = current.snapshot.programId, .snapshot = current.snapshot};
}

FxProgramAck FxProgramController::resetMacros(FxBusId unit, std::uint64_t expectedRevision) {
  auto& current = state(unit);
  if (!revisionMatches(current, expectedRevision)) return reject(unit, current.snapshot.programId, FxProgramError::revisionConflict);
  current.snapshot.modified = false;
  current.snapshot.revision += 1;
  return {.accepted = true, .applied = true, .unit = unit, .requestedProgramId = current.snapshot.programId, .snapshot = current.snapshot};
}

void FxProgramController::advance(std::uint32_t frames) noexcept {
  for (auto* unit : {&unitA_, &unitB_}) {
    unit->snapshot.crossfadeFramesRemaining = frames >= unit->snapshot.crossfadeFramesRemaining ? 0 : unit->snapshot.crossfadeFramesRemaining - frames;
    unit->snapshot.transitionActive = unit->snapshot.crossfadeFramesRemaining > 0;
  }
}

FxProgramUnitSnapshot FxProgramController::snapshot(FxBusId unit) const noexcept {
  return state(unit).snapshot;
}

std::uint32_t FxProgramController::pendingRequestCount(FxBusId unit) const noexcept {
  return state(unit).desiredProgramId.has_value() ? 1 : 0;
}

FxProgramAck FxProgramController::reject(FxBusId unit, std::uint32_t programId, FxProgramError error) const noexcept {
  return {.accepted = false, .applied = false, .error = error, .unit = unit, .requestedProgramId = programId, .snapshot = state(unit).snapshot};
}

FxProgramController::UnitState& FxProgramController::state(FxBusId unit) noexcept {
  return unit == FxBusId::a ? unitA_ : unitB_;
}

const FxProgramController::UnitState& FxProgramController::state(FxBusId unit) const noexcept {
  return unit == FxBusId::a ? unitA_ : unitB_;
}

bool FxProgramController::revisionMatches(const UnitState& unit, std::uint64_t expectedRevision) const noexcept {
  return expectedRevision == unit.snapshot.revision;
}

const char* fxProgramErrorName(FxProgramError error) noexcept {
  switch (error) {
    case FxProgramError::none: return "";
    case FxProgramError::invalidProgram: return "INVALID_PROGRAM";
    case FxProgramError::revisionConflict: return "REVISION_CONFLICT";
    case FxProgramError::prepareFailed: return "PREPARE_FAILED";
  }
  return "UNKNOWN";
}

const char* fxUnitName(FxBusId unit) noexcept {
  return unit == FxBusId::a ? "fx-a" : "fx-b";
}

std::optional<FxBusId> fxUnitFromName(const std::string& name) noexcept {
  if (name == "fx-a") return FxBusId::a;
  if (name == "fx-b") return FxBusId::b;
  return std::nullopt;
}

}  // namespace localmixer::engine
