# Local Audio Mixer

Local Audio Mixer is an Electron desktop shell with a native C++20 audio engine foundation. The UI is React/Vite, but production audio paths are native-only: do not use Web Audio, browser capture, renderer DSP, or browser playback fallbacks.

## Current Status

This repository is not `PRODUCT_VERIFIED` yet. UI, native contracts, DSP unit tests, package smoke, and QA reports exist, but real packaged hardware/listening/performance acceptance is still incomplete. See:

- `docs/progress.md`
- `docs/feature-traceability.md`
- `docs/reports/product-acceptance.md`
- `docs/reports/phase19-handoff.md`

## Development Commands

```sh
npm install
npm run verify
npm run dev:desktop:ui
npm run dev:engine
```

Use `npm run dev:desktop:ui` for the desktop UI preview. Use `npm run dev:engine` when testing the native engine bridge from the desktop app.

## Native Checks

```sh
npm run build:native
npm run test:native
npm run test:int01
```

Useful native commands:

```sh
native/engine/build/native/engine/local-mixer-engine --list-devices
native/engine/build/native/engine/local-mixer-engine --request-mic-permission
native/engine/build/native/engine/local-mixer-engine --meter-input --input-uid "<device uid>"
native/engine/build/native/engine/local-mixer-engine --monitor-passthrough --input-uid "<input uid>" --output-uid "<output uid>"
```

## Packaging

Build an unsigned development package:

```sh
npm run package:mac:unsigned
```

Artifacts are generated under `build/package/`:

- `Local Audio Mixer.app`
- `Local-Audio-Mixer-dev.zip`

The package is unsigned and not notarized. It is intended for local development review only.

## System Audio

System audio mode expects a user-installed BlackHole 2ch route. Follow `docs/setup/system-audio-routing.md`. The app must not silently change OS routing without an explicit user action.

## Known Gaps

- Full realtime insert graph wiring for all channel DSP, FX A/B, Vocal FX, and Harmony is partial.
- AU/VST3 runtime hosting/editor/latency compensation is not complete.
- Per-app Core Audio taps are capability/assignment foundation only.
- Packaged `.app` GUI/TCC permission, live record/replay, offline export playback, and stress benchmarks still require real target-Mac evidence.
