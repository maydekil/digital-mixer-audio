#include "engine/FxProgramRegistryJson.hpp"

#include "engine/FxProgramRegistry.hpp"

#include <cmath>
#include <string>

namespace localmixer::engine::protocol {
namespace {

std::string escape(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size());
  for (const char c : text) {
    if (c == '"') escaped += "\\\"";
    else if (c == '\\') escaped += "\\\\";
    else if (c == '\n') escaped += "\\n";
    else if (c == '\r') escaped += "\\r";
    else if (c == '\t') escaped += "\\t";
    else escaped += c;
  }
  return escaped;
}

std::string formatNumber(double value) {
  if (std::fabs(value - std::round(value)) < 0.000001) {
    return std::to_string(static_cast<int>(std::round(value)));
  }
  std::string text = std::to_string(value);
  while (!text.empty() && text.back() == '0') text.pop_back();
  if (!text.empty() && text.back() == '.') text.pop_back();
  return text;
}

std::string displayValue(double value, std::string_view unit) {
  auto text = formatNumber(value);
  if (!unit.empty()) text += " " + std::string(unit);
  return text;
}

}  // namespace

std::string fxProgramBankJsonFields() {
  const auto bank = buildFxFactoryBank();
  std::string json = "\"bankVersion\":1,\"programs\":[";
  for (std::size_t index = 0; index < bank.size(); index += 1) {
    const auto& program = bank[index];
    if (index > 0) json += ",";
    json += "{\"id\":" + std::to_string(program.programId);
    json += ",\"name\":\"" + escape(program.name) + "\"";
    json += ",\"family\":\"" + escape(fxProgramFamilyName(program.family)) + "\"";
    json += ",\"processorType\":\"" + escape(program.recipe.processorType) + "\"";
    json += ",\"macro1\":{\"label\":\"" + escape(program.macro1Label) + "\"";
    json += ",\"value\":\"" + escape(displayValue(program.macro1Value, program.macro1Unit)) + "\"}";
    json += ",\"macro2\":{\"label\":\"" + escape(program.macro2Label) + "\"";
    json += ",\"value\":\"" + escape(displayValue(program.macro2Value, program.macro2Unit)) + "\"}";
    json += "}";
  }
  json += "]";
  return json;
}

}  // namespace localmixer::engine::protocol
