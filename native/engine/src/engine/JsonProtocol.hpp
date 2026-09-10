#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace localmixer::engine::protocol {

std::string escapeJson(const std::string& text);
std::string readJsonStringField(const std::string& line, const std::string& field);
std::optional<double> readJsonNumberField(const std::string& line, const std::string& field);
std::optional<bool> readJsonBoolField(const std::string& line, const std::string& field);
void writeResponse(const std::string& id, bool ok, const std::string& type, const std::string& error = "");
void writeRawResponse(const std::string& id, bool ok, const std::string& type, const std::string& fields);

}  // namespace localmixer::engine::protocol
