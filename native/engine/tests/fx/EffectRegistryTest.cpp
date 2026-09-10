#include "dsp/fx/EffectRegistry.hpp"

#include <array>
#include <iostream>
#include <string_view>

namespace {

using localmixer::dsp::fx::EffectAvailability;
using localmixer::dsp::fx::availabilityName;
using localmixer::dsp::fx::effectCatalog;
using localmixer::dsp::fx::findEffect;
using localmixer::dsp::fx::fxErrorName;

constexpr std::array<std::string_view, 12> kRequiredIds{
  "reverb",
  "delay",
  "chorus",
  "doubler",
  "pitch_shift",
  "formant_shift",
  "pitch_correct",
  "harmony",
  "saturation",
  "flanger",
  "phaser",
  "vocoder",
};

}  // namespace

int main() {
  const auto catalog = effectCatalog();
  if (catalog.size() != kRequiredIds.size()) {
    std::cerr << "Vocal FX catalog should expose exactly the 12 required native effect IDs\n";
    return 1;
  }

  for (const auto id : kRequiredIds) {
    const auto effect = findEffect(id);
    if (!effect.has_value() || effect->effectType != id || effect->ownerModule.empty() ||
        effect->implementationPhase.empty() || effect->testPlan.empty() || effect->parameters.empty()) {
      std::cerr << "Vocal FX descriptor missing required metadata or parameter schema: " << id << "\n";
      return 1;
    }
  }

  const auto reverb = findEffect("reverb");
  const auto delay = findEffect("delay");
  if (!reverb.has_value() || !delay.has_value() ||
      reverb->availability != EffectAvailability::implementedUnverified ||
      delay->availability != EffectAvailability::implementedUnverified) {
    std::cerr << "Phase11-backed reverb/delay should be marked implemented_unverified, not verified\n";
    return 1;
  }

  const auto pitchCorrect = findEffect("pitch_correct");
  if (!pitchCorrect.has_value() || pitchCorrect->availability != EffectAvailability::implementedUnverified) {
    std::cerr << "Pitch correction should be implemented_unverified after VFX-05\n";
    return 1;
  }

  const auto pitchShift = findEffect("pitch_shift");
  const auto formantShift = findEffect("formant_shift");
  const auto harmony = findEffect("harmony");
  if (!pitchShift.has_value() || !formantShift.has_value() || !harmony.has_value() ||
      pitchShift->availability != EffectAvailability::implementedUnverified ||
      formantShift->availability != EffectAvailability::implementedUnverified ||
      harmony->availability != EffectAvailability::implementedUnverified) {
    std::cerr << "Pitch/formant/harmony effects should be marked implemented_unverified after VFX-06\n";
    return 1;
  }

  if (availabilityName(EffectAvailability::verified) != std::string_view("verified") ||
      fxErrorName(localmixer::dsp::fx::FxError::staleRevision) != std::string_view("STALE_REVISION")) {
    std::cerr << "developer report names and IPC error names should be stable\n";
    return 1;
  }

  std::cout << "local-mixer-effect-registry-tests ok\n";
  return 0;
}
