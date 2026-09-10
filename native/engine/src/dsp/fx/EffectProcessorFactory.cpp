#include "dsp/fx/EffectProcessorFactory.hpp"

#include "dsp/fx/CharacterEffects.hpp"
#include "dsp/fx/DelayEffect.hpp"
#include "dsp/fx/HarmonyEffect.hpp"
#include "dsp/fx/ModulationEffects.hpp"
#include "dsp/fx/PitchCorrectionEffect.hpp"
#include "dsp/fx/ReverbEffect.hpp"
#include "dsp/fx/VocoderEffect.hpp"

namespace localmixer::dsp::fx {

std::unique_ptr<EffectProcessor> createNativeEffectProcessor(std::string_view effectType) {
  if (effectType == "reverb") return std::make_unique<ReverbEffect>();
  if (effectType == "delay") return std::make_unique<DelayEffect>();
  if (effectType == "chorus") return std::make_unique<ChorusEffect>();
  if (effectType == "doubler") return std::make_unique<DoublerEffect>();
  if (effectType == "pitch_correct") return std::make_unique<PitchCorrectionEffect>();
  if (effectType == "harmony") return std::make_unique<HarmonyEffect>();
  if (effectType == "saturation") return std::make_unique<SaturationEffect>();
  if (effectType == "flanger") return std::make_unique<FlangerEffect>();
  if (effectType == "phaser") return std::make_unique<PhaserEffect>();
  if (effectType == "vocoder") return std::make_unique<VocoderEffect>();
  return nullptr;
}

EffectFactory nativeEffectFactory() {
  return [](std::string_view effectType) {
    return createNativeEffectProcessor(effectType);
  };
}

}  // namespace localmixer::dsp::fx
