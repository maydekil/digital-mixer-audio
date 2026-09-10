#include "engine/MixerControlQueue.hpp"

namespace localmixer::engine {

MixerControlQueue::MixerControlQueue(std::size_t capacity) : buffer_(capacity) {}

MixerError MixerControlQueue::push(const MixerCommand& command) {
  if (size_ == buffer_.size()) return MixerError::queueFull;
  buffer_[tail_] = command;
  tail_ = (tail_ + 1) % buffer_.size();
  size_ += 1;
  return MixerError::none;
}

std::optional<MixerCommand> MixerControlQueue::pop() {
  if (empty()) return std::nullopt;
  const auto command = buffer_[head_];
  head_ = (head_ + 1) % buffer_.size();
  size_ -= 1;
  return command;
}

bool MixerControlQueue::empty() const {
  return size_ == 0;
}

std::size_t MixerControlQueue::size() const {
  return size_;
}

std::size_t MixerControlQueue::capacity() const {
  return buffer_.size();
}

}  // namespace localmixer::engine
