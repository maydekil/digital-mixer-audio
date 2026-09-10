#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace localmixer::engine {

enum class FxProgramFamily {
  room,
  plate,
  hall,
  slapback,
  stereoDelay,
  pingPong,
  chorus,
  phaser,
  delayPlate,
};

struct FxProgramParameter {
  std::string_view id;
  double value = 0.0;
  std::string_view unit;
};

struct FxProgramRecipe {
  std::string_view processorType;
  std::vector<FxProgramParameter> parameters;
};

struct FxFactoryProgram {
  std::uint32_t bankVersion = 1;
  std::uint32_t programId = 0;
  std::string_view name;
  FxProgramFamily family = FxProgramFamily::room;
  std::string_view macro1Label;
  double macro1Value = 0.0;
  std::string_view macro1Unit;
  std::string_view macro2Label;
  double macro2Value = 0.0;
  std::string_view macro2Unit;
  FxProgramRecipe recipe;
};

std::vector<FxFactoryProgram> buildFxFactoryBank();
std::optional<FxFactoryProgram> findFxFactoryProgram(std::uint32_t programId);
std::string_view fxProgramFamilyName(FxProgramFamily family) noexcept;

}  // namespace localmixer::engine
