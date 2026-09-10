# Phase19 Handoff

Source gate: Phase19.

## Handoff Status

Status: `IMPLEMENTED_UNVERIFIED_HANDOFF`.

This handoff provides source, build commands, reports, an unsigned development `.app`, and a blocker list. It does not claim `PRODUCT_VERIFIED`; `docs/reports/product-acceptance.md` remains authoritative for unresolved acceptance gaps.

## Artifact

| Item | Value |
| --- | --- |
| Bundle | `build/package/Local Audio Mixer.app` |
| Archive | `build/package/Local-Audio-Mixer-dev.zip` |
| Archive SHA-256 | `1f76af5198503a55866d6b82b234804912e650905bda91b12f3befe97d8306e8` |
| Package source revision | `0f7acca` |
| Bundle ID | `audio.local-mixer.dev` |
| Code signing | `UNSIGNED` |
| Notarization | `NOT_RUN` |

## Commands

| Command | Purpose |
| --- | --- |
| `npm run verify` | Plan, file-size, architecture, typecheck, UI tests, native tests, UI build. |
| `npm run test:native` | Native CMake/CTest, sound-pad validation, engine smoke protocol. |
| `npm run test:int01` | Daily journey contract and data-integrity report. |
| `npm run package:mac:unsigned` | Builds unsigned local macOS development app/archive. |
| `npm run dev:desktop:ui` | Opens desktop UI preview. |
| `npm run dev:engine` | Opens desktop app with native engine required. |

## Included Reports

- `docs/feature-traceability.md`
- `docs/reports/product-acceptance.md`
- `docs/reports/int01-daily-journey.md`
- `docs/reports/int02-package-smoke.md`
- `docs/reports/vocal-fx-quality.md`
- `docs/reports/vocal-fx-latency.md`
- `docs/reports/mixer-dsp99.md`
- `docs/reports/harmony-button.md`
- `docs/reports/codec-probe-phase06.md`
- `docs/reports/sound-pad-assets.md`
- `docs/reports/ui-layout-review.md`

## Final Blockers

| Area | Status | Required to clear |
| --- | --- | --- |
| Product acceptance | `NOT_PRODUCT_VERIFIED` | Complete all mandatory E2E scenarios in `docs/reports/product-acceptance.md`. |
| Packaged app launch/TCC | `NOT_RUN` | Launch packaged `.app` from a clean location and verify microphone/capture permissions. |
| Realtime DSP graph | `PARTIAL` | Wire all channel DSP, FX A/B, Vocal FX, Harmony, metering, recording taps, and export graph snapshots into the live native callback. |
| AU/VST3 | `PARTIAL` | Implement actual runtime hosting, editor, state restore, latency compensation, and tested plugin compatibility. |
| Per-app capture | `PARTIAL` | Implement Core Audio tap enumeration/capture, permission handling, original mute, own-output exclusion, restart recovery, and clock bridge. |
| Recording/replay | `PARTIAL` | Prove live dry/processed/master takes, alignment, timeline replay, save/open, and export. |
| QA/performance | `NOT_RUN` | Generate fixture WAVs, perform listening QA, run 30/60-minute soak and 32-track stress with CPU/xrun/deadline metrics. |

## Result

Phase19 handoff materials exist, but final product completion is blocked by real implementation and verification gaps. Continue from `docs/reports/product-acceptance.md` and `docs/feature-traceability.md`; do not promote this build beyond development review until those reports are cleared with actual evidence.
