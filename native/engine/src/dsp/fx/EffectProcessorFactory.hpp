#pragma once

#include "dsp/fx/EffectRack.hpp"

namespace localmixer::dsp::fx {

std::unique_ptr<EffectProcessor> createNativeEffectProcessor(std::string_view effectType);
EffectFactory nativeEffectFactory();

}  // namespace localmixer::dsp::fx
