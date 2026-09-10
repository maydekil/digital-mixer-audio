#include "dsp/fx/EffectRegistry.hpp"

#include <array>
#include <algorithm>

namespace localmixer::dsp::fx {
namespace {

constexpr std::array<ParameterDescriptor, 6> kReverbParams{{
  {"type", "Type", ParameterKind::choice, 0.0f, 2.0f, 1.0f, ""},
  {"decay_s", "Decay", ParameterKind::floatValue, 0.2f, 10.0f, 1.4f, "s"},
  {"predelay_ms", "Predelay", ParameterKind::floatValue, 0.0f, 100.0f, 20.0f, "ms"},
  {"damping_hz", "Damping", ParameterKind::floatValue, 500.0f, 18000.0f, 6000.0f, "Hz"},
  {"width_pct", "Width", ParameterKind::floatValue, 0.0f, 100.0f, 100.0f, "%"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 15.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 7> kDelayParams{{
  {"mode", "Mode", ParameterKind::choice, 0.0f, 3.0f, 1.0f, ""},
  {"time_ms", "Time", ParameterKind::floatValue, 1.0f, 2000.0f, 250.0f, "ms"},
  {"sync", "Sync", ParameterKind::boolean, 0.0f, 1.0f, 0.0f, ""},
  {"note", "Note", ParameterKind::choice, 0.0f, 12.0f, 4.0f, ""},
  {"feedback_pct", "Feedback", ParameterKind::floatValue, 0.0f, 90.0f, 20.0f, "%"},
  {"low_cut_hz", "Low Cut", ParameterKind::floatValue, 20.0f, 2000.0f, 120.0f, "Hz"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 12.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 5> kChorusParams{{
  {"rate_hz", "Rate", ParameterKind::floatValue, 0.05f, 5.0f, 0.6f, "Hz"},
  {"depth_pct", "Depth", ParameterKind::floatValue, 0.0f, 100.0f, 25.0f, "%"},
  {"centre_delay_ms", "Centre Delay", ParameterKind::floatValue, 7.0f, 30.0f, 12.0f, "ms"},
  {"feedback_pct", "Feedback", ParameterKind::floatValue, -70.0f, 70.0f, 0.0f, "%"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 20.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 6> kDoublerParams{{
  {"voice1_delay_ms", "Voice 1 Delay", ParameterKind::floatValue, 5.0f, 40.0f, 15.0f, "ms"},
  {"voice2_delay_ms", "Voice 2 Delay", ParameterKind::floatValue, 5.0f, 40.0f, 23.0f, "ms"},
  {"voice1_detune_cent", "Voice 1 Detune", ParameterKind::floatValue, -20.0f, 20.0f, -6.0f, "cent"},
  {"voice2_detune_cent", "Voice 2 Detune", ParameterKind::floatValue, -20.0f, 20.0f, 6.0f, "cent"},
  {"voice_level_db", "Voice Level", ParameterKind::floatValue, -60.0f, 0.0f, -9.0f, "dB"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 25.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 4> kPitchShiftParams{{
  {"semitones", "Semitones", ParameterKind::floatValue, -12.0f, 12.0f, 0.0f, "st"},
  {"fine_cent", "Fine", ParameterKind::floatValue, -100.0f, 100.0f, 0.0f, "cent"},
  {"formant_preserve", "Formant Preserve", ParameterKind::boolean, 0.0f, 1.0f, 1.0f, ""},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 100.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 2> kFormantShiftParams{{
  {"shift_st", "Shift", ParameterKind::floatValue, -6.0f, 6.0f, 0.0f, "st"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 100.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 7> kPitchCorrectParams{{
  {"key", "Key", ParameterKind::choice, 0.0f, 11.0f, 0.0f, ""},
  {"scale", "Scale", ParameterKind::choice, 0.0f, 3.0f, 1.0f, ""},
  {"a4_hz", "A4", ParameterKind::floatValue, 430.0f, 450.0f, 440.0f, "Hz"},
  {"retune_ms", "Retune", ParameterKind::floatValue, 0.0f, 300.0f, 80.0f, "ms"},
  {"amount_pct", "Amount", ParameterKind::floatValue, 0.0f, 100.0f, 70.0f, "%"},
  {"tolerance_cent", "Tolerance", ParameterKind::floatValue, 0.0f, 50.0f, 10.0f, "cent"},
  {"confidence", "Confidence", ParameterKind::floatValue, 0.0f, 1.0f, 0.65f, ""},
}};

constexpr std::array<ParameterDescriptor, 6> kHarmonyParams{{
  {"mode", "Mode", ParameterKind::choice, 0.0f, 1.0f, 1.0f, ""},
  {"voice1_interval", "Voice 1 Interval", ParameterKind::floatValue, -12.0f, 12.0f, 2.0f, "step"},
  {"voice2_interval", "Voice 2 Interval", ParameterKind::floatValue, -12.0f, 12.0f, 4.0f, "step"},
  {"voice1_level_db", "Voice 1 Level", ParameterKind::floatValue, -60.0f, 0.0f, -12.0f, "dB"},
  {"voice2_level_db", "Voice 2 Level", ParameterKind::floatValue, -60.0f, 0.0f, -15.0f, "dB"},
  {"formant_preserve", "Formant Preserve", ParameterKind::boolean, 0.0f, 1.0f, 1.0f, ""},
}};

constexpr std::array<ParameterDescriptor, 5> kSaturationParams{{
  {"mode", "Mode", ParameterKind::choice, 0.0f, 3.0f, 0.0f, ""},
  {"drive_db", "Drive", ParameterKind::floatValue, 0.0f, 24.0f, 3.0f, "dB"},
  {"tone", "Tone", ParameterKind::floatValue, 0.0f, 1.0f, 0.5f, ""},
  {"output_db", "Output", ParameterKind::floatValue, -24.0f, 6.0f, -3.0f, "dB"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 20.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 5> kFlangerParams{{
  {"rate_hz", "Rate", ParameterKind::floatValue, 0.05f, 5.0f, 0.25f, "Hz"},
  {"base_delay_ms", "Base Delay", ParameterKind::floatValue, 1.0f, 10.0f, 3.0f, "ms"},
  {"depth_pct", "Depth", ParameterKind::floatValue, 0.0f, 100.0f, 35.0f, "%"},
  {"feedback_pct", "Feedback", ParameterKind::floatValue, -90.0f, 90.0f, 25.0f, "%"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 15.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 5> kPhaserParams{{
  {"rate_hz", "Rate", ParameterKind::floatValue, 0.05f, 5.0f, 0.4f, "Hz"},
  {"centre_hz", "Centre", ParameterKind::floatValue, 100.0f, 4000.0f, 900.0f, "Hz"},
  {"depth_pct", "Depth", ParameterKind::floatValue, 0.0f, 100.0f, 30.0f, "%"},
  {"feedback_pct", "Feedback", ParameterKind::floatValue, -80.0f, 80.0f, 20.0f, "%"},
  {"mix_pct", "Mix", ParameterKind::floatValue, 0.0f, 100.0f, 20.0f, "%"},
}};

constexpr std::array<ParameterDescriptor, 7> kVocoderParams{{
  {"bands", "Bands", ParameterKind::floatValue, 16.0f, 16.0f, 16.0f, ""},
  {"carrier", "Carrier", ParameterKind::choice, 0.0f, 2.0f, 0.0f, ""},
  {"fixed_midi_note", "Fixed Note", ParameterKind::floatValue, 36.0f, 84.0f, 48.0f, "MIDI"},
  {"attack_ms", "Attack", ParameterKind::floatValue, 1.0f, 100.0f, 5.0f, "ms"},
  {"release_ms", "Release", ParameterKind::floatValue, 10.0f, 500.0f, 100.0f, "ms"},
  {"unvoiced_pct", "Unvoiced", ParameterKind::floatValue, 0.0f, 100.0f, 20.0f, "%"},
  {"wet_pct", "Wet", ParameterKind::floatValue, 0.0f, 100.0f, 100.0f, "%"},
}};

constexpr std::array<EffectDescriptor, 12> kCatalog{{
  {"reverb", "Reverb", EffectCategory::space, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::implementedUnverified, "dsp/fx/ReverbEffect", "VFX-02", "impulse decay/tail/wet-only"},
  {"delay", "Delay / Echo", EffectCategory::time, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::implementedUnverified, "dsp/fx/DelayEffect", "VFX-02", "repeat spacing/feedback/sync"},
  {"chorus", "Chorus", EffectCategory::modulation, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::unavailable, "dsp/fx/ModulationEffects", "VFX-02", "fractional delay modulation sweep"},
  {"doubler", "Vocal Doubler", EffectCategory::modulation, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::unavailable, "dsp/fx/DoublerEffect", "VFX-03", "two independent micro-delay voices"},
  {"pitch_shift", "Pitch Shift", EffectCategory::pitch, 1, ChannelFormat::mono, ChannelFormat::mono,
    EffectAvailability::unavailable, "dsp/fx/PitchBackend", "VFX-04", "steady frequency shift and latency"},
  {"formant_shift", "Formant Shift", EffectCategory::pitch, 1, ChannelFormat::mono, ChannelFormat::mono,
    EffectAvailability::unavailable, "dsp/fx/FormantEffect", "VFX-04", "formant envelope shift without F0 drift"},
  {"pitch_correct", "Pitch Correction", EffectCategory::pitch, 1, ChannelFormat::mono, ChannelFormat::mono,
    EffectAvailability::unavailable, "dsp/fx/PitchCorrectionEffect", "VFX-05", "detuned notes to target scale"},
  {"harmony", "Harmony", EffectCategory::pitch, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::unavailable, "dsp/fx/HarmonyEffect", "VFX-06", "fixed and diatonic interval accuracy"},
  {"saturation", "Saturation / Drive", EffectCategory::character, 1, ChannelFormat::mono, ChannelFormat::mono,
    EffectAvailability::unavailable, "dsp/fx/SaturationEffect", "VFX-03", "harmonics/output/DC suppression"},
  {"flanger", "Flanger", EffectCategory::modulation, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::unavailable, "dsp/fx/ModulationEffects", "VFX-02", "comb sweep and bounded feedback"},
  {"phaser", "Phaser", EffectCategory::modulation, 1, ChannelFormat::monoToStereo, ChannelFormat::stereo,
    EffectAvailability::unavailable, "dsp/fx/ModulationEffects", "VFX-02", "all-pass sweep and silence stability"},
  {"vocoder", "Vocoder / Robot", EffectCategory::synth, 1, ChannelFormat::mono, ChannelFormat::mono,
    EffectAvailability::unavailable, "dsp/fx/VocoderEffect", "VFX-07", "filter bank envelope and carrier notes"},
}};

constexpr std::array<std::span<const ParameterDescriptor>, 12> kParameterSpans{{
  kReverbParams,
  kDelayParams,
  kChorusParams,
  kDoublerParams,
  kPitchShiftParams,
  kFormantShiftParams,
  kPitchCorrectParams,
  kHarmonyParams,
  kSaturationParams,
  kFlangerParams,
  kPhaserParams,
  kVocoderParams,
}};

}  // namespace

std::span<const EffectDescriptor> effectCatalog() {
  return kCatalog;
}

std::optional<EffectDescriptor> findEffect(std::string_view effectType) {
  const auto catalog = effectCatalog();
  const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const auto& effect) {
    return effect.effectType == effectType;
  });
  if (it == catalog.end()) return std::nullopt;
  auto descriptor = *it;
  const auto index = static_cast<std::size_t>(std::distance(catalog.begin(), it));
  descriptor.parameters = kParameterSpans[index];
  return descriptor;
}

const char* availabilityName(EffectAvailability availability) {
  switch (availability) {
    case EffectAvailability::unavailable: return "unavailable";
    case EffectAvailability::implementedUnverified: return "implemented_unverified";
    case EffectAvailability::verified: return "verified";
  }
  return "unknown";
}

const char* fxErrorName(FxError error) {
  switch (error) {
    case FxError::none: return "NONE";
    case FxError::unknownEffect: return "UNKNOWN_EFFECT";
    case FxError::unavailable: return "UNAVAILABLE";
    case FxError::slotLimit: return "SLOT_LIMIT";
    case FxError::incompatibleFormat: return "INCOMPATIBLE_FORMAT";
    case FxError::staleRevision: return "STALE_REVISION";
    case FxError::factoryFailed: return "FACTORY_FAILED";
  }
  return "UNKNOWN";
}

}  // namespace localmixer::dsp::fx
