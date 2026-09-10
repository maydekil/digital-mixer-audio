# Product Acceptance Audit

Source gate: INT-03.

## Verdict

Status: `NOT_PRODUCT_VERIFIED`.

The project has substantial UI, native contract, DSP unit, package-smoke, and documentation evidence. It is not a complete product yet because mandatory hardware, listening, plugin runtime, live record/replay, packaged permission, and performance acceptance items remain `PARTIAL` or `NOT_RUN`.

## Automated Gates

| Gate | Result | Evidence |
| --- | --- | --- |
| Plan integrity | PASS | `npm run check:plan` covers 49 phases. |
| File size | PASS | `npm run check:file-size` passes with all first-party code files under the 1,000-line hard limit. |
| Architecture | PASS | `npm run check:architecture` rejects browser audio/Web Audio production paths. |
| UI tests | PASS | Vitest 19/19 in `npm run verify`. |
| Native tests | PASS | CTest 43/43 in `npm run verify`. |
| Dev package smoke | PASS_PARTIAL | `docs/reports/int02-package-smoke.md` proves unsigned `.app` resources/binaries, not GUI/TCC acceptance. |

## E2E 8E.11 Status

| ID | Scenario | Status | Blocking gap |
| --- | --- | --- | --- |
| E2E-01 | File-only import/play/EQ/fader/export | `PARTIAL` | Native processed graph export has focused coverage; desktop export dialog/offline playback of result not run. |
| E2E-02 | System audio via BlackHole | `PARTIAL` | Full processed route and restore not run from packaged app. |
| E2E-03 | Hybrid mic+backing with FX A/B | `PARTIAL` | Audible isolation and dual-return evidence not run. |
| E2E-04 | Harmony ON/OFF/key/level | `PARTIAL` | Real vocal intervals, measured delay, and no-click audition not run. |
| E2E-05 | Record dry/master then replay | `PARTIAL` | Native take metadata exists; live take replay and alignment not exposed end-to-end. |
| E2E-06 | Processed vocal take replay | `PARTIAL` | Processed take metadata can request neutral insert replay and persists in session JSON; successful live processed recording flow not run. |
| E2E-07 | Save/open/collect/relink | `PARTIAL` | Session contracts cover channel state, FX, harmony, plugin, and take metadata; desktop Save Project serializes renderer snapshot to validated `.lam.json`; full open/apply flow pending. |
| E2E-08 | Master + FX return stems | `PARTIAL` | Stem planning tests pass; rendered files not inspected. |
| E2E-09 | Device unplug/sleep/crash/full kill | `PARTIAL` | Supervisor/recovery tests pass; device/sleep/manual kill pending. |
| E2E-10 | Automation+MIDI+plugin state | `PARTIAL` | Automation/plugin state contracts pass; MIDI device and runtime plugin host pending. |
| E2E-11 | Clean packaged app offline | `PARTIAL` | Unsigned package exists; GUI launch/permission/offline import-record-export not run. |
| E2E-12 | 60-minute baseline + stress | `NOT_RUN` | Metrics instrumentation and soak are pending. |

## Conditional Items

| Feature | Status | Notes |
| --- | --- | --- |
| Per-app taps | `PARTIAL_CAPABILITY` | Native capability/assignment audit exists; real Core Audio tap backend and permission flow pending. |
| Multi-output physical | `NOT_RUN` | Requires interface with enough output channels. |
| Signed public release | `CAPABILITY_UNAVAILABLE` | Signing/notarization credentials are not present. |
| MP3 export | `PARTIAL` | Codec probe exists; packaged bundled encoder/export matrix pending. |
| 96 kHz | `NOT_RUN_OPTIONAL` | 44.1/48 kHz remain primary. |

## Critical Unresolved Work

- Native realtime insert graph and monitor selection have channel processor wiring, FX A/B send-return state, Vocal FX rack slots, and callback/deadline counters carried in the Core Audio monitor runtime; factory FX recipes now have native wet processors, 12 Vocal FX catalog entries have production factory coverage, and processed graph export has focused native coverage, but still need 99-program auditory QA, Harmony acceptance, full record/export parity, stress runs, and live listening QA.
- AU/VST3 runtime hosting/editor/latency compensation is not complete; Phase17 is scanner/registry/state foundation.
- Per-app Core Audio tap capture is not complete; Phase18 is capability/assignment foundation.
- Live recording to timeline and replay through desktop workflow are not complete; native take metadata now marks replay insert behavior, partial/overrun state, and session roundtrip.
- Product-level 30/60-minute soak and 32-track stress are not run; monitor callback counters exist but do not replace acceptance soak evidence.
- Packaged `.app` TCC permission prompt, clean-location launch, and offline workflow require manual target-Mac testing.

## Inputs For Phase19

- Keep final handoff status honest: package artifacts exist, but final acceptance remains incomplete.
- Continue by converting the remaining `PARTIAL` traceability rows into real command/UI flows and hardware/package evidence.
