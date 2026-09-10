# Vocal FX Latency Report

Source gate: VFX-09.

## Automated Evidence

| Check | Result | Evidence |
| --- | --- | --- |
| Rack latency aggregation | PASS | `EffectRackTest` verifies summed processor latency and latency-preserving slot movement. |
| Pitch backend start delay | PASS | `PitchBackendTest` requires non-zero Rubber Band block size and start delay. |
| Pitch correction latency reporting | PASS | `PitchCorrectionEffect::latencySamples()` reports backend start delay plus half detector window. |
| Harmony latency reporting | PASS | `HarmonyEffect::latencySamples()` reports backend start delay plus half detector window. |

## Current Latency Model

| Processor | Reported latency behavior | Status |
| --- | --- | --- |
| Reverb / Delay | Musical tail only; latency reports 0 | `PARTIAL` |
| Chorus / Flanger / Phaser | Modulated delay buffers report no fixed latency compensation | `PARTIAL` |
| Doubler | Creative voice delays are not treated as alignment latency | `PARTIAL` |
| Saturation | Reports 0 fixed latency | `PARTIAL` |
| Vocoder | Reports attack-derived envelope latency | `PARTIAL` |
| Pitch Shift / Formant | Uses Rubber Band LiveShifter start delay | `PARTIAL` |
| Pitch Correction | Rubber Band start delay + detector analysis half-window | `PARTIAL` |
| Harmony | Rubber Band voice start delay + detector analysis half-window | `PARTIAL` |

## Open Measurements

| Measurement | Status | Notes |
| --- | --- | --- |
| Cross-correlation/envelope latency WAV fixture report | `NOT_RUN` | Dedicated persistent latency render files are pending. |
| Live vocal monitoring comfort | `NOT_RUN` | Rubber Band LiveShifter latency may be too high for comfortable live singing; must be measured with hardware. |
| Callback p99/max and xrun counts at 256/512 frames | `NOT_RUN` | Runtime timing instrumentation is pending. |
| Record processed vs dry alignment | `PARTIAL` | Writer tests exist; end-to-end device-frame alignment is pending. |
| Offline export latency trim plus explicit tail | `PARTIAL` | Export tests exist; full Vocal FX graph export comparison is pending. |

## Result

The latency contracts are represented in native processors and unit tests, but VFX-09 latency is not final. Hardware audition, generated fixture files, callback metrics, and record/export alignment evidence remain required before a `VERIFIED` release claim.
