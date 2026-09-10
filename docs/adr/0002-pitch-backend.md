# ADR 0002: Vocal FX Pitch Backend Boundary

Status: Proposed for VFX-00

## Context

Vocal FX requires pitch shift, formant shift, pitch correction, and harmony. The implementation spec names Rubber Band LiveShifter as the first concrete backend to spike, but it also requires a backend abstraction so UI, harmony, correction, and rack lifecycle do not depend on a specific library API.

## Decision

Create native pitch effects behind `dsp/fx/PitchBackend` in later VFX phases. The backend boundary owns fixed-block adaptation, start-delay reporting, ratio/formant control, reset/drain, and structural rebuild decisions. UI and IPC must talk to effect/rack parameters, not directly to Rubber Band or any other pitch library.

Rubber Band is not considered integrated by VFX-00. It remains unavailable until build, license, latency, and accuracy tests are recorded. If the backend cannot be used, the project must document the blocker and pick another native backend that satisfies the same interface.

## Consequences

- Pitch-related catalog entries remain `unavailable` until VFX-04/VFX-05/VFX-06 implement and test actual DSP.
- Pitch correction still needs a native monophonic detector; Rubber Band pitch shifting alone is not pitch correction.
- Reported latency must include backend start delay and adapter buffering.
- No browser audio, Web Audio, renderer WASM DSP, or resampling shortcut may be used as fallback.
