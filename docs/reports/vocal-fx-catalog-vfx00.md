# VFX-00 Catalog Report

Date: 2026-09-10

This report mirrors the native registry in `native/engine/src/dsp/fx/EffectRegistry.cpp`. It is developer evidence for catalog/schema coverage only; it is not a claim that every DSP algorithm is finished.

## Status Summary

- Required native effect IDs: 12
- Registered native effect IDs: 12
- `implemented_unverified`: `reverb`, `delay`
- `unavailable`: `chorus`, `doubler`, `pitch_shift`, `formant_shift`, `pitch_correct`, `harmony`, `saturation`, `flanger`, `phaser`, `vocoder`
- `verified`: none

## Verification

- Native registry unit test: `local-mixer-effect-registry-tests`
- Full command used after wiring: `npm run test:native`
- Result: PASS, CTest 21/21

## Open Follow-Up

- VFX-01 must add rack runtime and plumbing tests before production UI activation.
- VFX-02 through VFX-07 must move individual effects from metadata to measured DSP.
- VFX-08 must connect UI/preset/automation hooks and keep unavailable effects disabled.
