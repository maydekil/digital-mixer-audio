#pragma once

#include "engine/FxProgramRegistry.hpp"
#include "engine/FxSendReturnBus.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace localmixer::engine {

enum class FxProgramError {
  none,
  invalidProgram,
  revisionConflict,
  prepareFailed,
};

struct FxProgramUnitSnapshot {
  std::uint32_t programId = 0;
  std::uint32_t previousProgramId = 0;
  std::uint64_t revision = 0;
  bool modified = false;
  bool pending = false;
  bool transitionActive = false;
  std::uint32_t crossfadeFramesRemaining = 0;
};

struct FxProgramAck {
  bool accepted = false;
  bool applied = false;
  FxProgramError error = FxProgramError::none;
  FxBusId unit = FxBusId::a;
  std::uint32_t requestedProgramId = 0;
  FxProgramUnitSnapshot snapshot;
};

using FxProgramPrepare = std::function<bool(const FxFactoryProgram&)>;

class FxProgramController {
 public:
  explicit FxProgramController(std::uint32_t crossfadeFrames = 960);

  FxProgramAck requestProgram(FxBusId unit, std::uint32_t programId, std::uint64_t expectedRevision);
  FxProgramAck processPending(FxBusId unit, const FxProgramPrepare& prepare);
  FxProgramAck setMacroOverride(FxBusId unit, std::uint64_t expectedRevision);
  FxProgramAck resetMacros(FxBusId unit, std::uint64_t expectedRevision);
  void advance(std::uint32_t frames) noexcept;

  FxProgramUnitSnapshot snapshot(FxBusId unit) const noexcept;
  std::uint32_t pendingRequestCount(FxBusId unit) const noexcept;

 private:
  struct UnitState {
    FxProgramUnitSnapshot snapshot;
    std::optional<std::uint32_t> desiredProgramId;
  };

  FxProgramAck reject(FxBusId unit, std::uint32_t programId, FxProgramError error) const noexcept;
  UnitState& state(FxBusId unit) noexcept;
  const UnitState& state(FxBusId unit) const noexcept;
  bool revisionMatches(const UnitState& unit, std::uint64_t expectedRevision) const noexcept;

  UnitState unitA_;
  UnitState unitB_;
  std::uint32_t crossfadeFrames_ = 960;
};

const char* fxProgramErrorName(FxProgramError error) noexcept;
const char* fxUnitName(FxBusId unit) noexcept;
std::optional<FxBusId> fxUnitFromName(const std::string& name) noexcept;

}  // namespace localmixer::engine
