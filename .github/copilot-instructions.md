# Copilot Instructions

Build Local Audio Mixer according to `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md`.

- Resume from `docs/progress.md` and `docs/task-plan.json`.
- Complete UI-00 through UI-04 before new native audio work.
- Use the compact mixer layout in section 8D: FX A/B rows on top, mixer bank left, Channel Processing right, Harmony tray below the bank.
- Keep PreviewAdapter visibly labelled and silent. Do not claim audio, recording, capture, latency, or hardware behavior in preview.
- After `UI_VERIFIED`, continue native phases in section 8E.1 order.
- Keep UI controls reusable: buttons, toggles, program picker, faders, meters, knobs, numeric parameters, EQ graph, and Harmony controls.
- Keep first-party code files at or below 1,000 physical lines and run `check:file-size`.
- Run `check:architecture`; UI primitives must not import feature, adapter, native, or Electron modules.
- All production audio must be native C++20/JUCE/Core Audio or approved native workers.
- Never use Web Audio, browser capture/playback/recording, renderer WASM DSP, or browser audio fallback.
- The native audio callback must not allocate, block, serialize, log, or perform I/O.
- Record real command results and limitations in `docs/progress.md`.
- Do not equate UI/build success with product completion. INT-00 through INT-03 and packaged-app acceptance are required for full completion.
