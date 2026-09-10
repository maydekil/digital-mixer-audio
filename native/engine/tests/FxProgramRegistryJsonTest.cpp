#include "engine/FxProgramRegistryJson.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
  const auto json = localmixer::engine::protocol::fxProgramBankJsonFields();
  assert(json.find("\"bankVersion\":1") != std::string::npos);
  assert(json.find("\"id\":12") != std::string::npos);
  assert(json.find("\"name\":\"Vocal Plate\"") != std::string::npos);
  assert(json.find("\"macro1\":{\"label\":\"Decay\",\"value\":\"1.4 s\"}") != std::string::npos);
  assert(json.find("\"id\":99") != std::string::npos);
  assert(json.find("\"processorType\":\"delay_plate_parallel\"") != std::string::npos);
  std::cout << "fx program registry json ok\n";
  return 0;
}
