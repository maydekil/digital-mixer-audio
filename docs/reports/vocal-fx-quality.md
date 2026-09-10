# Vocal FX Quality Report

Source gate: VFX-09.

## Automated Evidence

| Check | Result | Evidence |
| --- | --- | --- |
| Effect registry/rack/time/character/pitch/harmony/vocoder CTest subset | PASS | `ctest --test-dir native/engine/build -R 'local-mixer-(effect\|time\|character\|pitch\|harmony\|vocoder)' --output-on-failure` passed 10/10 tests. |
| Full native suite | PASS | `npm run verify` passed native CTest 36/36 before this report checkpoint. |
| Browser audio exclusion | PASS | `npm run check:architecture` passes and production audio commands remain native. |

## Effect Coverage

| Family | Native owner | Automated quality evidence | Status |
| --- | --- | --- | --- |
| Reverb | `ReverbEffect` | wet tail/non-empty impulse response in time/modulation tests | `PARTIAL` |
| Delay / Echo | `DelayEffect` | repeat spacing/feedback tests | `PARTIAL` |
| Chorus | `ChorusEffect` | modulated delayed output tests | `PARTIAL` |
| Doubler | `DoublerEffect` | two-voice delay/detune presence tests | `PARTIAL` |
| Pitch Shift | `PitchBackend` | Rubber Band pitch shift up/down within tolerance | `PARTIAL` |
| Formant Shift | `PitchBackend` | backend formant-preserve construction covered; dedicated formant listening not run | `PARTIAL` |
| Pitch Correction | `PitchCorrectionEffect` | detector/scale correction and confidence behavior tests | `PARTIAL` |
| Harmony | `HarmonyEffect` | two-voice generation, disabled-lead preservation, level semantics | `PARTIAL` |
| Saturation | `SaturationEffect` | bounded nonlinear output and unity behavior tests | `PARTIAL` |
| Flanger | `FlangerEffect` | feedback-clamped modulation tests | `PARTIAL` |
| Phaser | `PhaserEffect` | modulated all-pass output tests | `PARTIAL` |
| Vocoder | `VocoderEffect` | carrier/envelope output and bounded processing tests | `PARTIAL` |

## Required Human/Hardware QA

| Requirement | Status | Notes |
| --- | --- | --- |
| Three licensed dry vocal recordings: speech, sustained vowel, melody | `NOT_RUN` | No vocal fixtures were recorded or bundled in this environment. |
| Level-matched dry/wet WAV renders per effect and preset | `NOT_RUN` | Current tests render in memory; persistent audition WAV matrix is pending. |
| Listening rubric: intelligibility, clicks, metallic artifacts, consonants, mono compatibility | `NOT_RUN` | Requires human audition on generated wet files. |
| Monitor Fast vs Studio FX vs record/export graphs | `PARTIAL` | Contracts exist, but full graph/profile record-export comparison is pending. |
| Output disconnect/recovery during Vocal FX | `NOT_RUN` | Requires device interaction. |
| Heavy-chain benchmark at 256/512 frames | `NOT_RUN` | Metrics collection is not yet implemented. |

## Result

VFX-09 has portable native DSP evidence, but it is not fully release-verified. Real vocal fixtures, persistent render files, listening QA, performance metrics, and packaged permission/device tests remain open.
