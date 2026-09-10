# Local Audio Mixer Agent Instructions

Build this project according to `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md`.
Use `docs/progress.md` and the canonical dependency plan in section 8E.1 to resume.

## Execution Order

- Complete UI-00 through UI-04 first, using the compact layout in section 8D.
- Do not start new native audio, DSP, Core Audio, or JUCE work before UI-04 is `UI_VERIFIED`.
- After the UI gate, continue the exact dependency order in `docs/task-plan.json`.
- Do not treat a mockup, preview build, or successful UI bundle as product completion.

## Architecture

- Keep React/Electron UI separate from the native C++20/JUCE/Core Audio engine.
- All audio capture, playback, DSP, mixing, monitoring, recording, export, and metering must run in native code or approved native media workers.
- Never use Web Audio, browser media capture/playback/recording APIs, renderer WASM DSP, or browser fallbacks.
- Preview mode is silent and must display `UI PREVIEW · Audio engine belum terhubung`.
- Never send PCM through renderer IPC, JSON, network services, or React state.

## Modularity

- Apply section 4A to UI, Electron, adapters, scripts, tests, and native code.
- Build small reusable primitives and audio controls before feature panels.
- Keep every first-party code file at or below 1,000 physical lines.
- Prefer focused modules of 60-300 lines over broad page or engine files.
- Run `npm run check:file-size` and `npm run check:architecture`.
- Do not evade file-size limits with minification, massive includes, part files, or unchecked exclusions.

## Realtime Native Rules

- The audio callback must not allocate, free, block, serialize JSON, log, or perform disk/network I/O.
- Preallocate resources, smooth audible parameter changes, and reclaim old graphs off the callback.
- Validate all IPC and routing in the engine. Reject graph cycles and loopback routes.

## Progress And Evidence

- Implement one phase at a time and record actual evidence in `docs/progress.md`.
- Track important controls and capabilities through UI entry point, command/API, native owner, save field, tests, and manual evidence.
- Hardware or permission tests that did not run are `NOT_RUN` or `BLOCKED_ENVIRONMENT`, never PASS.
- Preserve user files and existing changes. Do not silently alter OS audio routing, install drivers, or request mic permissions during UI phases.
