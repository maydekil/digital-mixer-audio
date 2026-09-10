# Vocal FX Native Contract

Source: `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md` section 8A.

## VFX-00 Scope

This document records the design contract for native Vocal FX. It is not evidence that the DSP for every effect is complete. Runtime audio processing remains native C++ only; renderer UI may display and edit metadata but must not process PCM or synthesize fallbacks.

## Catalog

| Effect ID | Owner Module | Phase | Current Status | Test Plan |
| --- | --- | --- | --- | --- |
| `reverb` | `dsp/fx/ReverbEffect` | VFX-02 | `implemented_unverified` | impulse decay, tail limit, wet-only send |
| `delay` | `dsp/fx/DelayEffect` | VFX-02 | `implemented_unverified` | repeat spacing, feedback bound, tempo sync |
| `chorus` | `dsp/fx/ModulationEffects` | VFX-02 | `implemented_unverified` | fractional delay modulation sweep |
| `doubler` | `dsp/fx/DoublerEffect` | VFX-03 | `implemented_unverified` | two independent micro-delay voices, mono fold-down |
| `pitch_shift` | `dsp/fx/PitchBackend` | VFX-04 | `implemented_unverified` | steady pitch ratio, latency, variable callback sizes |
| `formant_shift` | `dsp/fx/PitchBackend` | VFX-04 | `implemented_unverified` | envelope shift without intentional F0 move |
| `pitch_correct` | `dsp/fx/PitchCorrectionEffect` | VFX-05 | `implemented_unverified` | detector, target mapping, detuned-note correction |
| `harmony` | `dsp/fx/HarmonyEffect` | VFX-06 | `implemented_unverified` | fixed/diatonic interval accuracy and dry alignment |
| `saturation` | `dsp/fx/SaturationEffect` | VFX-03 | `implemented_unverified` | harmonics, output trim, DC suppression |
| `flanger` | `dsp/fx/ModulationEffects` | VFX-02 | `implemented_unverified` | comb sweep and bounded feedback |
| `phaser` | `dsp/fx/ModulationEffects` | VFX-02 | `implemented_unverified` | all-pass sweep and silence stability |
| `vocoder` | `dsp/fx/VocoderEffect` | VFX-07 | `unavailable` | filter-bank envelope, carrier note handling, panic |

## Format Policy

Vocal pitch processors run on the mono prefix unless a later phase adds an explicit stereo negotiation path. Widening processors can output stereo; mono-only processors after stereo widening must be rejected with `INCOMPATIBLE_FORMAT` until a tested negotiation exists. Harmony can be the first widening node.

## Rack Policy

Native creative racks have a bounded maximum of 8 native slots per channel and 4 per bus. Plugin slots are separate and are not part of VFX-00. Slot identity uses a stable instance UUID, not rack index. Rack replacement is all-or-nothing and must preserve the old rack when factory creation fails.

## Mix, Bypass, And Latency

Default wet/dry law is linear: `out = (1 - mix) * alignedDry + mix * wet`. Dry alignment compensates algorithmic latency only, not musical delay or reverb predelay. Bypass uses a short crossfade while preserving latency compensation. CPU-saving unload/suspend is a separate graph transaction because it can alter latency.

## IPC Error Contract

Stable errors are exposed by the native registry as:

- `UNKNOWN_EFFECT`
- `UNAVAILABLE`
- `SLOT_LIMIT`
- `INCOMPATIBLE_FORMAT`
- `STALE_REVISION`
- `FACTORY_FAILED`

## Developer Availability Flags

`unavailable` means the effect may appear in developer reports but cannot be activated in production UI. `implemented_unverified` means native code exists but final phase acceptance has not passed. `verified` is reserved for phase gates with representative DSP tests and integration evidence.
