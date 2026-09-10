# Harmony Button QA Report

Source gate: HARM-03.

## Automated Evidence

| Check | Result | Evidence |
| --- | --- | --- |
| Harmony/session/automation CTest subset | PASS | `ctest --test-dir native/engine/build -R 'local-mixer-(harmony\|channel-harmony\|automation\|session)' --output-on-failure` passed 4/4 tests. |
| Channel shortcut controller | PASS | `ChannelHarmonyControllerTest` covers primary instance binding, role rejection, rack capacity, revision handling, and duplicate protection. |
| Harmony DSP semantics | PASS | `HarmonyEffectTest` covers generated voices, disabled lead preservation, silence gating, and Harmony Level not trimming lead. |
| Session recall contract | PASS | `SessionDocumentTest` covers channel harmony state roundtrip. |
| Automation IDs | PASS | `AutomationTest` covers harmony enabled and level parameter IDs. |

## 8C.6 Checklist

| Requirement | Status | Notes |
| --- | --- | --- |
| Mixer VOICE strip has HARMONY button with ON/OFF and gear | `PASS_UI` | Implemented in Mixer UI and preserved by UI visual gate. |
| Activate/edit without opening Vocal FX | `PASS_UI_CONTRACT` | Quick tray controls key/scale/mode/voices/level from Mixer. |
| FX A/B remain visible and independent | `PASS_UI_CONTRACT` | Compact FX rows remain separate from Harmony shortcut. |
| ON adds two voices, OFF removes harmony only, lead remains | `PASS_NATIVE_CONTRACT` | Covered by `HarmonyEffectTest`; real vocal audition NOT_RUN. |
| Manual key/scale visible, no auto-detect claim | `PASS_UI_CONTRACT` | Defaults are visible as C Major and editable manually. |
| Shortcut/rack/quick tray share primary instance | `PASS_NATIVE_CONTRACT` | `ChannelHarmonyController` returns stable primary instance ID. |
| Multiple clicks do not duplicate processors; rack capacity honored | `PASS_NATIVE_CONTRACT` | Covered by controller tests. |
| Harmony Level affects harmony voices only | `PASS_NATIVE_CONTRACT` | Covered by DSP tests. |
| Lead alignment/latency does not jump on toggle | `PARTIAL` | Processor ramp exists; real audible transition/latency measurement NOT_RUN. |
| Per-channel/session/undo/redo/record/export/Monitor Fast tested | `PARTIAL` | Session and automation contracts pass; undo/redo, record/export, Monitor Fast end-to-end not run. |
| Button does not show false active state when backend unavailable | `PARTIAL` | ACK/pending/error UI exists; backend failure through full graph not run. |

## Required Manual Evidence

- Screenshot Mixer desktop for current packaged app at supported viewport is NOT_RUN in this checkpoint.
- Real vocal recording with known key is NOT_RUN.
- Audio-system plus music plus Harmony running together is NOT_RUN.
- UI tab switching while Harmony is enabled is PARTIAL from UI tests, not packaged manual evidence.

## Result

HARM-03 has native contract evidence and checklist coverage, but it is not final `VERIFIED`. Real listening, packaged screenshot, Monitor Fast status, undo/redo, and record/export end-to-end checks remain open.
