#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace localmixer::engine {

struct StripId {
  std::uint32_t value = 0;
  friend bool operator==(StripId left, StripId right) { return left.value == right.value; }
};

enum class MixerError {
  none,
  graphFull,
  staleStripId,
  invalidSource,
  invalidBuffer,
  queueFull,
};

enum class MixerCommandType {
  setLevel,
  setMute,
  setSolo,
  setEnabled,
  setInputMonitoring,
};

struct MixerCommand {
  MixerCommandType type = MixerCommandType::setLevel;
  StripId stripId;
  float trimDb = 0.0f;
  float faderDb = 0.0f;
  float pan = 0.0f;
  bool boolValue = false;
};

class MixerControlQueue {
 public:
  explicit MixerControlQueue(std::size_t capacity);

  MixerError push(const MixerCommand& command);
  std::optional<MixerCommand> pop();
  bool empty() const;
  std::size_t size() const;
  std::size_t capacity() const;

 private:
  std::vector<MixerCommand> buffer_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
};

}  // namespace localmixer::engine
