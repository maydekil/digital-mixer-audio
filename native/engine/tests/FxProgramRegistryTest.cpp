#include "engine/FxProgramRegistry.hpp"

#include <cmath>
#include <iostream>
#include <set>

namespace {

using localmixer::engine::FxProgramFamily;
using localmixer::engine::buildFxFactoryBank;
using localmixer::engine::findFxFactoryProgram;
using localmixer::engine::fxProgramFamilyName;

bool near(double value, double expected) {
  return std::fabs(value - expected) < 0.0001;
}

bool hasParam(std::uint32_t programId, std::string_view id, double value) {
  const auto program = findFxFactoryProgram(programId);
  if (!program.has_value()) return false;
  for (const auto& parameter : program->recipe.parameters) {
    if (parameter.id == id && near(parameter.value, value)) return true;
  }
  return false;
}

}  // namespace

int main() {
  const auto bank = buildFxFactoryBank();
  if (bank.size() != 99) {
    std::cerr << "factory FX bank should contain exactly 99 programs\n";
    return 1;
  }

  std::set<std::uint32_t> ids;
  for (const auto& program : bank) {
    ids.insert(program.programId);
    if (program.bankVersion != 1 || program.programId < 1 || program.programId > 99 || program.name.empty() ||
        program.recipe.processorType.empty() || program.recipe.parameters.empty()) {
      std::cerr << "factory FX program missing required schema fields\n";
      return 1;
    }
    if (!hasParam(program.programId, "wet_pct", 100) || !hasParam(program.programId, "output_db", 0)) {
      std::cerr << "factory FX program should expand wet-only recipe and output trim\n";
      return 1;
    }
  }
  if (ids.size() != 99 || *ids.begin() != 1 || *ids.rbegin() != 99) {
    std::cerr << "factory FX program IDs should be unique and contiguous 1-99\n";
    return 1;
  }

  const auto vocalPlate = findFxFactoryProgram(12);
  const auto stereo320 = findFxFactoryProgram(50);
  const auto infiniteMood = findFxFactoryProgram(99);
  if (!vocalPlate.has_value() || vocalPlate->name != "Vocal Plate" || vocalPlate->family != FxProgramFamily::plate ||
      vocalPlate->macro1Label != "Decay" || !near(vocalPlate->macro1Value, 1.4) ||
      !hasParam(12, "damping_hz", 6000)) {
    std::cerr << "program 12 should match Vocal Plate contract\n";
    return 1;
  }
  if (!stereo320.has_value() || stereo320->name != "Stereo 320" || stereo320->family != FxProgramFamily::stereoDelay ||
      stereo320->macro1Label != "Time" || !near(stereo320->macro1Value, 320) ||
      !hasParam(50, "feedback_pct", 25)) {
    std::cerr << "program 50 should match Stereo 320 contract\n";
    return 1;
  }
  if (!infiniteMood.has_value() || infiniteMood->name != "Infinite Mood" || infiniteMood->family != FxProgramFamily::delayPlate ||
      !hasParam(99, "plate_decay_s", 6.0) || !hasParam(99, "delay_feedback_pct", 20)) {
    std::cerr << "program 99 should remain bounded Delay + Plate, not infinite feedback\n";
    return 1;
  }
  if (fxProgramFamilyName(FxProgramFamily::pingPong) != "Ping-pong") {
    std::cerr << "family names should be stable for protocol/UI reports\n";
    return 1;
  }

  std::cout << "local-mixer-fx-program-registry-tests ok\n";
  return 0;
}
