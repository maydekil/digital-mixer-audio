# VFX-00 Catalog Report

Date: 2026-09-10

This report mirrors the native registry in `native/engine/src/dsp/fx/EffectRegistry.cpp`. It is developer evidence for catalog/schema coverage only; it is not a claim that every DSP algorithm is finished.

## Status Summary

- Required native effect IDs: 12
- Registered native effect IDs: 12
- `implemented_unverified`: `reverb`, `delay`, `chorus`, `doubler`, `saturation`, `flanger`, `phaser`, `pitch_shift`, `formant_shift`, `pitch_correct`, `harmony`, `vocoder`
- `unavailable`: none
- `verified`: none

## Verification

- Native registry unit test: `local-mixer-effect-registry-tests`
- Latest full command used after wiring: `npm run test:native`
- Latest result: PASS, CTest 28/28

## Open Follow-Up

- VFX-01 must add rack runtime and plumbing tests before production UI activation.
- VFX-02 through VFX-07 must move individual effects from metadata to measured DSP.
- VFX-04 adds the Rubber Band pitch/formant backend and unit evidence, but musical QA, latency compensation integration, and production rack wiring remain pending.
- VFX-05 adds monophonic pitch detection, scale mapping, and pitch-correction unit evidence, but real vocal audition, vibrato preservation, and rack/UI integration remain pending.
- VFX-06 adds two-voice harmony generation, fixed/diatonic mapping, panning, and voicing gate unit evidence, but latency-aligned rack integration and real vocal QA remain pending.
- VFX-07 adds a native 16-band vocoder/robot foundation with internal fixed/MIDI carrier and panic tests, but articulation listening QA and rack/UI integration remain pending.
- VFX-08 must connect UI/preset/automation hooks and keep unavailable effects disabled.
