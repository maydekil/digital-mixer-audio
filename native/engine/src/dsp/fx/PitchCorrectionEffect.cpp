#include "dsp/fx/PitchCorrectionEffect.hpp"

#include <algorithm>
#include <cmath>

namespace localmixer::dsp::fx {

PitchCorrectionEffect::PitchCorrectionEffect(PitchCorrectionConfig config) : config_(config) {}

void PitchCorrectionEffect::prepare(const ProcessSpec& spec) {
  spec_ = spec;
  detector_.prepare(PitchDetectorConfig{.sampleRate = spec.sampleRate});
  analysisRing_.assign(detector_.windowFrames(), 0.0f);
  analysisWindow_.assign(detector_.windowFrames(), 0.0f);
  backend_ = makeRubberBandPitchBackend();
  backend_->prepare(PitchBackendSpec{.sampleRate = spec.sampleRate, .channels = 1, .preserveFormants = true});
  backendInput_.assign(backend_->blockSize(), 0.0f);
  backendOutput_.assign(backend_->blockSize(), 0.0f);
  hopFrames_ = 256;
  reset();
}

void PitchCorrectionEffect::reset() noexcept {
  std::fill(analysisRing_.begin(), analysisRing_.end(), 0.0f);
  std::fill(analysisWindow_.begin(), analysisWindow_.end(), 0.0f);
  std::fill(backendInput_.begin(), backendInput_.end(), 0.0f);
  std::fill(backendOutput_.begin(), backendOutput_.end(), 0.0f);
  analysisWrite_ = 0;
  samplesSinceAnalysis_ = 0;
  filledAnalysis_ = 0;
  currentSemitones_ = 0.0f;
  targetSemitones_ = 0.0f;
  lastEstimate_ = {};
  lastTarget_ = {};
  if (backend_) backend_->reset();
}

void PitchCorrectionEffect::process(AudioBlockView& block, const ProcessContext& context) noexcept {
  const auto frames = std::min(block.left.size(), block.right.size());
  for (std::size_t index = 0; index < frames; index += 1) {
    const auto mono = (block.left[index] + block.right[index]) * 0.5f;
    pushAnalysisSample(mono, context.absoluteFrame + index);
    analyzeIfReady(context.absoluteFrame + index);
  }

  const auto backendFrames = backend_ ? backend_->blockSize() : 0;
  if (backendFrames == 0) return;
  for (std::size_t offset = 0; offset < frames; offset += backendFrames) {
    const auto count = std::min<std::size_t>(backendFrames, frames - offset);
    if (!processBackendBlock(block, offset, count)) return;
  }
}

void PitchCorrectionEffect::applyRealtimeParameter(ParameterId parameter, float value) noexcept {
  if (parameter == 1) config_.keySemitone = std::clamp(static_cast<int>(std::lround(value)), 0, 11);
  if (parameter == 2) config_.amount = std::clamp(value, 0.0f, 1.0f);
  if (parameter == 3) config_.retuneMs = std::clamp(value, 0.0f, 300.0f);
  if (parameter == 4) config_.toleranceCents = std::clamp(value, 0.0f, 50.0f);
}

std::uint32_t PitchCorrectionEffect::latencySamples() const noexcept {
  return backend_ ? backend_->startDelaySamples() + detector_.windowFrames() / 2 : 0;
}

std::uint64_t PitchCorrectionEffect::maximumTailSamples() const noexcept {
  return latencySamples() + detector_.windowFrames();
}

void PitchCorrectionEffect::pushAnalysisSample(float sample, std::uint64_t) noexcept {
  if (analysisRing_.empty()) return;
  analysisRing_[analysisWrite_] = sample;
  analysisWrite_ = (analysisWrite_ + 1) % analysisRing_.size();
  samplesSinceAnalysis_ += 1;
  filledAnalysis_ = std::min<std::uint32_t>(filledAnalysis_ + 1, static_cast<std::uint32_t>(analysisRing_.size()));
}

void PitchCorrectionEffect::analyzeIfReady(std::uint64_t frame) noexcept {
  if (filledAnalysis_ < analysisRing_.size() || samplesSinceAnalysis_ < hopFrames_) return;
  samplesSinceAnalysis_ = 0;
  for (std::size_t index = 0; index < analysisRing_.size(); index += 1) {
    analysisWindow_[index] = analysisRing_[(analysisWrite_ + index) % analysisRing_.size()];
  }

  lastEstimate_ = detector_.analyze(analysisWindow_, frame - analysisRing_.size() + 1);
  if (!lastEstimate_.voiced || lastEstimate_.confidence < config_.confidenceThreshold) {
    lastTarget_ = {};
    targetSemitones_ = 0.0f;
    return;
  }

  lastTarget_ = mapPitchToScale(lastEstimate_.frequencyHz, ScaleMapperConfig{
    .keySemitone = config_.keySemitone,
    .scale = config_.scale,
    .a4Hz = config_.a4Hz,
    .toleranceCents = config_.toleranceCents,
    .amount = config_.amount,
  });
  targetSemitones_ = std::clamp(lastTarget_.correctionSemitones, -12.0f, 12.0f);
}

float PitchCorrectionEffect::nextSmoothedSemitones() noexcept {
  if (config_.retuneMs <= 0.0f) {
    currentSemitones_ = targetSemitones_;
    return currentSemitones_;
  }
  const auto retuneSamples = std::max(1.0f, static_cast<float>(spec_.sampleRate) * config_.retuneMs * 0.001f);
  const auto step = std::clamp(1.0f / retuneSamples, 0.00001f, 1.0f);
  currentSemitones_ += (targetSemitones_ - currentSemitones_) * step;
  return currentSemitones_;
}

bool PitchCorrectionEffect::processBackendBlock(AudioBlockView& block, std::size_t offset, std::size_t frames) noexcept {
  if (!backend_ || frames != backend_->blockSize()) return true;
  const auto semitones = nextSmoothedSemitones();
  backend_->setPitchSemitones(semitones);
  for (std::size_t index = 0; index < frames; index += 1) {
    backendInput_[index] = (block.left[offset + index] + block.right[offset + index]) * 0.5f;
  }
  if (!backend_->processBlock(backendInput_, backendOutput_)) return false;
  for (std::size_t index = 0; index < frames; index += 1) {
    block.left[offset + index] = backendOutput_[index];
    block.right[offset + index] = backendOutput_[index];
  }
  return true;
}

}  // namespace localmixer::dsp::fx
