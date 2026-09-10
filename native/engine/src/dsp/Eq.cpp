#include "dsp/Eq.hpp"

#include <algorithm>
#include <cmath>
#include <complex>

namespace localmixer::dsp {
namespace {

constexpr double kPi = 3.14159265358979323846;

BiquadCoefficients normalize(double b0, double b1, double b2, double a0, double a1, double a2) {
  if (a0 == 0.0) return {};
  return {.b0 = b0 / a0, .b1 = b1 / a0, .b2 = b2 / a0, .a1 = a1 / a0, .a2 = a2 / a0};
}

double clampedFrequency(double sampleRate, double frequencyHz) {
  return std::clamp(frequencyHz, 1.0, std::max(1.0, sampleRate * 0.49));
}

}  // namespace

BiquadCoefficients makeBiquad(const EqBandConfig& config) {
  if (!config.enabled || config.sampleRate <= 0.0) return {};
  const auto frequency = clampedFrequency(config.sampleRate, config.frequencyHz);
  const auto q = std::max(0.05, config.q);
  const auto omega = 2.0 * kPi * frequency / config.sampleRate;
  const auto sinw = std::sin(omega);
  const auto cosw = std::cos(omega);
  const auto alpha = sinw / (2.0 * q);
  const auto amplitude = std::pow(10.0, config.gainDb / 40.0);

  switch (config.type) {
    case EqFilterType::highPass:
      return normalize((1.0 + cosw) * 0.5, -(1.0 + cosw), (1.0 + cosw) * 0.5, 1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
    case EqFilterType::lowPass:
      return normalize((1.0 - cosw) * 0.5, 1.0 - cosw, (1.0 - cosw) * 0.5, 1.0 + alpha, -2.0 * cosw, 1.0 - alpha);
    case EqFilterType::peaking:
      return normalize(1.0 + alpha * amplitude, -2.0 * cosw, 1.0 - alpha * amplitude,
        1.0 + alpha / amplitude, -2.0 * cosw, 1.0 - alpha / amplitude);
    case EqFilterType::lowShelf: {
      const auto beta = std::sqrt(amplitude) / q;
      return normalize(
        amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * cosw + beta * sinw),
        2.0 * amplitude * ((amplitude - 1.0) - (amplitude + 1.0) * cosw),
        amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * cosw - beta * sinw),
        (amplitude + 1.0) + (amplitude - 1.0) * cosw + beta * sinw,
        -2.0 * ((amplitude - 1.0) + (amplitude + 1.0) * cosw),
        (amplitude + 1.0) + (amplitude - 1.0) * cosw - beta * sinw
      );
    }
    case EqFilterType::highShelf: {
      const auto beta = std::sqrt(amplitude) / q;
      return normalize(
        amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * cosw + beta * sinw),
        -2.0 * amplitude * ((amplitude - 1.0) + (amplitude + 1.0) * cosw),
        amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * cosw - beta * sinw),
        (amplitude + 1.0) - (amplitude - 1.0) * cosw + beta * sinw,
        2.0 * ((amplitude - 1.0) - (amplitude + 1.0) * cosw),
        (amplitude + 1.0) - (amplitude - 1.0) * cosw - beta * sinw
      );
    }
  }
  return {};
}

double biquadMagnitudeDb(const BiquadCoefficients& coefficients, double sampleRate, double frequencyHz) {
  const auto frequency = clampedFrequency(sampleRate, frequencyHz);
  const auto omega = -2.0 * kPi * frequency / sampleRate;
  const std::complex<double> z1{std::cos(omega), std::sin(omega)};
  const auto z2 = z1 * z1;
  const auto numerator = coefficients.b0 + coefficients.b1 * z1 + coefficients.b2 * z2;
  const auto denominator = 1.0 + coefficients.a1 * z1 + coefficients.a2 * z2;
  const auto magnitude = std::abs(numerator / denominator);
  if (magnitude <= 0.0) return -120.0;
  return std::max(-120.0, 20.0 * std::log10(magnitude));
}

void BiquadFilter::setCoefficients(BiquadCoefficients coefficients) {
  coefficients_ = coefficients;
}

void BiquadFilter::reset() {
  x1_ = 0.0;
  x2_ = 0.0;
  y1_ = 0.0;
  y2_ = 0.0;
}

void BiquadFilter::process(std::span<float> samples) {
  for (auto& sample : samples) {
    const auto input = static_cast<double>(sample);
    const auto output = coefficients_.b0 * input + coefficients_.b1 * x1_ + coefficients_.b2 * x2_ -
      coefficients_.a1 * y1_ - coefficients_.a2 * y2_;
    x2_ = x1_;
    x1_ = input;
    y2_ = y1_;
    y1_ = output;
    sample = static_cast<float>(output);
  }
}

}  // namespace localmixer::dsp
