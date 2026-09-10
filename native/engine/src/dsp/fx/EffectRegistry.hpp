#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace localmixer::dsp::fx {

enum class EffectAvailability {
  unavailable,
  implementedUnverified,
  verified,
};

enum class EffectCategory {
  space,
  time,
  modulation,
  pitch,
  character,
  synth,
};

enum class ChannelFormat {
  mono,
  stereo,
  monoToStereo,
};

enum class ParameterKind {
  floatValue,
  choice,
  boolean,
};

enum class FxError {
  none,
  unknownEffect,
  unavailable,
  slotLimit,
  incompatibleFormat,
  staleRevision,
  factoryFailed,
};

struct ParameterDescriptor {
  std::string_view id;
  std::string_view displayName;
  ParameterKind kind = ParameterKind::floatValue;
  float minValue = 0.0f;
  float maxValue = 1.0f;
  float defaultValue = 0.0f;
  std::string_view unit;
};

struct EffectDescriptor {
  std::string_view effectType;
  std::string_view displayName;
  EffectCategory category = EffectCategory::space;
  std::uint32_t version = 1;
  ChannelFormat inputFormat = ChannelFormat::mono;
  ChannelFormat outputFormat = ChannelFormat::mono;
  EffectAvailability availability = EffectAvailability::unavailable;
  std::string_view ownerModule;
  std::string_view implementationPhase;
  std::string_view testPlan;
  std::span<const ParameterDescriptor> parameters;
};

std::span<const EffectDescriptor> effectCatalog();
std::optional<EffectDescriptor> findEffect(std::string_view effectType);
const char* availabilityName(EffectAvailability availability);
const char* fxErrorName(FxError error);

}  // namespace localmixer::dsp::fx
