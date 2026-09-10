#include "engine/SystemRouteRecovery.hpp"

#include <fstream>
#include <sstream>
#include <utility>

namespace localmixer::engine {
namespace {

constexpr const char* kMarkerHeader = "LOCAL_MIXER_SYSTEM_ROUTE_V1";

std::string encode(const std::string& value) {
  std::string out;
  out.reserve(value.size());
  for (const unsigned char c : value) {
    if (c == '%' || c == '\n' || c == '\r' || c == '=') {
      constexpr char hex[] = "0123456789ABCDEF";
      out.push_back('%');
      out.push_back(hex[(c >> 4) & 0x0F]);
      out.push_back(hex[c & 0x0F]);
    } else {
      out.push_back(static_cast<char>(c));
    }
  }
  return out;
}

int hexValue(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

std::string decode(const std::string& value) {
  std::string out;
  out.reserve(value.size());
  for (std::size_t index = 0; index < value.size(); index += 1) {
    if (value[index] == '%' && index + 2 < value.size()) {
      const auto high = hexValue(value[index + 1]);
      const auto low = hexValue(value[index + 2]);
      if (high >= 0 && low >= 0) {
        out.push_back(static_cast<char>((high << 4) | low));
        index += 2;
        continue;
      }
    }
    out.push_back(value[index]);
  }
  return out;
}

std::string valueFor(const std::string& text, const std::string& key) {
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    const auto split = line.find('=');
    if (split == std::string::npos) continue;
    if (line.substr(0, split) == key) return decode(line.substr(split + 1));
  }
  return "";
}

}  // namespace

SystemRouteRecoveryStore::SystemRouteRecoveryStore(std::filesystem::path path) : path_(std::move(path)) {}

bool SystemRouteRecoveryStore::save(const SystemRouteTransaction& transaction) const {
  if (!transaction.ownsSystemRoute || transaction.originalOutputUid.empty()) return false;
  std::error_code error;
  const auto parent = path_.parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent, error);
    if (error) return false;
  }

  std::ofstream file(path_, std::ios::trunc);
  if (!file) return false;
  file << kMarkerHeader << "\n";
  file << "originalOutputUid=" << encode(transaction.originalOutputUid) << "\n";
  file << "blackHoleUid=" << encode(transaction.selection.blackHoleUid) << "\n";
  file << "physicalOutputUid=" << encode(transaction.selection.physicalOutputUid) << "\n";
  return static_cast<bool>(file);
}

std::optional<SystemRouteRecoveryMarker> SystemRouteRecoveryStore::load() const {
  std::ifstream file(path_);
  if (!file) return std::nullopt;
  std::ostringstream buffer;
  buffer << file.rdbuf();
  const auto text = buffer.str();
  if (text.find(kMarkerHeader) != 0) return std::nullopt;

  SystemRouteRecoveryMarker marker{
    .originalOutputUid = valueFor(text, "originalOutputUid"),
    .selection = {
      .blackHoleUid = valueFor(text, "blackHoleUid"),
      .physicalOutputUid = valueFor(text, "physicalOutputUid"),
    },
  };
  if (marker.originalOutputUid.empty()) return std::nullopt;
  return marker;
}

bool SystemRouteRecoveryStore::clear() const {
  std::error_code error;
  if (!std::filesystem::exists(path_, error)) return true;
  return std::filesystem::remove(path_, error) && !error;
}

const std::filesystem::path& SystemRouteRecoveryStore::path() const {
  return path_;
}

}  // namespace localmixer::engine
