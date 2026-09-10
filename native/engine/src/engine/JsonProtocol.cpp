#include "engine/JsonProtocol.hpp"

#include <iostream>

namespace localmixer::engine::protocol {

std::string escapeJson(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    if (c == '"' || c == '\\') out.push_back('\\');
    if (c >= 0 && c < 0x20) continue;
    out.push_back(c);
  }
  return out;
}

std::string readJsonStringField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return "";
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return "";
  const auto start = line.find('"', colon + 1);
  if (start == std::string::npos) return "";
  const auto end = line.find('"', start + 1);
  if (end == std::string::npos) return "";
  return line.substr(start + 1, end - start - 1);
}

std::optional<double> readJsonNumberField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return std::nullopt;
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return std::nullopt;
  const auto start = line.find_first_of("-0123456789", colon + 1);
  if (start == std::string::npos) return std::nullopt;
  const auto end = line.find_first_not_of("-0123456789.", start);
  try {
    return std::stod(line.substr(start, end - start));
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<bool> readJsonBoolField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return std::nullopt;
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return std::nullopt;
  if (line.find("true", colon + 1) == colon + 1) return true;
  if (line.find("false", colon + 1) == colon + 1) return false;
  return std::nullopt;
}

void writeResponse(const std::string& id, bool ok, const std::string& type, const std::string& error) {
  std::cout << "{\"id\":\"" << id << "\",\"type\":\"" << type << "\",\"ok\":" << (ok ? "true" : "false");
  if (!error.empty()) std::cout << ",\"error\":\"" << escapeJson(error) << "\"";
  std::cout << "}" << std::endl;
}

void writeRawResponse(const std::string& id, bool ok, const std::string& type, const std::string& fields) {
  std::cout << "{\"id\":\"" << id << "\",\"type\":\"" << type << "\",\"ok\":" << (ok ? "true" : "false");
  if (!fields.empty()) std::cout << "," << fields;
  std::cout << "}" << std::endl;
}

}  // namespace localmixer::engine::protocol
