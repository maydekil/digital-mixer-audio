#include "dsp/Fx.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using localmixer::dsp::DelayConfig;
using localmixer::dsp::DelayLine;
using localmixer::dsp::DuckerConfig;
using localmixer::dsp::ReverbConfig;
using localmixer::dsp::SimpleReverb;
using localmixer::dsp::VoiceDucker;

bool near(float actual, float expected, float tolerance = 0.001f) {
  return std::fabs(actual - expected) <= tolerance;
}

float peakAfter(const std::vector<float>& samples, std::size_t start) {
  float peak = 0.0f;
  for (std::size_t index = start; index < samples.size(); index += 1) {
    peak = std::max(peak, std::fabs(samples[index]));
  }
  return peak;
}

}  // namespace

int main() {
  DelayLine delay;
  delay.configure(DelayConfig{.sampleRate = 48000.0, .delayMs = 1.0f, .feedback = 0.5f, .wetGain = 1.0f, .tone = 1.0f});
  std::vector<float> impulse(160, 0.0f);
  impulse[0] = 1.0f;
  std::vector<float> delayed(160, 0.0f);
  delay.processWet(impulse, delayed);
  if (!near(delayed[0], 0.0f) || !near(delayed[48], 1.0f) || !near(delayed[96], 0.5f) ||
      !near(delayed[144], 0.25f)) {
    std::cerr << "delay repeats should follow capped feedback\n";
    return 1;
  }

  DelayLine cappedDelay;
  cappedDelay.configure(DelayConfig{.sampleRate = 48000.0, .delayMs = 1.0f, .feedback = 1.0f, .wetGain = 1.0f, .tone = 1.0f});
  std::fill(delayed.begin(), delayed.end(), 0.0f);
  cappedDelay.processWet(impulse, delayed);
  if (!(delayed[96] < delayed[48])) {
    std::cerr << "delay feedback should be capped below unity\n";
    return 1;
  }

  SimpleReverb reverb;
  reverb.configure(ReverbConfig{.sampleRate = 48000.0, .preDelayMs = 1.0f, .decay = 0.5f, .wetGain = 1.0f});
  std::vector<float> reverbOut(6000, 0.0f);
  std::vector<float> reverbImpulse(6000, 0.0f);
  reverbImpulse[0] = 1.0f;
  reverb.processWet(reverbImpulse, reverbOut);
  if (peakAfter(reverbOut, 1000) <= 0.0f) {
    std::cerr << "reverb should produce a wet tail after the dry impulse\n";
    return 1;
  }

  VoiceDucker ducker;
  ducker.configure(DuckerConfig{.thresholdDb = -30.0f, .depthDb = -12.0f, .attackMs = 0.0f, .holdMs = 0.0f, .releaseMs = 10.0f});
  std::vector<float> voice(256, 0.5f);
  std::vector<float> music(256, 1.0f);
  ducker.process(voice, music);
  if (music.back() > 0.26f || ducker.currentGainDb() > -11.5f) {
    std::cerr << "voice activity should duck the target bus\n";
    return 1;
  }

  std::fill(voice.begin(), voice.end(), 0.0f);
  ducker.process(voice, music);
  if (ducker.currentGainDb() >= -1.0f) {
    std::cerr << "ducker should recover gradually after detector silence\n";
    return 1;
  }

  std::cout << "local-mixer-fx-tests ok\n";
  return 0;
}
