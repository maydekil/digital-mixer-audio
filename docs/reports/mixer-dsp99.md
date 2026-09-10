# Mixer DSP99 Report

Source gate: MIXFX-05.

## Automated Evidence

| Check | Result | Evidence |
| --- | --- | --- |
| FX/send/session/export CTest subset | PASS | `ctest --test-dir native/engine/build -R 'local-mixer-fx-(program\|send\|tests)\|local-mixer-export-tests\|local-mixer-session-document-tests' --output-on-failure` passed 7/7 tests. |
| Factory bank size | PASS | `FxProgramRegistryTest` verifies exactly 99 programs with unique contiguous IDs 1-99. |
| Required program anchors | PASS | `FxProgramRegistryTest` verifies program 12 `Vocal Plate`, program 50 `Stereo 320`, and bounded program 99 `Infinite Mood`. |
| Wet-only recipe expansion | PASS | `FxProgramRegistryTest` verifies each factory program expands `wet_pct=100` and `output_db=0`. |
| Dual-unit state | PASS | `FxProgramControllerTest` verifies unit A defaults to 12, unit B defaults to 50, independent request queues, crossfade transition frames, revision conflict, macro override/reset, and prepare failure rollback. |
| Session/export contracts | PASS | `SessionDocumentTest` and `ExportTest` cover persisted FX snapshots and return stem planning. |

## User Scenario 8B.9

| Step | Status | Notes |
| --- | --- | --- |
| Mixer opens with A/B visible and sources not auto-monitoring | `PARTIAL` | UI exists; packaged manual open not run in this checkpoint. |
| Select VOICE mic and monitor dry through headphones | `NOT_RUN` | Requires physical device test. |
| FX A program 12 ON, return -6 dB, VOICE send -18 dB | `PARTIAL` | Program/ACK contracts covered; audible plate not run. |
| MUSIC send A remains OFF | `PARTIAL` | Preview fixture and send data support this; native audible isolation not run. |
| FX B program 50 ON with delay while plate remains | `PARTIAL` | Program B default and independent controller covered; audible dual-return not run. |
| Macro Decay A changes tail while B time remains 320 ms | `PARTIAL` | Macro override/reset contract covered; tail audition not run. |
| FX A OFF fades plate, dry vocal and B stay alive | `NOT_RUN` | Requires realtime graph/audition evidence. |
| VOICE INSERT FX quick drawer edits correction amount | `PARTIAL` | UI exists; full native insert parameter command path remains partial. |
| Save/open restores numbers, overrides, sends, returns, insert | `PARTIAL` | Session roundtrip tests cover state contracts; desktop menu save/open flow pending. |

## Open QA

- Rendered impulse/sine/vocal WAV files for all 99 programs are not generated yet.
- Batch audition of all 99 programs is NOT_RUN.
- 30-minute system+mic+file soak with both FX units active is NOT_RUN.
- CPU/deadline/xrun metrics for periodic program changes are NOT_RUN.
- MIXFX-05 should not be promoted to `VERIFIED` until the above measurements exist.
