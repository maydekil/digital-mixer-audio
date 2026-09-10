#include "engine/FxProgramWetProcessor.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::engine {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr std::uint32_t kDefaultMaximumFrames = 4096;

float pctToUnit(double value) {
  return static_cast<float>(std::clamp(value, 0.0, 100.0) * 0.01);
}

}  // namespace

FxProgramWetProcessor::FxProgramWetProcessor(double sampleRate) : sampleRate_(sampleRate) {
  prepare(kDefaultMaximumFrames);
}

bool FxProgramWetProcessor::configure(std::uint32_t programId) {
  const auto program = findFxFactoryProgram(programId);
  if (!program.has_value()) return false;
  program_ = program;
  programId_ = programId;

  const auto type = program_->recipe.processorType;
  if (type == "reverb") {
    reverb_.configure(dsp::ReverbConfig{
      .sampleRate = sampleRate_,
      .preDelayMs = static_cast<float>(parameter("predelay_ms", 20.0)),
      .decay = static_cast<float>(std::clamp(parameter("decay_s", 1.0) / 7.0, 0.05, 0.95)),
      .wetGain = 1.0f,
    });
  } else if (type == "delay_plate_parallel") {
    delay_.configure(dsp::DelayConfig{
      .sampleRate = sampleRate_,
      .delayMs = static_cast<float>(parameter("delay_time_ms", 250.0)),
      .feedback = pctToUnit(parameter("delay_feedback_pct", 20.0)),
      .wetGain = 1.0f,
      .tone = 0.35f,
    });
    reverb_.configure(dsp::ReverbConfig{
      .sampleRate = sampleRate_,
      .preDelayMs = static_cast<float>(parameter("plate_predelay_ms", 20.0)),
      .decay = static_cast<float>(std::clamp(parameter("plate_decay_s", 1.0) / 7.0, 0.05, 0.95)),
      .wetGain = 1.0f,
    });
  } else if (type == "slapback_delay" || type == "stereo_delay" || type == "ping_pong_delay") {
    delay_.configure(dsp::DelayConfig{
      .sampleRate = sampleRate_,
      .delayMs = static_cast<float>(parameter("time_ms", 250.0)),
      .feedback = pctToUnit(parameter("feedback_pct", 20.0)),
      .wetGain = 1.0f,
      .tone = 0.35f,
    });
  }
  reset();
  return true;
}

void FxProgramWetProcessor::reset() {
  delay_.reset();
  reverb_.reset();
  std::fill(scratchA_.begin(), scratchA_.end(), 0.0f);
  std::fill(scratchB_.begin(), scratchB_.end(), 0.0f);
  phase_ = 0.0f;
}

void FxProgramWetProcessor::prepare(std::uint32_t maximumFrames) {
  maximumFrames_ = std::max<std::uint32_t>(1, maximumFrames);
  scratchA_.assign(maximumFrames_, 0.0f);
  scratchB_.assign(maximumFrames_, 0.0f);
}

void FxProgramWetProcessor::process(std::span<const float> input, std::span<float> left, std::span<float> right) {
  const auto count = std::min(input.size(), std::min(left.size(), right.size()));
  std::fill(left.begin(), left.begin() + static_cast<std::ptrdiff_t>(count), 0.0f);
  std::fill(right.begin(), right.begin() + static_cast<std::ptrdiff_t>(count), 0.0f);
  if (!program_.has_value() || count == 0) return;

  const auto type = program_->recipe.processorType;
  if (type == "reverb") {
    reverb_.processWet(input.first(count), left.first(count));
    std::copy(left.begin(), left.begin() + static_cast<std::ptrdiff_t>(count), right.begin());
  } else if (type == "delay_plate_parallel") {
    processDelayPlate(input.first(count), left.first(count), right.first(count));
  } else if (type == "slapback_delay" || type == "stereo_delay" || type == "ping_pong_delay") {
    delay_.processWet(input.first(count), left.first(count));
    std::copy(left.begin(), left.begin() + static_cast<std::ptrdiff_t>(count), right.begin());
    if (type == "ping_pong_delay") {
      for (std::size_t index = 1; index < count; index += 2) std::swap(left[index], right[index]);
    }
  } else {
    processModulatedCopy(input.first(count), left.first(count), right.first(count));
  }
}

FxWetProcessor FxProgramWetProcessor::callback() {
  return [this](std::span<const float> input, std::span<float> left, std::span<float> right) {
    process(input, left, right);
  };
}

std::uint32_t FxProgramWetProcessor::programId() const noexcept {
  return programId_;
}

double FxProgramWetProcessor::parameter(std::string_view id, double fallback) const {
  if (!program_.has_value()) return fallback;
  const auto it = std::find_if(program_->recipe.parameters.begin(), program_->recipe.parameters.end(), [&](const auto& parameter) {
    return parameter.id == id;
  });
  return it == program_->recipe.parameters.end() ? fallback : it->value;
}

void FxProgramWetProcessor::processDelayPlate(std::span<const float> input, std::span<float> left, std::span<float> right) {
  if (input.size() > scratchA_.size() || input.size() > scratchB_.size()) return;
  std::fill(scratchA_.begin(), scratchA_.begin() + static_cast<std::ptrdiff_t>(input.size()), 0.0f);
  std::fill(scratchB_.begin(), scratchB_.begin() + static_cast<std::ptrdiff_t>(input.size()), 0.0f);
  delay_.processWet(input, scratchA_);
  reverb_.processWet(input, scratchB_);
  const auto delayGain = static_cast<float>(parameter("delay_branch_gain", 0.5));
  const auto plateGain = static_cast<float>(parameter("plate_branch_gain", 0.5));
  for (std::size_t index = 0; index < input.size(); index += 1) {
    left[index] = scratchA_[index] * delayGain + scratchB_[index] * plateGain;
    right[index] = scratchA_[index] * delayGain - scratchB_[index] * plateGain;
  }
}

void FxProgramWetProcessor::processModulatedCopy(std::span<const float> input, std::span<float> left, std::span<float> right) {
  const auto rate = static_cast<float>(parameter("rate_hz", 0.3));
  const auto depth = pctToUnit(parameter("depth_pct", 25.0));
  for (std::size_t index = 0; index < input.size(); index += 1) {
    const auto lfo = std::sin(phase_);
    phase_ += 2.0f * kPi * rate / static_cast<float>(sampleRate_);
    if (phase_ > 2.0f * kPi) phase_ -= 2.0f * kPi;
    left[index] = input[index] * (0.5f + 0.5f * depth * lfo);
    right[index] = input[index] * (0.5f - 0.5f * depth * lfo);
  }
}

}  // namespace localmixer::engine
