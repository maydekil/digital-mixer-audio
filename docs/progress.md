# Local Audio Mixer Progress

Specification: `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md` revision 1.7.

## Project Status

- Current gate: Phase04 system audio routing foundation.
- Reference image: `docs/design/Digital Mixer Audio.png`; visual target is available and inspected.
- Native audio engine: `PHASE05_GRAPH_FOUNDATION_IMPLEMENTED_UNVERIFIED`; native sound-pad spike remains separate early user-requested work.
- Product completion: `NOT_STARTED`; UI work is not DSP, hardware, or package acceptance.

## UI-00 — Audit Frontend And Design Contract
Status: VERIFIED
Prerequisites: none

Changed files:
- `AGENTS.md`: repository execution, modularity, and native-only audio rules.
- `.github/copilot-instructions.md`: AI assistant instructions aligned to the specification.
- `docs/task-plan.json`: canonical 49-phase dependency plan from section 8E.1.
- `docs/progress.md`: progress and evidence log.

Implemented behavior:
- Node/npm environment audited for UI scaffold.
- Design source selected as section 8D because the approved image file is unavailable in this repository.
- UI-00 to UI-04 are established as the active dependency chain before native work.

Validation:
- command: `node --version`
- environment: macOS shell, arch not yet audited for native phase
- exit/result: `0`, output `v24.15.0`
- command: `npm --version`
- exit/result: `0`, output `11.12.1`
- command: `git status --short`
- exit/result: `128`, not a git repository
- hardware/manual: NOT_RUN, UI phase only

Known limitations:
- Approved screenshot asset uses a project-local name, `docs/design/Digital Mixer Audio.png`, rather than the spec's suggested `docs/design/approved-mixer-layout.png`.
- No native audio, device, recording, or engine claims have been made.

Next exact action:
- Complete UI-01 scaffold, guards, PreviewAdapter, and skeleton layout.

## UI-01 — Scaffold And State Adapter
Status: VERIFIED
Prerequisites: UI-00 VERIFIED

Changed files:
- `package.json`, `tsconfig.json`, `vite.config.ts`, `index.html`: React/Vite UI preview scaffold with exact dependency pins.
- `scripts/check-plan.mjs`, `scripts/check-file-size.mjs`, `scripts/check-architecture.mjs`: initial verification guards.
- `apps/desktop/src/adapters/MixerControlPort.ts`: typed UI control port and snapshot contracts.
- `apps/desktop/src/adapters/preview/PreviewAdapter.ts`: silent deterministic PreviewAdapter.
- `apps/desktop/src/fixtures/approvedMixerSession.ts`, `apps/desktop/src/fixtures/fxPrograms.ts`: preview session and 99-program metadata.
- `apps/desktop/electron/main.ts`, `apps/desktop/electron/preload.ts`, `apps/desktop/electron/tsconfig.json`: Electron desktop shell with security defaults for UI preview.
- `scripts/dev-desktop-ui.mjs`: dev launcher that starts Vite and opens the Electron shell.

Implemented behavior:
- Browser UI preview scaffold with React/Vite.
- Desktop UI preview opens through Electron using the same React component tree.
- Electron desktop preview default window is `1500x930` to keep the mixer bank close to the EQ panel while leaving bottom controls visible after spacing refinements; `1280x800` minimum support remains.
- Preview mode is explicit and silent; no native engine dependency is required.
- Guard scripts for plan, architecture, and file-size are available.

Validation:
- command: `npm install`
- exit/result: `0`; lockfile generated; npm audit reports 6 vulnerabilities in frontend dependency tree.
- command: `npm run check:plan`
- exit/result: `0`; 49 phases validated.
- command: `npm run check:file-size`
- exit/result: `0`
- command: `npm run check:architecture`
- exit/result: `0`
- command: `npm run typecheck`
- exit/result: `0`
- command: `npm run build:desktop:main`
- exit/result: `0`
- command: `npm run dev:desktop:ui`
- exit/result: process running; Electron desktop UI preview launched with Vite at `http://127.0.0.1:5173/`

Known limitations:
- NativeAdapter is intentionally absent until after UI-04.
- Electron shell is development UI preview only; packaged app, native helper, and engine supervision are Phase01+ work after UI-04.

Next exact action:
- Complete UI-02 layout fidelity.

## UI-02 — Layout Mixer Fidelity
Status: VERIFIED
Prerequisites: UI-01 VERIFIED

Changed files:
- `apps/desktop/src/components/ui/*`: reusable buttons, badges, and select fields.
- `apps/desktop/src/components/audio/*`: reusable rotary, fader, meter, clip, numeric parameter, and EQ graph controls.
- `apps/desktop/src/features/fx/components/*`: compact FX rows and program picker.
- `apps/desktop/src/features/mixer/*`: Mixer page, channel bank, and channel strips.
- `apps/desktop/src/features/processing/components/ChannelProcessingPanel.tsx`: right-side Channel Processing inspector.
- `apps/desktop/src/features/harmony/components/HarmonyQuickPanel.tsx`: Harmony tray.
- `apps/desktop/src/styles/*`: tokens and layout styling.

Implemented behavior:
- Layout follows the available reference image with top compact FX A/B rows, left mixer bank, right Channel Processing, Harmony tray, and footer monitor controls.
- Preview UI includes VOICE highlighted, INSERT FX and HARMONY as separate controls, CLIP, MON/REC, Modified, EQ graph, compressor, de-esser, and linked SEND A panel.
- Typography, knob/fader dimensions, FX macro layout, inspector graph, and Harmony tray were compacted to avoid large labels overflowing their containers.
- Channel strip lower controls now align to a consistent bottom dock; non-vocal source strips reserve Harmony spacing invisibly so fader/meter/action groups do not hang at different heights.
- Added aligned strip dividers above the pan/send area; master reserves matching upper spacing so its divider aligns with source channels.
- Master fader and meter are taller than source strips and extend upward toward the aligned divider.
- Reduced UI font weights and added spacing between labels and controls so the mixer reads less bold and less crowded.
- Right-side Channel Processing inspector was scaled down: smaller title text, tighter cards, shorter EQ graph, smaller inspector knobs, and reduced parameter typography.
- Harmony tray now uses two explicit rows: enable/key/scale/voice controls on the first row, and level/advanced/helper controls on the second row to reduce crowding.
- FX A/B compact rows now use fixed grid columns so macro knobs align between rows, and return meters show explicit L/R channel labels.
- FX A/B return control groups are right-aligned with shorter sliders so they no longer start too close to the macro knobs.
- Right-side workspace now separates Channel Processing from a new Sound Pads panel, using the formerly empty lower area for common effect buttons such as applause, laugh, cheer, drum roll, ding, and whoosh.

Validation:
- command: `npm run build:ui`
- exit/result: `0`
- command: `npm run test:visual`
- exit/result: `0`; screenshots written to `docs/reports/ui/desktop-1680x945.png` and `docs/reports/ui/minimum-1280x800.png`
- manual visual inspection: PASSED_WITH_NOTES; 1280 layout is dense but required regions and primary controls are visible without obvious text overflow, and channel bottoms align more consistently.
- hardware/manual audio: NOT_RUN, UI preview only.

Known limitations:
- This is not native audio and does not prove capture, playback, DSP, recording, export, or hardware behavior.
- Layout fidelity is close to the reference, but UI-04 final gate still requires final screenshot review before native Phase00.

Next exact action:
- Complete UI-03 interactions: full control editing, linked SEND A updates from both locations, program search behavior, EQ drag/numeric edits, MON/REC toggles, and edit popover.

## UI-03 — Interaksi Preview Koheren
Status: VERIFIED
Prerequisites: UI-02 VERIFIED

Changed files:
- `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`: expanded preview command contract and state mutation for mixer controls, FX returns, toggles, Harmony level, and EQ bands.
- `apps/desktop/src/components/audio/RotaryKnob.tsx`, `VerticalFader.tsx`, `EqResponseGraph.tsx`, `NumericParameter.tsx`: reusable controls now accept range/input/drag updates while preserving compact styling.
- `apps/desktop/src/features/mixer/components/*`, `apps/desktop/src/features/fx/components/*`, `apps/desktop/src/features/processing/components/ChannelProcessingPanel.tsx`, `apps/desktop/src/features/harmony/components/HarmonyQuickPanel.tsx`: UI wires preview interactions to adapter state.
- `apps/desktop/src/styles/app.css`: pointer-enabled knob/fader styling and compact FX edit popover.
- `tests/ui/preview-adapter.test.ts`: adapter behavior tests for send linkage, FX program state isolation, and Harmony toggle scope.
- `tests/ui/visual.visual.ts`: visual screenshots plus UI interaction smoke test.

Implemented behavior:
- Selecting channel, fader changes, FX program selection, FX ON/OFF, Modified reset, Harmony toggle/config fields, and CLIP reset have preview-state plumbing.
- Trim, pan, fader, Send A/B knobs, mute, solo, monitor, and record-arm buttons mutate visible preview state.
- Channel-strip input level is labeled `Gain` to match hardware mixer terminology.
- Channel processor buttons are stateful per channel: `EQ`, `COMP`, `NOISE`, and `INSERT FX`; the old `GATE` label was replaced with `NOISE`.
- Active and bypassed channel processor buttons now have distinct visual states: active buttons use cyan border/glow/left accent, bypassed buttons are darker and muted.
- EQ band settings are scoped to the selected channel in preview state, so editing one channel's EQ does not mutate another channel's EQ.
- Channel Processing cards dim when the corresponding channel processor is bypassed from the strip.
- FX A/B return sliders mutate state and mark the edited unit as modified.
- Program picker is searchable through typed number/name input with Enter/blur commit.
- EQ graph nodes can be dragged or adjusted with keyboard arrows; Freq/Gain/Q fields commit typed values.
- Right-side Send A knob and selected channel Send A knob update the same linked state.
- Harmony Level slider is editable.
- FX Edit opens a compact popover with return control and current macro values.

Validation:
- command: `npm run test:ui`
- exit/result: `0`; 1 file, 6 tests passed.
- command: `npm run test:visual`
- exit/result: `0`; 3 Playwright tests passed, including screenshot capture, `Gain` interaction, active/bypassed processor state assertion, `NOISE` toggle, and preview-control mutation smoke test.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI unit test, native sound-pad validation, UI build, and Electron main build passed.

Known limitations:
- UI-03 remains preview-only. These controls do not claim native DSP, routing, recording, or hardware behavior.

Next exact action:
- Complete UI-04 visual QA/gate with refreshed screenshots and final interaction/layout review, then move to Phase00 native dependency audit.

## UI-04 — Visual QA And Gate Before Engine
Status: UI_VERIFIED
Prerequisites: UI-03 VERIFIED

Changed files:
- `docs/design/Digital Mixer Audio Final.png`: final approved visual reference supplied by the user.
- `docs/reports/ui/desktop-1680x945.png`, `docs/reports/ui/minimum-1280x800.png`: refreshed visual evidence screenshots.
- `docs/reports/ui-layout-review.md`: UI-04 visual QA report updated against the final reference.
- `docs/task-plan.json`: UI-04 moved to `UI_VERIFIED`.
- `apps/desktop/src/styles/app.css`: left-zone overflow clipped so minimum viewport channel bank does not overlap the right processing panel.

Implemented behavior:
- Final visual QA uses `docs/design/Digital Mixer Audio Final.png` as the primary reference.
- UI intentionally reflects the final reference evolution: `Gain` label, `NOISE` processor, Sound Pads panel, active/bypassed processor button states, compact right processing panel, and two-row Harmony tray.
- Desktop viewport keeps the main mixer bank, right processing panel, Harmony tray, Sound Pads, and footer visible.
- Minimum viewport remains dense but avoids incoherent overlap; channel bank overflow is contained in the left zone and remains horizontally scrollable.

Validation:
- command: `file docs/design/Digital Mixer Audio Final.png`
- exit/result: `0`; PNG image data, `3002 x 1800`, 8-bit RGBA.
- command: `npm run test:visual`
- exit/result: `0`; 3 Playwright tests passed and refreshed `desktop-1680x945` plus `minimum-1280x800` screenshots.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI tests, native sound-pad validation, UI build, and Electron main build passed before final documentation update.

Known limitations:
- UI_VERIFIED means layout and preview interactions are accepted; it does not verify native capture, DSP, routing, recording, export, hardware devices, or packaged app behavior.
- Browser visual tests do not prove audible sound-pad playback; desktop playback remains native helper based.

Next exact action:
- Start Phase00 native dependency audit: pin JUCE/native toolchain requirements, verify local macOS build prerequisites, and prepare the native engine path without Web Audio/browser fallback.

## Phase00 — Native Dependency Audit
Status: VERIFIED
Prerequisites: UI-04 UI_VERIFIED

Changed files:
- `docs/dependency-manifest.md`: pinned UI/runtime/native dependency manifest, including JUCE tag SHA and macOS deployment target.
- `docs/adr/0001-native-audio-stack.md`: accepted stack decision for Electron UI plus C++20/JUCE/Core Audio native audio.
- `docs/reports/doctor-phase00.md`: doctor report from local toolchain audit.
- `scripts/doctor.mjs`, `package.json`: read-only doctor command for repeatable environment checks.
- `docs/task-plan.json`: Phase00 moved to `VERIFIED`.

Implemented behavior:
- Phase00 keeps the UI/Electron shell and native engine boundaries separate.
- Production audio remains constrained to native C++20/JUCE/Core Audio or approved native workers; no Web Audio/browser fallback was introduced.
- JUCE is pinned to `8.0.15` at tag SHA `91ad83ae34a81e0833b1a2b0866f54846370ae53`.
- Initial macOS deployment target is `14.0`; newer Core Audio APIs require runtime availability guards.
- Doctor script reads environment only and reports tool availability.

Validation:
- command: `uname -m`
- exit/result: `0`; `arm64`.
- command: `sw_vers`
- exit/result: `0`; macOS `26.6.2`, build `25G83`.
- command: `xcode-select -p`
- exit/result: `0`; `/Library/Developer/CommandLineTools`.
- command: `xcrun --show-sdk-path`
- exit/result: `0`; `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`.
- command: `xcrun --sdk macosx --show-sdk-version`
- exit/result: `0`; `26.5`.
- command: `clang --version`
- exit/result: `0`; Apple Clang `21.0.0 (clang-2100.1.1.101)`.
- command: `cmake --version`
- exit/result: `0`; CMake `4.3.2`.
- command: `brew install ninja`
- exit/result: `0`; Ninja installed by Homebrew at version `1.13.2`.
- command: `ninja --version`
- exit/result: `0`; `1.13.2`.
- command: `node --version`
- exit/result: `0`; `v24.15.0`.
- command: `npm --version`
- exit/result: `0`; `11.12.1`.
- command: `git ls-remote https://github.com/juce-framework/JUCE.git refs/tags/8.0.15 refs/tags/8.0.15^{}`
- exit/result: `0`; JUCE tag SHA `91ad83ae34a81e0833b1a2b0866f54846370ae53`.
- command: `npm run doctor`
- exit/result: `0`; report prints OS/toolchain/package pins and Ninja `1.13.2`.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI unit tests, native sound-pad build/asset validation, UI build, and Electron main build passed after Phase00 updates.

Known limitations:
- No JUCE source has been downloaded or built yet; Phase00 pins the dependency only.
- No native engine, Core Audio device, DSP, routing, recording, export, or hardware behavior is verified by this phase.

Next exact action:
- Start Phase01 native executable scaffolding with CMake + Ninja + Apple Clang.

## Phase01 — Native Executable Connected To Desktop Shell
Status: VERIFIED
Prerequisites: Phase00 VERIFIED

Changed files:
- `CMakeLists.txt`, `native/engine/CMakeLists.txt`: CMake/Ninja native engine project scaffold.
- `native/engine/src/main.cpp`: `local-mixer-engine` Phase01 executable with `--version` and `--self-test`.
- `native/engine/src/dsp/Gain.hpp`, `native/engine/src/dsp/Gain.cpp`: tiny portable DSP utility used only for native test scaffolding.
- `native/engine/tests/GainTest.cpp`: portable native DSP test target.
- `scripts/build-native.mjs`, `scripts/test-native.mjs`, `scripts/dev-engine.mjs`, `scripts/package-mac-unsigned.mjs`: root native build/test/dev scaffolding.
- `package.json`: added root `dev`, `dev:engine`, `build:native`, `build`, `test:native`, `test:e2e`, and `package:mac:unsigned` scripts; `verify` now includes native tests.
- `apps/desktop/electron/main.ts`: native resource path helpers and explicit `LOCAL_MIXER_REQUIRE_ENGINE=1` validation for engine-mode launches.
- `apps/desktop/electron/main.ts`: closing the last desktop window now quits Electron in dev/package flow so launcher scripts can stop child processes.
- `docs/task-plan.json`: Phase01 moved to `IMPLEMENTED_UNVERIFIED`.

Implemented behavior:
- `dev` remains an alias for `dev:ui`; engine mode is explicit through `dev:engine`.
- `dev:engine` builds native targets first, builds Electron main/preload, requires the native engine binary, and passes an explicit engine path to Electron.
- Native engine executable is separate from the earlier sound-pad helper.
- Native build uses CMake + Ninja + Apple Clang with `CMAKE_OSX_DEPLOYMENT_TARGET=14.0`.
- Portable native tests run through CTest and do not depend on Electron, Core Audio, hardware, or browser APIs.
- Electron packaged/dev resource path helpers avoid assuming `cwd` for packaged resource lookup.
- Closing the desktop window now requests app quit instead of leaving the macOS app process alive with no window.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake configured/generated, Ninja built `local-mixer-engine` and `local-mixer-dsp-tests`, CTest passed `1/1`, engine `--self-test` passed, engine `--version` printed Phase01 JSON.
- command: `file native/engine/build/native/engine/local-mixer-engine native/engine/build/native/engine/local-mixer-dsp-tests`
- exit/result: `0`; both binaries are `Mach-O 64-bit executable arm64`.
- command: `native/engine/build/native/engine/local-mixer-engine --version`
- exit/result: `0`; `{"name":"local-mixer-engine","version":"0.1.0-phase01","protocol":1,"audio":"not-started","juce":"pinned-8.0.15-not-linked"}`.
- command: `ctest --test-dir native/engine/build --output-on-failure`
- exit/result: `0`; `100% tests passed, 0 tests failed out of 1`.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI tests, native tests, UI build, and Electron main build passed.
- command: `npm run build:desktop:main`
- exit/result: `0`; Electron main/preload rebuilt after quit lifecycle change.
- manual: `npm run dev:engine`
- result: PASSED_BY_USER; desktop app opened, closing the desktop window returned the terminal to prompt after the Electron quit lifecycle fix.

Known limitations:
- JUCE is pinned but not downloaded or linked yet; `local-mixer-engine` is a C++20 Phase01 scaffold, not the full JUCE/Core Audio engine.
- No capture, playback, device enumeration, routing, recording, export, plugin hosting, or native DSP feature is implemented by Phase01.
- `package:mac:unsigned` is a reserved script that intentionally fails until the packaging phase.

Next exact action:
- Proceed to Phase02 protocol/supervisor engine: JSONL parser, handshake, bounded messages, child lifecycle, error states, and crash handling.

## Phase02 — Protocol And Engine Supervisor
Status: VERIFIED
Prerequisites: Phase01 VERIFIED

Changed files:
- `apps/desktop/electron/EngineProtocol.ts`: shared protocol constants, engine states, bounded message size, and message types.
- `apps/desktop/electron/EngineSupervisor.ts`: JSONL engine supervisor with spawn, handshake, bounded parsing, command timeout, stderr tail, EOF/crash handling, and restart-rate guard.
- `apps/desktop/electron/main.ts`: `dev:engine` path starts the supervisor when `LOCAL_MIXER_REQUIRE_ENGINE=1` and stops it during app quit.
- `native/engine/src/main.cpp`: added `--stdio` JSONL protocol mode with hello, ping, shutdown, malformed/invalid command errors, and test crash command.
- `tests/electron/fake-engine.mjs`, `tests/electron/engine-supervisor.test.ts`: fake-engine tests for handshake, command timeout, malformed output, oversized output, unexpected EOF, and restart limiting.
- `scripts/test-native.mjs`: native engine protocol smoke test added to CMake/CTest/native self-test flow.
- `docs/task-plan.json`: Phase02 moved to `VERIFIED`.

Implemented behavior:
- Engine process is launched via `spawn` argument array with `shell:false`.
- `dev:engine` no longer only validates the file path; it starts the native engine stdio protocol and waits for a matching protocol handshake before showing the desktop window.
- Supervisor states include `STOPPED`, `STARTING`, `RUNNING`, `RECONFIGURING`, `RECOVERING`, and `ERROR`.
- JSONL stdout messages are bounded to 8192 bytes; malformed and oversized messages move supervisor state to `ERROR`.
- Commands carry ids and are rejected on timeout instead of hanging UI/control code.
- Unexpected EOF before handshake is reported as an error.
- Fake engine is used only in tests; production/dev engine mode uses the native executable path.

Validation:
- command: `npm run test:ui`
- exit/result: `0`; 2 files, 12 tests passed including 6 EngineSupervisor tests.
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed, engine self-test passed, engine JSONL protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI/supervisor tests, native tests, UI build, and Electron main build passed.

Known limitations:
- Phase02 does not implement audio devices, Core Audio callbacks, passthrough, DSP processing, meters, recording, export, or plugin hosting.
- Engine crash/error is surfaced through supervisor state and rejected commands; broader in-renderer user-facing error panels are deferred until the native engine state is connected to runtime UI flows.
- Auto-restart policy is guarded by rate limit but not yet used for real restart loops; recovery behavior belongs to later engine lifecycle phases.

Next exact action:
- Continue Phase03 toward real device open, no-monitor default, and explicit passthrough mapping.

## Phase03 — Audio Device And Passthrough Foundation
Status: VERIFIED
Prerequisites: Phase02 VERIFIED

Changed files:
- `CMakeLists.txt`, `native/engine/CMakeLists.txt`: native engine build now enables Objective-C++ on Apple and links AVFoundation/Core Audio/Core Foundation for permission status, device enumeration, and output callback.
- `native/engine/src/platform/macos/CoreAudioDevices.hpp`, `native/engine/src/platform/macos/CoreAudioDevices.mm`: Core Audio hardware device enumeration with UID, name, default input/output flags, input/output channel counts, and nominal sample rate.
- `native/engine/src/platform/macos/CoreAudioInputMeter.hpp`, `native/engine/src/platform/macos/CoreAudioInputMeter.mm`: Core Audio HAL input callback for short peak measurement from default or explicit input UID.
- `native/engine/src/platform/macos/CoreAudioOutputStream.hpp`, `native/engine/src/platform/macos/CoreAudioOutputStream.mm`: Core Audio HAL output callback for protected test tone playback, with explicit output UID selection and default-output fallback.
- `native/engine/src/platform/macos/CoreAudioPassthrough.hpp`, `native/engine/src/platform/macos/CoreAudioPassthrough.mm`: short native input-to-output monitor path using preallocated ring buffer, explicit input/output UIDs, explicit channels, conservative monitor gain, zeroed output buffers, and output limiting.
- `native/engine/src/platform/macos/CoreAudioDevices.hpp`, `native/engine/src/platform/macos/CoreAudioDevices.mm`: added native microphone permission request path in addition to permission status query.
- `native/engine/src/dsp/OutputProtection.hpp`, `native/engine/src/dsp/OutputProtection.cpp`: native master gain, mute, ceiling limiter, and -30 dBFS test-tone utility.
- `native/engine/src/engine/DeviceService.hpp`, `native/engine/src/engine/DeviceService.cpp`: native device catalog/UID lookup/channel labels, project rate/block validation, explicit one-channel passthrough mapping, monitoring-off zero output, and bounded chunk rendering.
- `native/engine/src/engine/EngineRuntime.hpp`, `native/engine/src/engine/EngineRuntime.cpp`: native runtime status, default device selection, prepared passthrough lifecycle, and explicit error-state reporting.
- `native/engine/tests/OutputProtectionTest.cpp`: native tests for limiter ceiling, mute, and test tone level.
- `native/engine/tests/DeviceServiceTest.cpp`: native tests for UID lookup, channel labels, sample-rate mismatch rejection, oversized block rejection, monitoring-off silence, explicit channel passthrough, and limiter-protected render output.
- `native/engine/tests/EngineRuntimeTest.cpp`: native tests for no-device error state, default input/output selection, monitoring-off prepared state, and sample-rate mismatch error propagation.
- `native/engine/src/main.cpp`: added `--list-devices`, `--test-tone`, `--meter-input`, and `--monitor-passthrough`; stdio protocol now handles `engine-status`, `list-devices`, `prepare-passthrough`, `play-test-tone`, `meter-input`, and `monitor-passthrough`; self-test now includes output protection.
- `native/engine/src/main.cpp`: `--meter-input` now accepts `--input-uid`, `--sample-rate`, and `--duration-ms`; `--test-tone` accepts `--output-uid`, `--sample-rate`, `--duration-ms`, and `--monitor-gain-db` for explicit device testing.
- `apps/desktop/electron/main.ts`, `apps/desktop/electron/preload.ts`, `apps/desktop/electron/preload.cjs`: desktop engine IPC bridge exposes a whitelist for `engine-status`, `list-devices`, `meter-input`, `play-test-tone`, `monitor-passthrough`, and `prepare-passthrough`.
- `apps/desktop/src/features/hardware/components/HardwareMonitorPanel.tsx`, `apps/desktop/src/features/mixer/MixerPage.tsx`, `apps/desktop/src/styles/app.css`, `apps/desktop/src/types/localMixer.d.ts`: desktop app now exposes Hardware Monitor as a modal opened from a small footer `HW` button, with device refresh, input/output selectors, input meter, output tone, and 3-second monitor command.
- `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`, `apps/desktop/src/fixtures/approvedMixerSession.ts`, `apps/desktop/src/features/mixer/components/ChannelBank.tsx`, `apps/desktop/src/features/mixer/components/ChannelStrip.tsx`, `apps/desktop/src/styles/app.css`: each mixer channel now has an independent ON/OFF state and compact header button; disabled channels dim and show silent preview meters.
- `apps/desktop/src/features/mixer/MixerPage.tsx`, `apps/desktop/src/features/mixer/components/ChannelBank.tsx`, `apps/desktop/src/features/mixer/components/ChannelStrip.tsx`, `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`: source channels now support compact input-device selectors from native `list-devices`; footer output selector now uses native output devices as the global monitor/master output.
- `native/engine/src/main.cpp`: added `--request-mic-permission` and stdio `request-mic-permission`.
- `apps/desktop/electron/main.ts`: `dev:engine` now requests native microphone permission after engine handshake.
- `apps/desktop/electron/EngineSupervisor.ts`, `tests/electron/fake-engine.mjs`, `tests/electron/engine-supervisor.test.ts`: guarded closed engine stdin writes so closing the app or a dead engine no longer raises uncaught `EPIPE`.
- `scripts/test-native.mjs`: native verification now includes JSON smoke checks for `--list-devices`, protocol `list-devices`, protocol `prepare-passthrough`, zero-duration protocol `play-test-tone`, zero-duration protocol `meter-input`, and zero-duration protocol `monitor-passthrough`.
- `docs/task-plan.json`: Phase03 moved to `VERIFIED` after hardware evidence was supplied from the user's desktop session.

Implemented behavior:
- Device enumeration is native-only through Core Audio on macOS.
- Device list output is JSON and bounded to native executable commands, not browser audio APIs.
- Output protection primitives exist before any future physical output callback: mute, master gain, sample clamp limiter, and conservative test tone generation.
- DeviceService now rejects missing devices, invalid channel mapping, sample-rate mismatches, and oversized project blocks before rendering.
- Passthrough rendering is explicit and starts silent because input monitoring defaults off.
- Engine stdio exposes device/status commands for the desktop shell without involving renderer-side audio APIs.
- Microphone permission state is reported natively as `AUTHORIZED`, `DENIED`, `RESTRICTED`, `NOT_DETERMINED`, `UNAVAILABLE`, or `UNSUPPORTED_PLATFORM`.
- Native microphone permission can now be requested by the engine; if macOS already records `DENIED`, the command reports `DENIED` without prompting until the user resets TCC permission.
- Protected test tone is implemented in a native Core Audio output callback; automated tests use zero-duration command smoke only so verification does not unexpectedly play audio.
- Input signal presence can now be tested from native code with `--meter-input`; it returns a peak value without routing mic audio to speakers.
- Input/output hardware checks can target explicit device UIDs instead of only macOS defaults.
- Short passthrough monitoring is explicit-only through `--monitor-passthrough`; it defaults to `-24 dB` monitor gain and never starts automatically.
- Passthrough CLI now mirrors mono mic monitoring to all output channels by default so headset tests are audible on left/right; `--single-output-channel` keeps explicit one-channel output checks.
- Desktop app hardware testing is available only in `dev:engine` mode through the native engine bridge; browser UI preview remains silent. Hardware Monitor is a debug modal, not a permanent right-panel production surface.
- Channel ON/OFF is represented in app state separately from mute/solo so later native routing can exclude disabled sources without conflating that with mix mute.
- Channel MON in the desktop app can trigger the current short native monitor command using that channel's selected input and the footer's selected output; persistent graph monitoring remains a later engine-routing step.
- Engine shutdown/write races are handled by supervisor rejection paths instead of uncaught `EPIPE` errors.
- No Web Audio, browser capture, or renderer-side DSP was introduced.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed including Core Audio input/output/passthrough stream files, CTest passed 4/4 including output protection, DeviceService, and EngineRuntime, engine self-test passed, device enumeration smoke passed, JSONL protocol smoke passed including zero-duration `play-test-tone`, `meter-input`, and `monitor-passthrough`.
- command: `native/engine/build/native/engine/local-mixer-engine --list-devices`
- exit/result: `0`; returned valid JSON with `devices: []` and `micPermission: DENIED` in this command environment.
- command: `native/engine/build/native/engine/local-mixer-engine --request-mic-permission`
- exit/result: `0`; returned `{"micPermission":"DENIED"}`, confirming the environment is already denied and will not show a prompt until reset.
- command: `native/engine/build/native/engine/local-mixer-engine --meter-input`
- exit/result: `0`; returned `{"measured":false,"error":"NO_INPUT_DEVICE","inputChannels":0,"actualSampleRate":0.000000,"peak":0.000000}` in this command environment.
- command: `native/engine/build/native/engine/local-mixer-engine --monitor-passthrough --input-uid BuiltInHeadphoneInputDevice --duration-ms 0`
- exit/result: `0`; returned `{"monitored":false,"error":"NO_INPUT_DEVICE","inputChannels":0,"outputChannels":0,"inputSampleRate":0.000000,"outputSampleRate":0.000000,"inputPeak":0.000000}` in this command environment.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI/supervisor tests, native tests, UI build, and Electron main build passed.
- command: `npm run typecheck`
- exit/result: `0`; Hardware Monitor panel and bridge types passed.
- command: `npm run build:desktop:main`
- exit/result: `0`; Electron main/preload rebuilt after engine bridge changes.
- manual/user: `native/engine/build/native/engine/local-mixer-engine --meter-input --input-uid "BuiltInHeadphoneInputDevice"`
- result: PASSED_BY_USER; returned `{"measured":true,"error":"","inputChannels":1,"actualSampleRate":48000.000000,"peak":1.000000}`. Mic input was detected, but peak showed clipping.
- manual/user: `native/engine/build/native/engine/local-mixer-engine --monitor-passthrough ...`
- result: PASSED_BY_USER; returned `{"monitored":true,"error":"","inputChannels":1,"outputChannels":2,"inputSampleRate":48000.000000,"outputSampleRate":48000.000000,"inputPeak":1.000000}` and user confirmed passthrough was audible after mono input was mirrored to both output channels.

Known limitations:
- Physical evidence is user-run from the normal desktop session because this command environment returned zero Core Audio devices and microphone permission was `DENIED`.
- Input peak reached `1.000000`; mic gain calibration/clip handling must be handled in later mixer gain stages.
- Passthrough is a short explicit monitor command, not the final persistent mixer graph monitoring path.

Next exact action:
- Continue to Phase05 multi-source engine/channel strip foundation.

## Phase05 — Multi-Source Engine And Basic Channel Strips
Status: IMPLEMENTED_UNVERIFIED
Prerequisites: Phase03 VERIFIED

Changed files:
- `native/engine/src/engine/MixerGraph.hpp`, `native/engine/src/engine/MixerGraph.cpp`: native 32-strip mixer graph foundation with create/remove/rename/color, mono/stereo assignment, trim, fader, pan, mute, solo, enabled state, input monitoring flag, stale ID rejection, and separate input/output strip meters.
- `native/engine/tests/MixerGraphTest.cpp`: native fixture tests for known-amplitude summing, mute isolation, solo isolation, disabled channel silence, stale ID rejection, 32-strip capacity, and 32 silent strip processing.
- `native/engine/CMakeLists.txt`: added `MixerGraph` to native library and `local-mixer-graph-tests` to CTest.
- `native/engine/src/main.cpp`: engine self-test now includes a minimal mixer graph sum; version now reports `0.1.0-phase05-foundation`.
- `docs/task-plan.json`: Phase03 moved to `VERIFIED`; Phase05 moved to `IMPLEMENTED_UNVERIFIED`.

Implemented behavior:
- Native mixer graph can sum multiple source buffers into stereo output.
- Source A changes do not alter source B state; mute/solo/disable logic is per-strip.
- 32 strip pool limit is enforced and the 33rd strip is rejected.
- Stale strip IDs are rejected during remove/process instead of being ignored.
- Output buffers are cleared before each process call.
- Input and output peak meters are tracked separately per strip.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed 5/5 including `local-mixer-graph-tests`, engine self-test passed, engine version reported `0.1.0-phase05-foundation`, device/protocol smoke passed.

Known limitations:
- Phase05 is not fully acceptance-verified yet: graph update prepare/publish/reclaim and bounded parameter queue are implemented and unit-tested, but not wired as the persistent realtime callback owner used by desktop MON.
- The graph is not yet the persistent realtime callback owner used by the desktop MON button; short Phase03 monitor command remains in use for hardware checks.

Next exact action:
- Continue Phase05 by wiring desktop channel state to the native graph and replacing short MON passthrough with graph-owned monitoring.

### Phase05 Checkpoint — Bounded Controls And Graph Publication

Changed files:
- `native/engine/src/engine/MixerControlQueue.hpp`, `native/engine/src/engine/MixerControlQueue.cpp`: bounded FIFO for channel control commands.
- `native/engine/src/engine/MixerGraphController.hpp`, `native/engine/src/engine/MixerGraphController.cpp`: prepare/publish/reclaim wrapper for graph updates outside the callback path.
- `native/engine/src/engine/MixerGraph.hpp`, `native/engine/src/engine/MixerGraph.cpp`: preallocated strip storage, queued control application, fader/pan ramp runtime, and independent input meter behavior.
- `native/engine/tests/MixerGraphTest.cpp`: coverage for queue overflow/FIFO, stale queued command rejection, ramped level changes, muted input metering, and publish/reclaim behavior.
- `native/engine/CMakeLists.txt`: added new engine graph control modules to the native library.

Implemented behavior:
- Parameter changes can be queued through a bounded command queue and applied with a fixed-frame ramp.
- Input meters remain active from source samples even when a strip is muted, while output meters stay silent.
- Prepared graph changes do not mutate active graph state until publish; retired graphs are reclaimed explicitly off the callback path.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed 5/5 including expanded `local-mixer-graph-tests`, engine self-test passed, device/protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

### Phase05 Checkpoint — Desktop State Sync To Native Graph

Changed files:
- `native/engine/src/main.cpp`: added `sync-mixer-graph` stdio command that validates a flat channel snapshot, prepares a native `MixerGraph`, publishes it through `MixerGraphController`, and reports strip/monitor counts.
- `native/engine/src/engine/MixerGraph.hpp`, `native/engine/src/engine/MixerGraph.cpp`: strip configs now retain `sourceUid` so selected input devices can be owned by the native graph state.
- `apps/desktop/electron/main.ts`: whitelisted `sync-mixer-graph` for the Electron IPC bridge.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: desktop UI now syncs channel enabled/mute/solo/monitor/source/gain/fader/pan state to the native engine when state changes.
- `scripts/test-native.mjs`: protocol smoke now verifies `sync-mixer-graph` over the engine stdio path.
- `native/engine/tests/MixerGraphTest.cpp`: verifies `sourceUid` is stored on strip config.

Implemented behavior:
- Desktop channel state has a native graph owner path; source UID selections are included in the graph sync payload.
- The new sync path does not use Web Audio, renderer audio capture, PCM IPC, or browser playback APIs.
- Native graph publication remains transactional: prepared graph is published only after validation succeeds.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed 5/5, engine self-test passed, and protocol smoke confirmed `sync-mixer-graph` with `stripCount: 2` and `activeMonitorCount: 1`.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- The desktop MON button still uses the short Phase03 `monitor-passthrough` command for audible monitoring.
- Persistent graph-owned realtime monitoring, live graph meters, and graph-owned output routing remain the next Phase05 work.

### Phase05 Checkpoint — Persistent Native Monitor Lifecycle

Changed files:
- `native/engine/src/platform/macos/CoreAudioPassthrough.hpp`, `native/engine/src/platform/macos/CoreAudioPassthrough.mm`: added a persistent native monitor class with start/stop/status lifecycle while retaining the existing short passthrough diagnostic command.
- `native/engine/src/main.cpp`: added `start-mixer-monitor`, `stop-mixer-monitor`, and `mixer-monitor-status` stdio commands using the last synced native graph monitor selection.
- `apps/desktop/electron/main.ts`: whitelisted persistent monitor lifecycle commands.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: channel MON now syncs the native graph, starts/stops persistent native monitoring, and stops monitoring when a channel is disabled.
- `scripts/test-native.mjs`: protocol smoke now verifies monitor start/status/stop responses.

Implemented behavior:
- Desktop MON no longer calls the short fixed-duration `monitor-passthrough` path.
- A native monitor can stay running until explicitly stopped by the desktop app or engine shutdown.
- Native start uses the source/output selected through the graph sync payload; no browser audio APIs or PCM renderer IPC were introduced.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed 5/5, engine self-test passed, and protocol smoke covered graph sync plus monitor start/status/stop.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Persistent monitor currently supports one active monitored source; multiple MON source strips are rejected with `MULTIPLE_MONITOR_SOURCES_UNSUPPORTED`.
- The persistent monitor still uses the CoreAudio passthrough ring rather than rendering a full multi-source `MixerGraph` through the output callback.
- Manual audible desktop verification for persistent MON has not been run in this turn.

### Phase05 Checkpoint — Native Monitor Renders Through MixerGraph

Changed files:
- `native/engine/src/platform/macos/CoreAudioPassthrough.hpp`, `native/engine/src/platform/macos/CoreAudioPassthrough.mm`: persistent monitor output callback now feeds captured input through a preallocated native `MixerGraph` scratch buffer before writing to CoreAudio output.
- `native/engine/src/main.cpp`: synced channel Gain/Fader/Pan are carried into the persistent monitor request so audible monitoring follows the selected channel strip level and pan.

Implemented behavior:
- Persistent MON uses the native mixer graph path for monitored source gain/fader/pan instead of direct sample passthrough.
- Output callback uses preallocated scratch buffers and a prepared graph strip; no browser audio, renderer PCM IPC, or Web Audio fallback was introduced.
- One-shot `monitor-passthrough` remains available as a hardware diagnostic command.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native CMake/Ninja build passed, CTest passed 5/5, engine self-test passed, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Physical persistent monitoring is still limited to one active source device/channel until Phase04 aggregate/system routing work.
- Manual audible desktop verification for graph-rendered persistent MON has not been run in this turn.

### Phase05 Checkpoint — Exclusive Monitor Guard

Changed files:
- `apps/desktop/src/adapters/preview/PreviewAdapter.ts`: source MON is exclusive while persistent monitor is limited to one physical source.
- `tests/ui/preview-adapter.test.ts`: verifies enabling MON on one source turns off source MON on the previous source.
- `scripts/test-native.mjs`: verifies native `sync-mixer-graph` rejects multiple active monitor sources with `MULTIPLE_MONITOR_SOURCES_UNSUPPORTED`.

Implemented behavior:
- UI prevents accidental multi-monitor selection before aggregate/multi-device routing exists.
- Native engine still validates and rejects invalid multi-monitor payloads even if bypassing the UI.

Validation:
- command: `npm run test:ui`
- exit/result: `0`; 14 UI/Electron tests passed including exclusive source monitor coverage.
- command: `npm run test:native`
- exit/result: `0`; native protocol smoke verified multi-monitor rejection.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

## Phase04 — System Audio Capture And Routing Recovery
Status: IMPLEMENTED_UNVERIFIED
Prerequisites: Phase03 VERIFIED, Phase05 IMPLEMENTED_UNVERIFIED foundation

Changed files:
- `native/engine/src/engine/SystemRouting.hpp`, `native/engine/src/engine/SystemRouting.cpp`: native read-only route diagnostics for BlackHole detection, physical output validation, selected stereo ranges, sample-rate validation, and loopback-output rejection.
- `native/engine/tests/SystemRoutingTest.cpp`: covers missing BlackHole, valid default route, loopback output rejection, sample-rate mismatch, and invalid stereo input range.
- `native/engine/CMakeLists.txt`: adds the system routing module and `local-mixer-system-routing-tests`.
- `native/engine/src/main.cpp`: exposes `routing-system-diagnostics` over bounded stdio protocol and reports route validity/rejection reason without changing OS audio routing.
- `apps/desktop/electron/main.ts`: whitelists the route diagnostics command.
- `scripts/test-native.mjs`: protocol smoke verifies `routing-system-diagnostics` returns structured route status.
- `docs/setup/system-audio-routing.md`: manual BlackHole/Aggregate Device setup and recovery guidance.
- `docs/task-plan.json`: Phase04 moved to `IMPLEMENTED_UNVERIFIED`.

Implemented behavior:
- Engine can detect a BlackHole-like loopback device by UID/name.
- Engine rejects output routes that point back to BlackHole/loopback/aggregate devices to avoid feedback or double dry path.
- Engine reports selected stereo input/output channel ranges and sample-rate mismatch reasons.
- Phase04 foundation is read-only; it does not install drivers, create aggregate devices, restart `coreaudiod`, or alter macOS default output.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native CMake/Ninja build passed, CTest passed 6/6 including `local-mixer-system-routing-tests`, engine self-test passed, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Real BlackHole hardware/system-audio path was not available in this automated test environment; physical browser/QuickTime -> BlackHole -> engine -> headphones test remains `NOT_RUN`.
- System route transaction/restore is not implemented yet; no OS default output is changed by this checkpoint.
- Aggregate channel mapping wizard UI is not implemented yet.

### Phase04 Checkpoint — Desktop Route Diagnostics UI

Changed files:
- `apps/desktop/src/features/hardware/components/HardwareMonitorPanel.tsx`: hardware modal now exposes read-only system route diagnostics through the native `routing-system-diagnostics` command.
- `apps/desktop/src/styles/app.css`: compact modal layout updated with a route status row while keeping labels small and bounded.

Implemented behavior:
- User can run `Route Check` from the desktop hardware modal to see BlackHole readiness, selected stereo channel ranges, and route rejection reason.
- Diagnostics remain read-only and do not change macOS default output, install drivers, create aggregate devices, or route audio by themselves.

Validation:
- command: `npm run typecheck`
- exit/result: `0`; TypeScript passed.
- command: `npm run test:ui`
- exit/result: `0`; 14 UI/Electron tests passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Modal diagnostics depend on running desktop engine mode; browser/UI preview still cannot query native audio devices.
- No screenshot QA was run for this modal change in this turn.

### Phase04 Checkpoint — Route Transaction Ownership Foundation

Changed files:
- `native/engine/src/engine/SystemRouting.hpp`, `native/engine/src/engine/SystemRouting.cpp`: added route transaction state/ownership model for guarded enable/disable/restore flow.
- `native/engine/tests/SystemRoutingTest.cpp`: verifies engine-not-ready, invalid route, OS apply unavailable, active ownership, restore, and no-owned-route behavior.
- `native/engine/src/main.cpp`: exposes `routing-system-enable`, `routing-system-disable`, and `routing-system-status` over stdio.
- `apps/desktop/electron/main.ts`: whitelists route transaction commands.
- `scripts/test-native.mjs`: protocol smoke verifies transaction commands return structured state and no false success.
- `docs/setup/system-audio-routing.md`: documents current transaction commands and `OS_APPLY_UNAVAILABLE` behavior.

Implemented behavior:
- Native engine now has an explicit ownership state machine for system route changes.
- Enable refuses invalid diagnostics, engine-not-ready state, and missing OS apply support.
- Disable refuses to restore unowned routes and reports `NO_OWNED_ROUTE`.
- This checkpoint intentionally avoids changing macOS default output until the CoreAudio apply/restore adapter is implemented and verified.

Validation:
- command: `npm run test:native`
- exit/result: `0`; CMake/Ninja build passed, CTest passed 6/6, engine self-test passed, and protocol smoke covered route transaction commands.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Route apply/restore is still blocked with `OS_APPLY_UNAVAILABLE`; no macOS default output is changed by this checkpoint.
- Durable recovery marker across process restart is not implemented yet.

### Phase04 Checkpoint — Guarded CoreAudio Route Apply Adapter

Changed files:
- `native/engine/src/platform/macos/CoreAudioSystemRoute.hpp`, `native/engine/src/platform/macos/CoreAudioSystemRoute.mm`: native macOS adapter for reading current default output UID and setting default output UID.
- `native/engine/src/main.cpp`: `routing-system-enable` can call the CoreAudio route adapter only when `allowOsRouteChange: true`; default remains no OS change. `routing-system-disable` restores only an owned route with stored original output UID.
- `native/engine/CMakeLists.txt`: links the new macOS route adapter into the engine executable.
- `docs/setup/system-audio-routing.md`: documents the explicit `allowOsRouteChange` guard.
- `docs/task-plan.json`: Phase04 evidence includes the CoreAudio route adapter.

Implemented behavior:
- CoreAudio default output apply/restore code now exists behind an explicit protocol guard.
- Automated tests and current desktop UI do not change OS routing.
- Route ownership still flows through the native transaction manager, so disable does not restore routes it does not own.

Validation:
- command: `npm run test:native`
- exit/result: `0`; Objective-C++ CoreAudio route adapter compiled, CTest passed 6/6, engine self-test passed, and protocol smoke passed without changing OS route.

Known limitations:
- Manual route apply/restore with real BlackHole remains `NOT_RUN`.
- Durable recovery marker across app restart is still not implemented.

### Phase04 Maintenance — Protocol Helper Split

Changed files:
- `native/engine/src/engine/JsonProtocol.hpp`, `native/engine/src/engine/JsonProtocol.cpp`: extracted bounded JSONL field readers and response writers from `main.cpp`.
- `native/engine/src/engine/SystemRoutingJson.hpp`, `native/engine/src/engine/SystemRoutingJson.cpp`: extracted system route diagnostics/transaction JSON serialization.
- `native/engine/src/main.cpp`: reduced from 750 lines to 655 lines to keep protocol entry point maintainable before adding more Phase04 commands.
- `native/engine/CMakeLists.txt`: includes the new helper modules in the native engine executable.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 6/6, engine self-test, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

### Phase04 Checkpoint — Durable Route Recovery Marker

Changed files:
- `native/engine/src/engine/SystemRouteRecovery.hpp`, `native/engine/src/engine/SystemRouteRecovery.cpp`: added a small file-backed recovery marker store for owned system-route transactions.
- `native/engine/src/main.cpp`: saves the marker after an active owned route, exposes marker state in `routing-system-status`, and clears the marker after successful disable/restore.
- `native/engine/tests/SystemRoutingTest.cpp`: verifies marker save refusal for unowned routes, owned route round-trip, escaped UID handling, and clear behavior.
- `native/engine/CMakeLists.txt`, `scripts/test-native.mjs`: compile the marker module and smoke-test the new protocol status field.
- `docs/setup/system-audio-routing.md`, `docs/task-plan.json`, `docs/progress.md`: documented marker location, guarded behavior, and evidence paths.

Implemented behavior:
- Owned route transactions persist the original output UID and selected BlackHole/physical route to `~/Library/Application Support/Local Audio Mixer/route-recovery.marker`.
- Tests can override the marker path with `LOCAL_MIXER_ROUTE_RECOVERY_MARKER`.
- `routing-system-status` now reports `recoveryMarkerPresent` and `recoveryOriginalOutputUid`.
- `routing-system-disable` clears the marker only after the transaction reports a successful restore.
- Recovery marker metadata is passive; it does not auto-restore or change macOS output without an explicit route command.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 6/6, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Manual crash/restart recovery flow is `NOT_RUN`.
- Real `allowOsRouteChange: true` route apply/restore with BlackHole and physical hardware remains `NOT_RUN`.

### Phase04 Checkpoint — Desktop System Route Controls

Changed files:
- `apps/desktop/src/features/hardware/components/HardwareMonitorPanel.tsx`: added Loopback selection, guarded Enable Route, Disable, owned-route status, and recovery marker status in the desktop hardware modal.
- `apps/desktop/src/styles/app.css`: widened and compacted the hardware modal for three device selectors and route actions.
- `tests/ui/visual.visual.ts`: added a Playwright assertion that the hardware modal exposes system routing controls.
- `docs/progress.md`: recorded verification evidence for the checkpoint.

Implemented behavior:
- Opening the hardware modal still only refreshes devices and status; it does not change macOS routing.
- `Enable Route` asks for explicit confirmation before sending `routing-system-enable` with `allowOsRouteChange: true`.
- `Disable` calls `routing-system-disable` and is available when a route is owned or a recovery marker exists.
- The modal displays recovery marker presence reported by native `routing-system-status`.

Validation:
- command: `npm run test:visual`
- exit/result: `0`; 4 Playwright tests passed, including the hardware route controls assertion.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Real hardware route enable/disable was not clicked during automated verification; macOS default output was not changed by tests.
- Manual crash/restart recovery flow remains `NOT_RUN`.
- The desktop modal is a developer/control surface, not final first-run setup UX.

### Phase04 Checkpoint — Supervisor Route Restore And Startup Recovery

Changed files:
- `native/engine/src/main.cpp`: added `routing-system-recover`, which reads the durable marker, restores the marked original output through the native CoreAudio adapter, and clears the marker after success.
- `apps/desktop/electron/EngineSupervisor.ts`: added optional normal-stop route restore and startup marker recovery flows.
- `apps/desktop/electron/main.ts`: enabled supervisor route restore/recovery for desktop engine mode and whitelisted `routing-system-recover`.
- `apps/desktop/src/features/hardware/components/HardwareMonitorPanel.tsx`: uses `routing-system-recover` when only a marker remains after restart.
- `tests/electron/fake-engine.mjs`, `tests/electron/engine-supervisor.test.ts`, `scripts/test-native.mjs`: added fake-engine and protocol coverage for restore/recovery commands.
- `docs/setup/system-audio-routing.md`, `docs/progress.md`: documented the explicit recovery command and current behavior.

Implemented behavior:
- Normal desktop engine shutdown tries `routing-system-status` and `routing-system-disable` before engine shutdown when an owned route or marker is present.
- Desktop engine startup checks for a durable route marker and runs `routing-system-recover` when no live transaction owns the route.
- Recovery remains best-effort; manual macOS output recovery remains documented.

Validation:
- command: `npm run test:ui`
- exit/result: `0`; 16 tests passed, including supervisor stop/recover route cleanup cases.
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 6/6, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Real crash recovery with BlackHole/default-output switching remains `NOT_RUN` on hardware.
- The recovery command does not promise fail-open behavior after whole-app SIGKILL or power loss; it restores only when a later native engine process can run.

## Native Sound Pad Spike — Early User-Requested
Status: IMPLEMENTED_UNVERIFIED_PLAYBACK
Prerequisites: user explicitly requested this before UI-04

Changed files:
- `native/sound-pad/src/*`: small C++20 native helper that plays persistent local WAV assets and still keeps a procedural validation/render path for diagnostics.
- `assets/sound-pads/*.wav`: real CC0 sound pad WAV files used at click time.
- `docs/reports/sound-pad-assets.md`: source/license table for each sound pad asset.
- `scripts/build-native-sound-pad.mjs`: macOS native build script using Apple Clang; it validates persistent WAV assets without overwriting them.
- `apps/desktop/electron/main.ts`, `apps/desktop/electron/preload.ts`: Electron IPC bridge that launches the native helper for sound pad clicks.
- `apps/desktop/src/features/sound-pads/components/SoundPadPanel.tsx`: sound pad buttons now call native playback when the Electron bridge is available.
- `apps/desktop/electron/preload.cjs`, `scripts/build-desktop-main.mjs`, `scripts/dev-desktop-ui.mjs`, `package.json`: desktop dev launcher now builds the native helper and Electron main/preload before opening the app; CommonJS preload is copied into `dist/electron` so the sandboxed Electron bridge exposes `window.localMixer.playSoundPad`.

Implemented behavior:
- React/browser renderer does not use Web Audio, `<audio>`, `AudioContext`, browser capture, or JS audio sample processing.
- Sound pad playback path is renderer click -> preload IPC -> Electron main -> native C++ helper -> persistent WAV asset -> macOS `/usr/bin/afplay`.
- Click playback no longer creates or deletes temporary audio files.
- Sound pads now use local real CC0 samples from OpenGameArt instead of the generated placeholder tones/noise.
- `npm run dev:desktop:ui` still builds the native helper first, but that build now preserves the checked-in WAV samples.
- Browser preview leaves sound pad buttons clickable but reports that the desktop native bridge is required for audio.
- Sound pad buttons remain clickable in preview and Electron; missing native bridge reports a console warning instead of rendering disabled buttons.
- Sound pad buttons now provide hover, focus, and pressed visual states so the active target is clear before clicking.
- Sound pad panel displays playback/error status so helper failures are visible in the UI.
- Electron now loads `dist/electron/preload.cjs`; this avoids the sound-pad bridge being unavailable due to ESM preload loading differences.
- Desktop sound pad playback is now single-voice: starting a pad stops the currently playing pad first.
- Sound pad panel includes a `STOP` button wired through preload IPC to Electron main.
- Electron auto-stops active sound pad playback after 6 seconds so long source samples cannot run indefinitely.
- The native helper forwards SIGTERM/SIGINT to its child `/usr/bin/afplay` process so stopping the helper also stops audible playback.
- Long real samples were shortened into pad-friendly cues: applause is 4.5 seconds, drumroll is 4.0 seconds, both with fade-out.

Validation:
- command: `npm run build:native:sound-pad`
- exit/result: `0`; native helper built at `native/sound-pad/build/sound-pad-helper` and six existing WAV assets validated without being regenerated.
- command: `file assets/sound-pads/*.wav`
- exit/result: `0`; six persistent WAV assets parse as RIFF/WAVE audio: applause, cheer, ding, drumroll, laugh, whoosh.
- command: `afinfo assets/sound-pads/applause.wav assets/sound-pads/laugh.wav assets/sound-pads/cheer.wav assets/sound-pads/drumroll.wav assets/sound-pads/ding.wav assets/sound-pads/whoosh.wav`
- exit/result: `0`; all six assets parse as PCM WAV files with valid duration/channel/sample-rate metadata.
- command: `afinfo assets/sound-pads/applause.wav assets/sound-pads/drumroll.wav`
- exit/result: `0`; applause duration is `4.500000 sec` and drumroll duration is `4.000000 sec`.
- command: `wc -c assets/sound-pads/*.wav`
- exit/result: `0`; post-verify asset sizes confirm real downloaded/converted samples remain in place: applause `793,878`, cheer `194,876`, ding `366,660`, drumroll `705,678`, laugh `246,802`, whoosh `925,662` bytes.
- command: `native/sound-pad/build/sound-pad-helper --validate`
- exit/result: `0`; all six sound pads render generated buffers.
- command: `native/sound-pad/build/sound-pad-helper --play ding`
- exit/result: `1`; current command environment reports `AudioQueueStart failed (-66680)` from `/usr/bin/afplay`, so audible playback was not verified here.
- command: `npm run test:visual`
- exit/result: `0`; 2 Playwright visual smoke tests passed, including enabled Applause pad assertion.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, UI tests, native helper build with persistent WAV validation, UI build, and Electron main build passed.
- command: `npm run test:visual`
- exit/result: `0`; 2 Playwright visual smoke tests passed after adding the sound pad `STOP` control.
- command: `sed -n '1,60p' dist/electron/preload.cjs`
- exit/result: `0`; built preload exposes `window.localMixer.playSoundPad`.

Known limitations:
- This is a narrow native sound-pad spike, not the full C++20/JUCE mixer engine.
- Audible playback must be checked by running the Electron desktop app on a macOS session with an available default output device.
- Playback still shells out to macOS `/usr/bin/afplay`; a JUCE-owned sample player belongs in the later native engine phases.

## Phase06 — Import File And Basic Transport
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase04 IMPLEMENTED_UNVERIFIED

### Phase06 Checkpoint — Native WAV Streaming Reader And Transport Clock

Changed files:
- `native/engine/src/engine/MediaFile.hpp`, `native/engine/src/engine/MediaFile.cpp`: added a RIFF/WAV stream reader that parses metadata at open and reads requested frame windows into normalized float buffers.
- `native/engine/src/engine/Transport.hpp`, `native/engine/src/engine/Transport.cpp`: added native transport clock state for play, pause, stop, seek, and simple loop wrapping.
- `native/engine/tests/MediaTransportTest.cpp`: added fixture-backed CTest coverage for WAV metadata, read-window normalization, pause/seek/stop, and loop wrap.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired the module into native builds and Phase06 evidence.

Implemented behavior:
- WAV open does not decode the whole file; audio samples are read by frame window.
- PCM 16/24/32-bit and 32-bit float WAV sample conversion paths are present.
- Stop returns to the configured playback start frame; pause retains position; loop wraps position in sample frames.
- All file and transport work remains native C++ and does not use browser audio.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- AIFF, FLAC, MP3, and M4A import are not implemented yet.
- Import job progress/cancel, waveform pyramid, timeline scheduling, stale-buffer flush, and audible file playback are not implemented yet.
- Mixed-rate resampling and hardware playback verification remain `NOT_RUN`.

### Phase06 Checkpoint — Native Media Inspect Protocol

Changed files:
- `native/engine/src/main.cpp`: added `media-inspect` protocol command for metadata-only WAV import.
- `apps/desktop/electron/main.ts`: whitelisted `media-inspect` for desktop engine IPC.
- `scripts/test-native.mjs`: added a temporary WAV fixture and protocol smoke assertion for metadata import.
- `docs/progress.md`: recorded verification evidence.

Implemented behavior:
- Renderer/Electron can request file metadata through the native engine without receiving PCM over IPC.
- `media-inspect` returns container, path, channels, sample rate, bit depth, frame count, and duration.
- Missing/unsupported files return typed native errors instead of pretending import succeeded.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke including `media-inspect` passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Metadata import is not yet a queued import job with progress/cancel.
- PCM streaming is available in native tests but is not yet scheduled into audible timeline playback.

### Phase06 Checkpoint — Streaming Waveform Pyramid

Changed files:
- `native/engine/src/engine/WaveformPyramid.hpp`, `native/engine/src/engine/WaveformPyramid.cpp`: added native min/max waveform point generation from `WavStreamReader`.
- `native/engine/tests/MediaTransportTest.cpp`: verifies waveform points from a stereo fixture without browser audio.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired waveform module into native build and evidence.

Implemented behavior:
- Waveform generation reads bounded frame chunks from the native stream reader instead of loading entire media into renderer state.
- Stereo samples are folded to a mono display envelope for min/max point generation.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Waveform pyramid is not yet exposed as a cancellable import job.
- Timeline UI still uses fixture state; imported waveform display is not wired yet.

### Phase06 Checkpoint — Native Transport Protocol

Changed files:
- `native/engine/src/main.cpp`: added `transport-play`, `transport-pause`, `transport-stop`, `transport-seek`, and `transport-status` protocol commands backed by the native `TransportClock`.
- `apps/desktop/electron/main.ts`: whitelisted transport protocol commands.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: wired top-bar play/pause/stop buttons to native transport commands when the desktop engine bridge is available.
- `scripts/test-native.mjs`: added protocol smoke coverage for play, seek, and stop.
- `docs/progress.md`: recorded verification evidence.

Implemented behavior:
- Transport state changes are owned by the native engine protocol.
- Stop returns native transport position to playback start frame `0`.
- Browser preview does not create browser audio playback or fake native transport success.

Validation:
- command: `npm run typecheck`
- exit/result: `0`.
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke including transport passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Transport is not yet scheduling file audio into the device callback.
- Pause/resume timing is stateful but not yet tied to a rendered timeline clock in the UI.

### Phase06 Maintenance — Media Transport JSON Split

Changed files:
- `native/engine/src/engine/MediaTransportJson.hpp`, `native/engine/src/engine/MediaTransportJson.cpp`: extracted media inspect and transport JSON serialization helpers from `main.cpp`.
- `native/engine/src/main.cpp`: reduced protocol entry-point size before adding more Phase06 commands.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: added helper module evidence and build wiring.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

### Phase06 Checkpoint — Pollable Native Import Jobs

Changed files:
- `native/engine/src/engine/MediaImportJob.hpp`, `native/engine/src/engine/MediaImportJob.cpp`: added native import job manager with queued/running/completed/canceled/error states, bounded status-poll processing, and waveform point counting.
- `native/engine/src/main.cpp`: added `media-import-start`, `media-import-status`, and `media-import-cancel` protocol commands.
- `native/engine/src/engine/MediaTransportJson.cpp`: added import job status JSON without PCM payloads.
- `apps/desktop/electron/main.ts`: whitelisted import job commands for desktop IPC.
- `native/engine/tests/MediaTransportTest.cpp`, `scripts/test-native.mjs`: added CTest and protocol smoke coverage for import completion and cancellation.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired build and evidence.

Implemented behavior:
- Import jobs expose progress and cancellation through native protocol.
- Status polling processes bounded chunks from the stream reader and counts waveform points.
- Canceled jobs do not report completed media references.
- PCM remains inside native code; IPC carries metadata/progress only.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 7/7, engine self-test, device enumeration smoke, and protocol smoke including import job start/status/cancel passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Import processing is currently poll-driven rather than a separate background thread.
- Import jobs support WAV through the current native reader; AIFF/FLAC/MP3/M4A remain not implemented.
- Imported media is not yet placed on a playable timeline track.

### Phase06 Checkpoint — Local Codec Capability Probe

Changed files:
- `scripts/probe-codecs.mjs`: added a repeatable FFmpeg decoder probe using argv spawning with `shell:false`.
- `package.json`: added `npm run probe:codecs`.
- `docs/reports/codec-probe-phase06.md`: recorded current local decoder availability.
- `docs/task-plan.json`, `docs/progress.md`: added evidence paths and validation record.

Implemented behavior:
- Local development probe verifies FFmpeg availability and required decoder names for M4A/AAC, FLAC, MP3, WAV PCM, and WAV float.
- Probe output is explicit about scope: it proves this machine's FFmpeg capability, not packaged end-user bundling.

Validation:
- command: `npm run probe:codecs`
- exit/result: `0`; report shows FFmpeg `8.1.1` at `/opt/homebrew/bin/ffmpeg`, with `aac`, `flac`, `mp3`, `pcm_s16le`, and `pcm_f32le` available.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- FFmpeg decoding is not yet integrated into native import jobs.
- End-user packaged FFmpeg resource path and bundling are still a later packaging requirement.

### Phase06 Checkpoint — Desktop Media Import Surface

Changed files:
- `apps/desktop/src/features/media/components/MediaImportPanel.tsx`: added a compact media import modal with native file selection, inspect, import, poll, and cancel controls.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: Timeline tab opens the media import surface and keeps transport controls wired to native transport.
- `apps/desktop/electron/main.ts`, `apps/desktop/electron/preload.ts`, `apps/desktop/src/types/localMixer.d.ts`: added a sandbox-safe Electron file picker bridge for local audio files.
- `apps/desktop/src/styles/app.css`: added compact modal styling.
- `tests/ui/visual.visual.ts`: added Playwright coverage for the Timeline import surface.
- `docs/task-plan.json`, `docs/progress.md`: added evidence paths and verification record.

Implemented behavior:
- Desktop media selection runs through Electron main; renderer does not access filesystem directly.
- Import UI calls native `media-inspect` and pollable import job commands through the existing engine bridge.
- Browser preview can open the panel but cannot fake import success without the desktop/native bridge.
- No browser audio element, Web Audio, or PCM-over-IPC path was introduced.

Validation:
- command: `npm run typecheck`
- exit/result: `0`.
- command: `npm run test:visual`
- exit/result: `0`; 5 Playwright tests passed, including media import panel visibility.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Automated test does not click the OS file picker.
- Imported media still is not placed on a playable timeline track.

### Phase06 Checkpoint — Native Timeline Scheduler

Changed files:
- `native/engine/src/engine/Timeline.hpp`, `native/engine/src/engine/Timeline.cpp`: added a sample-frame timeline scheduler for clip start, source offset, duration, gain, and stereo output rendering.
- `native/engine/tests/TimelineTest.cpp`: verifies two impulse fixtures align at the same sample frame, source offset renders correctly, and silent areas do not leak stale samples.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired the timeline module and evidence.

Implemented behavior:
- Multiple clips are scheduled against one native timeline frame domain.
- Mono sources are mapped to stereo output, and stereo sources preserve left/right samples.
- Missing media causes render failure rather than silent fake success.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 8/8 including `local-mixer-timeline-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Scheduler currently renders in-memory fixture media, not imported file readers in the device callback.
- Loop boundary fade and resampling are still not implemented.

### Phase06 Checkpoint — Transport Buffer Invalidation

Changed files:
- `native/engine/src/engine/Transport.hpp`, `native/engine/src/engine/Transport.cpp`: added `bufferGeneration` to transport snapshots and increment it on seek/stop.
- `native/engine/src/engine/MediaTransportJson.cpp`: exposes `bufferGeneration` in transport protocol status.
- `native/engine/tests/MediaTransportTest.cpp`, `scripts/test-native.mjs`: verify generation changes on seek/stop.
- `docs/progress.md`: recorded evidence.

Implemented behavior:
- Native consumers can detect seek/stop invalidation and discard stale file buffers.
- Protocol smoke confirms generation increments deterministically.

Validation:
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Actual queued file playback buffers are not implemented yet, so invalidation is exposed before a device-callback file player consumes it.

### Phase06 Checkpoint — Timeline Clip Boundary Fades

Changed files:
- `native/engine/src/engine/Timeline.hpp`, `native/engine/src/engine/Timeline.cpp`: added per-clip fade-in and fade-out frame fields and render-time fade gain.
- `native/engine/tests/TimelineTest.cpp`: verifies boundary fade ramp samples.
- `docs/progress.md`: recorded validation evidence.

Implemented behavior:
- Timeline clips can apply deterministic sample-frame fade ramps at their edges.
- This provides the native primitive needed for click-reduced clip and loop-boundary transitions.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 8/8, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Full loop playback has not yet been connected to the device callback.
- Fade shape is currently linear; final UX may require equal-power or configurable curves.

### Phase06 Checkpoint — Mixed-Rate Resampler Foundation

Changed files:
- `native/engine/src/engine/MediaResampler.hpp`, `native/engine/src/engine/MediaResampler.cpp`: added native linear resampling helpers and target frame-count calculation.
- `native/engine/tests/MediaTransportTest.cpp`: verifies 24 kHz to 48 kHz frame count and source sample positions.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired build and evidence.

Implemented behavior:
- Native code can convert decoded PCM between source and project sample rates.
- Duration frame count is calculated from source/target rate instead of accepting mismatched rates silently.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 8/8, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Resampler is linear and intended as a correctness foundation; final import quality may require a higher-quality native resampler.
- Resampler is not yet integrated into import jobs or realtime file playback.

## Phase07 — Meter, Gain Staging, And EQ
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase06 IMPLEMENTED_UNVERIFIED

### Phase07 Checkpoint — Native Peak/RMS Meter

Changed files:
- `native/engine/src/dsp/Meter.hpp`, `native/engine/src/dsp/Meter.cpp`: added native peak/RMS dBFS metering with clip latch/reset.
- `native/engine/tests/MeterTest.cpp`: verifies `0.5` amplitude as about `-6.02 dBFS`, RMS fixture behavior, clip latch, and reset.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired the module into native tests and evidence.

Implemented behavior:
- Peak and RMS readings are represented in dBFS with silence clamped to `-120 dBFS`.
- Clip state latches until explicitly reset.
- Meter work is native C++; no renderer audio processing was introduced.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 9/9 including `local-mixer-meter-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.
- command: `npm run verify`
- exit/result: `0`; plan/file-size/architecture/typecheck/UI tests/native tests/UI build/Electron main build passed.

Known limitations:
- Meter taps are not yet wired into realtime graph points.
- UI meter decay/hold remains preview-simulated.

### Phase07 Checkpoint — Native Biquad EQ Foundation

Changed files:
- `native/engine/src/dsp/Eq.hpp`, `native/engine/src/dsp/Eq.cpp`: added native biquad coefficient generation, response measurement, and sample processing.
- `native/engine/tests/EqTest.cpp`: verifies bypass unity, peaking EQ center gain, HPF cutoff/attenuation, and finite processed output.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired EQ into native test/build evidence.

Implemented behavior:
- Native C++20 DSP supports HPF, LPF, peaking, low-shelf, and high-shelf biquad filters.
- Bypassed EQ returns unity coefficients.
- Magnitude response can be inspected without renderer DSP or browser audio paths.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 10/10 including `local-mixer-eq-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- EQ bands are not yet wired into the realtime mixer graph or right-side channel strip controls.
- Parameter smoothing/crossfade for audible EQ changes is still pending.

## Phase08 — Gate, Expander, And Compressor
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase07 IMPLEMENTED_UNVERIFIED

### Phase08 Checkpoint — Native Dynamics Foundation

Changed files:
- `native/engine/src/dsp/Dynamics.hpp`, `native/engine/src/dsp/Dynamics.cpp`: added native compressor and noise gate/expander processors.
- `native/engine/tests/DynamicsTest.cpp`: verifies compressor gain law, gain-reduction reading, bypass, linked stereo processing, silent gate stability, gate hold, and expander attenuation.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired dynamics build/test/progress evidence.

Implemented behavior:
- Compressor supports threshold, ratio, soft knee, attack, release, manual makeup, and linked stereo detector processing.
- Noise control supports gate and expander modes with threshold, hysteresis, hold, range, attack, and release.
- Phase08 acceptance fixture is covered: steady `-12 dBFS` into threshold `-24 dBFS` ratio `4:1` settles near `-21 dBFS` with makeup `0`.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 11/11 including `local-mixer-dynamics-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Dynamics processors are not yet wired into the realtime channel strip graph or native parameter API.
- Gain-reduction telemetry is local to the processor instance; UI subscriptions remain pending.

## Phase09 — Vocal Channel And De-Esser
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase08 IMPLEMENTED_UNVERIFIED

### Phase09 Checkpoint — Native De-Esser And Vocal Presets

Changed files:
- `native/engine/src/dsp/DeEsser.hpp`, `native/engine/src/dsp/DeEsser.cpp`: added native high-frequency detector de-esser with reduction cap, linked stereo processing, and separate detector audition render path.
- `native/engine/src/engine/VocalStripPreset.hpp`, `native/engine/src/engine/VocalStripPreset.cpp`: added native starting values for `Voice Clean`, `Voice Warm`, `Voice Broadcast`, and `Music Flat`.
- `native/engine/tests/DeEsserTest.cpp`, `native/engine/tests/VocalStripPresetTest.cpp`: added DSP and preset coverage.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase09 build/test/progress evidence.

Implemented behavior:
- High-frequency bursts above detector threshold are reduced with a configurable maximum reduction cap.
- Low-frequency content outside the detector path is not over-reduced in the acceptance fixture.
- Linked stereo detection applies the same de-essing gain to both channels.
- Detector audition renders to a separate monitor buffer and does not replace the master/recording output path.
- Voice preset starting values are native-owned and editable by later UI/session layers.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 13/13 including `local-mixer-de-esser-tests` and `local-mixer-vocal-strip-preset-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- De-esser is not yet wired into realtime channel strip processing or UI/native parameter commands.
- Presets are native configuration factories only; session persistence and UI selection are pending.

## Phase10 — Bus, Aux, DCA, And Monitoring
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase09 IMPLEMENTED_UNVERIFIED

### Phase10 Checkpoint — Native Routing Graph Foundation

Changed files:
- `native/engine/src/engine/RoutingGraph.hpp`, `native/engine/src/engine/RoutingGraph.cpp`: added native route nodes, route edges, topological ordering, cycle rejection, send tap semantics, DCA effective gain, and export eligibility helpers.
- `native/engine/tests/RoutingGraphTest.cpp`: verifies duplicate node rejection, valid channel/subgroup/aux/main/monitor routing, rejected cycles without graph mutation, pre/post send semantics, mute behavior, DCA effective gain, and PFL/AFL export exclusion.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase10 build/test/progress evidence.

Implemented behavior:
- Route graph edges are validated before mutation; cycles are rejected before the active graph changes.
- Pre-fader sends ignore fader and DCA movement, while post-fader/AFL sends include fader and DCA effective gain.
- Muted channels do not feed sends.
- PFL/AFL monitor routes are explicitly excluded from export eligibility.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 14/14 including `local-mixer-routing-graph-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- The foundation does not yet instantiate all fixed buses (4 subgroups, 4 aux, FX A/B returns, 4 DCA) in a session graph.
- Route graph is not yet connected to realtime mixing buffers, monitor selector UI, or save/session persistence.

## Phase11 — Reverb, Delay, And Voice Ducking
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase10 IMPLEMENTED_UNVERIFIED

### Phase11 Checkpoint — Native FX Processor Foundation

Changed files:
- `native/engine/src/dsp/Fx.hpp`, `native/engine/src/dsp/Fx.cpp`: added native wet-only delay, simple wet reverb, and voice ducking processors.
- `native/engine/tests/FxTest.cpp`: verifies decreasing delay repeats, feedback capping below unity, wet reverb tail generation, and voice-ducking gain reduction/recovery.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase11 build/test/progress evidence.

Implemented behavior:
- Delay send-return processing outputs wet signal only and clamps feedback below unity.
- Reverb processor produces a native wet tail from an impulse.
- Voice ducking uses a detector signal to attenuate a target bus with attack/hold/release behavior.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 15/15 including `local-mixer-fx-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- FX processors are not yet wired into FX A/B buses, realtime routing, UI controls, panic mute, or tail policy during transport pause.
- Reverb is a lightweight native foundation, not final quality/program bank DSP.

## Phase12 — Timeline Editing And Transport
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase11 IMPLEMENTED_UNVERIFIED

### Phase12 Checkpoint — Native Timeline Edit And Metronome Foundation

Changed files:
- `native/engine/src/engine/TimelineEdit.hpp`, `native/engine/src/engine/TimelineEdit.cpp`: added non-destructive clip edit model with add, move, trim, split, join, undo, redo, and snap helpers.
- `native/engine/src/engine/Metronome.hpp`, `native/engine/src/engine/Metronome.cpp`: added tempo-based click scheduling and explicit export eligibility.
- `native/engine/tests/TimelineEditTest.cpp`: verifies split/join sample-position consistency, undo/redo snapshots, overlap retention, snap behavior, stable 120 BPM click frames, and monitor-only metronome default.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase12 build/test/progress evidence.

Implemented behavior:
- Split and join preserve media reference, source offset, timeline position, and duration semantics without processing the audio.
- Undo/redo restores edit snapshots.
- Overlapping clips are retained instead of silently replacing an existing clip.
- Metronome clicks are scheduled by sample frame; export contribution is disabled unless print-click is explicit.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 16/16 including `local-mixer-timeline-edit-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Edit commands are not yet wired to desktop UI shortcuts/drag operations or session persistence.
- Timeline playback still uses earlier scheduler primitives; edit-during-playback concurrency has not been integrated into realtime graph publication.

## Phase13 — Master And Multitrack Recording
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase12 IMPLEMENTED_UNVERIFIED

### Phase13 Checkpoint — Native Recording Writer Foundation

Changed files:
- `native/engine/src/engine/Recording.hpp`, `native/engine/src/engine/Recording.cpp`: added native float32 WAV writer, collision-safe take naming, recording tap enum, and recording session state machine.
- `native/engine/tests/RecordingTest.cpp`: verifies collision-safe filenames, arm/start/write/stop transitions, WAV metadata round-trip through the native reader, clamped float samples, and overrun status preserving valid recorded data.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase13 build/test/progress evidence.

Implemented behavior:
- Recording output is written as native WAV float32 with finalized RIFF/data sizes.
- Take filenames avoid overwriting existing files.
- Session state distinguishes saved, failed, and overrun outcomes.
- Overrun stop finalizes valid data and keeps `overrun` status instead of reporting success.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 17/17 including `local-mixer-recording-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Realtime writer thread, bounded ring buffer, free-space monitoring, RF64/segmentation, and desktop recording controls are not yet implemented.
- Dry/processed/master tap routing exists as metadata only; callback tap capture is pending.

## Phase14 — Export, Loudness, Limiter, And Latency Compensation
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase13 IMPLEMENTED_UNVERIFIED

### Phase14 Checkpoint — Native Offline Export Foundation

Changed files:
- `native/engine/src/engine/Export.hpp`, `native/engine/src/engine/Export.cpp`: added native offline timeline-to-WAV export job, live-source preflight rejection, explicit tail frames, and cancel-to-partial output.
- `native/engine/tests/ExportTest.cpp`: verifies timeline sample export, explicit tail length, live-source rejection before rendering, and canceled `.partial` output.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase14 build/test/progress evidence.

Implemented behavior:
- Offline export renders the native timeline scheduler to float32 WAV without opening physical audio devices.
- Live sources are reported and rejected before an offline export starts.
- Canceled export jobs write a `.partial` file and do not report success.
- Tail length is explicit in frames for deterministic testing.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 18/18 including `local-mixer-export-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Offline export is not yet using the full realtime DSP graph, automation, pan law, plugin latency graph, stems, normalization, LUFS, or true-peak validation.
- FLAC/MP3 export, TPDF dither for integer output, and UI export jobs are pending.

## Phase15 — Session, Preset, Autosave, And Recovery
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase14 IMPLEMENTED_UNVERIFIED

### Phase15 Checkpoint — Native Session Document Foundation

Changed files:
- `native/engine/src/engine/SessionDocument.hpp`, `native/engine/src/engine/SessionDocument.cpp`: added native session JSON document serialization/parsing, schema version validation, atomic save temp+rename, previous-good backup, autosave path, and recovery candidate lookup.
- `native/engine/tests/SessionDocumentTest.cpp`: verifies save/open round-trip, newer schema rejection, corrupt session rejection, previous-good backup survival, and autosave recovery candidate behavior.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase15 build/test/progress evidence.

Implemented behavior:
- Saved session path and autosave recovery path are represented separately.
- Sessions with schema versions newer than the engine are rejected.
- Atomic save writes a temp file and preserves a previous-good backup on subsequent saves.
- Autosave recovery can be offered without overwriting the original session.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 19/19 including `local-mixer-session-document-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Full routing/FX/automation/session schema, migrations, relink UI, portable media collection, debounce timers, and preset library persistence are not yet implemented.
- JSON parser currently supports the engine-owned session shape only; external arbitrary JSON compatibility is not claimed.

## Phase16 — Automation And MIDI
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase15 IMPLEMENTED_UNVERIFIED

### Phase16 Checkpoint — Native Automation Lane And MIDI Mapping Foundation

Changed files:
- `native/engine/src/engine/Automation.hpp`, `native/engine/src/engine/Automation.cpp`: added native automation lanes, continuous interpolation, discrete event lookup, gesture undo snapshot, MIDI CC mapping, and soft-takeover helper.
- `native/engine/tests/AutomationTest.cpp`: verifies value-at-seek, interpolation, discrete mute timing, one-gesture undo, MIDI mapping retention across disconnect, injected CC mapping, mismatched channel ignore, and soft-takeover pickup behavior.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Phase16 build/test/progress evidence.

Implemented behavior:
- Continuous automation interpolates by engine sample frame.
- Discrete automation changes exactly at keyed frames for mute-style parameters.
- Undo restores a single edited gesture as one action.
- MIDI CC mappings are data-owned independently of current device connection status.
- Soft takeover prevents fader jumps until the incoming control reaches the current value tolerance.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 20/20 including `local-mixer-automation-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Native MIDI device enumeration/input callbacks, timestamped realtime scheduling, write/touch/latch mode engine integration, mapping persistence in session JSON, and UI MIDI learn are not yet implemented.
- Offline/realtime automation parity is covered only at value lookup level, not full graph rendering.

## VFX-00 — Audit Gap, Catalog, Schema, And Design Contract
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase16 IMPLEMENTED_UNVERIFIED

### VFX-00 Checkpoint — Native Vocal FX Catalog Contract

Changed files:
- `native/engine/src/dsp/fx/EffectRegistry.hpp`, `native/engine/src/dsp/fx/EffectRegistry.cpp`: added the native Vocal FX catalog contract with 12 stable effect IDs, parameter descriptors, owner modules, implementation phases, format policies, availability flags, and stable FX error names.
- `native/engine/tests/fx/EffectRegistryTest.cpp`: verifies all required effect IDs are present, each descriptor has owner/schema/test metadata, Phase11-backed reverb/delay are only `implemented_unverified`, pitch correction remains unavailable, and developer/error names are stable.
- `docs/specs/vocal-fx.md`: split out the VFX contract, rack policy, format policy, availability semantics, and IPC error contract.
- `docs/adr/0002-pitch-backend.md`: records pitch backend boundary and Rubber Band spike status without claiming integration.
- `docs/reports/vocal-fx-catalog-vfx00.md`: records the VFX-00 catalog report and current availability summary.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-00 build/test/progress evidence.

Implemented behavior:
- The native registry exposes all 12 required Vocal FX effect IDs from section 8A.2.
- Each effect has a native-owned parameter schema, owner module, implementation phase, and test plan.
- Availability status is explicit and conservative; unavailable effects cannot be treated as implemented DSP.
- Mono/stereo format policy and IPC/rack error names are defined for later rack runtime phases.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 21/21 including `local-mixer-effect-registry-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- VFX-00 is catalog/contract work only; it does not complete rack runtime or the remaining Vocal FX DSP.
- Rubber Band/pitch backend build, license, latency, pitch detector, and audio quality gates remain pending in VFX-04 through VFX-06.

## VFX-01 — Rack Runtime, Dry Alignment, Bypass, And Editor Skeleton
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-00 IMPLEMENTED_UNVERIFIED

### VFX-01 Checkpoint — Native Effect Rack Runtime Foundation

Changed files:
- `native/engine/src/dsp/fx/EffectProcessor.hpp`: added the native effect processor runtime interface with prepare/reset/process/realtime-parameter/latency/tail methods.
- `native/engine/src/dsp/fx/EffectRack.hpp`, `native/engine/src/dsp/fx/EffectRack.cpp`: added bounded rack slots, revision-guarded add/remove/move/bypass/replace, all-or-nothing factory rollback, A/B compare store/recall, serial processing, linear wet/dry mix, slot limit, and mono/stereo compatibility checks.
- `native/engine/tests/fx/EffectRackTest.cpp`: verifies 50% linear mix, bypass behavior, rapid bypass changes, injected latency reporting, factory failure rollback, move/reorder, A/B recall, stale revision rejection, slot limit, and incompatible format rejection before publish.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-01 build/test/progress evidence.

Implemented behavior:
- Rack actions produce explicit `FxError` outcomes and applied revisions.
- Factory failure preserves the previous active rack.
- Native slot count is bounded.
- Test gain/latency processors validate plumbing but are not registered in the production Vocal FX catalog.
- Mono-only processors cannot be moved after a stereo-widening slot unless a later phase adds tested negotiation support.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 22/22 including `local-mixer-effect-rack-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Full latency-compensated dry alignment/history, graph publication/retirement from the realtime mixer graph, IPC commands, and UI editor skeleton are not yet connected.
- Bypass crossfade is represented by state changes only; click-free ramp verification is pending with real processors.

## VFX-02 — Reverb, Delay, Chorus, Flanger, And Phaser
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-01 IMPLEMENTED_UNVERIFIED

### VFX-02 Checkpoint — Native Time And Modulation FX Processors

Changed files:
- `native/engine/src/dsp/fx/DelayEffect.hpp`, `native/engine/src/dsp/fx/DelayEffect.cpp`: adapted the native wet-only delay processor to the VFX `EffectProcessor` interface.
- `native/engine/src/dsp/fx/ReverbEffect.hpp`, `native/engine/src/dsp/fx/ReverbEffect.cpp`: adapted native wet reverb to the VFX interface with tail reporting.
- `native/engine/src/dsp/fx/ModulationEffects.hpp`, `native/engine/src/dsp/fx/ModulationEffects.cpp`: added native chorus, flanger, and phaser processors.
- `native/engine/tests/fx/TimeModulationEffectsTest.cpp`: verifies delay wet-only repeat spacing and feedback, reverb wet tail and tail report, chorus finite modulated output, flanger bounded feedback behavior, and phaser altered finite output.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated reverb/delay/chorus/flanger/phaser to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-02 build/test/progress evidence.

Implemented behavior:
- Reverb and delay no longer need to be treated as only Phase11 primitives; they now have VFX runtime wrappers.
- Chorus, flanger, and phaser are native processors with bounded state and no renderer/browser DSP.
- Delay and reverb output wet signal for rack/send-return use; dry/mix remains owned by the rack layer.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 23/23 including `local-mixer-time-modulation-effects-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Room/Plate/Hall character differences, ping-pong/stereo delay modes, tempo-sync note mapping, damping filters, and click-free automation ramps remain incomplete.
- Processors are not yet connected to production UI controls/presets, offline export equivalence tests, or realtime rack publication.

## VFX-03 — Saturation, Telephone/Megaphone, And Doubler
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-02 IMPLEMENTED_UNVERIFIED

### VFX-03 Checkpoint — Native Character FX Foundation

Changed files:
- `native/engine/src/dsp/fx/CharacterEffects.hpp`, `native/engine/src/dsp/fx/CharacterEffects.cpp`: added native saturation and vocal doubler processors using the VFX `EffectProcessor` interface.
- `native/engine/tests/fx/CharacterEffectsTest.cpp`: verifies saturation output bounds, DC suppression, drive changing waveform shape, doubler wet-only output without direct dry, and two independent bounded micro-delay voices.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated `doubler` and `saturation` to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-03 build/test/progress evidence.

Implemented behavior:
- Saturation uses native soft waveshaping, output trim, clipping bounds, and a DC blocker.
- Doubler produces wet micro-delay voices with independent delays and pan placement.
- Doubler output does not include the dry vocal; dry is counted once by the rack mix layer.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 24/24 including `local-mixer-character-effects-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Telephone/megaphone preset band-limiting, oversampling modes, aliasing comparison, detune cents, slow modulation, and mono fold-down tests remain pending.
- Processors are not yet connected to production UI controls/presets, offline export equivalence tests, or realtime rack publication.

## VFX-04 — Pitch And Formant Backend
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-03 IMPLEMENTED_UNVERIFIED

### VFX-04 Checkpoint — Native Rubber Band Pitch Backend

Changed files:
- `native/engine/src/dsp/fx/PitchBackend.hpp`, `native/engine/src/dsp/fx/PitchBackend.cpp`: added a native Rubber Band LiveShifter backend boundary with prepare/reset, pitch ratio, formant ratio, latency, block-size, and deinterleaved mono/stereo processing support.
- `native/engine/tests/fx/PitchBackendTest.cpp`: verifies semitone ratio conversion and measured octave up/down pitch shifts on generated native sine input.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated `pitch_shift` and `formant_shift` to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired Rubber Band via pkg-config and added VFX-04 build/test/progress evidence.

Implemented behavior:
- Pitch/formant processing remains native C++ and does not use Web Audio, renderer media capture/playback, or browser DSP.
- The backend reports Rubber Band block size and startup latency for later rack dry-alignment integration.
- Pitch shift evidence measures 220 Hz shifted to 440 Hz and 110 Hz within a 10-cent tolerance.
- Formant control is exposed through Rubber Band `setFormantScale`; detailed perceptual formant QA remains later work.

Validation:
- command: `pkg-config --modversion rubberband`
- exit/result: `0`; installed native Rubber Band version `4.0.0` found.
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 25/25 including `local-mixer-pitch-backend-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Pitch/formant processors are not yet wrapped as production `EffectProcessor` instances in the rack.
- Latency compensation is reported but not yet integrated with rack dry alignment or the realtime mixer graph.
- Pitch correction, harmony generation, and vocal formant listening QA remain pending in VFX-05 through VFX-09.

## VFX-05 — Monophonic Pitch Correction
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-04 IMPLEMENTED_UNVERIFIED

### VFX-05 Checkpoint — Detector, Scale Mapper, And Correction Processor

Changed files:
- `native/engine/src/dsp/fx/PitchDetector.hpp`, `native/engine/src/dsp/fx/PitchDetector.cpp`: added a native monophonic YIN-style pitch detector, voiced confidence output, MIDI/frequency helpers, and chromatic/major/natural-minor scale target mapping.
- `native/engine/src/dsp/fx/PitchCorrectionEffect.hpp`, `native/engine/src/dsp/fx/PitchCorrectionEffect.cpp`: added a native pitch-correction `EffectProcessor` wrapper using `PitchBackend`, rolling analysis windows, confidence gate, amount/tolerance controls, retune smoothing, and reported analysis/backend latency.
- `native/engine/tests/fx/PitchCorrectionTest.cpp`: verifies detector pitch/silence behavior, scale mapper correction amount, and a full correction pass for a sharp A4 test tone settling within 10 cents of target.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/specs/vocal-fx.md`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated `pitch_correct` to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-05 build/test/progress evidence.

Implemented behavior:
- Pitch detection, note mapping, and correction run in native C++ only; no browser capture, Web Audio, renderer DSP, ML service, or network dependency is used.
- Low-confidence or unvoiced input ramps correction back toward unity instead of locking to a random note.
- Amount `0` is represented as unity correction through the mapper; full amount on a steady detuned note corrects toward the nearest allowed target.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 26/26 including `local-mixer-pitch-correction-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Correction is implemented as a foundation processor and is not yet connected to production rack IPC/UI controls.
- Variable callback FIFO adaptation, detector timestamp alignment against delayed audio, vibrato preservation, octave-transition robustness, fricative fixtures, and real vocal audition are still pending for later Vocal FX QA/integration.
- The current detector favors correctness in tests over optimized realtime complexity; performance profiling remains pending.

## VFX-06 — Harmony Two Voices
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-05 IMPLEMENTED_UNVERIFIED

### VFX-06 Checkpoint — Native Harmony Voice Foundation

Changed files:
- `native/engine/src/dsp/fx/HarmonyEffect.hpp`, `native/engine/src/dsp/fx/HarmonyEffect.cpp`: added a two-voice native harmony processor using the common pitch detector, Rubber Band pitch backends, fixed/diatonic interval mapping, independent voice level/pan, formant preserve config, and low-confidence voicing gate.
- `native/engine/tests/fx/HarmonyEffectTest.cpp`: verifies C-major/natural-minor diatonic interval mapping, pan helpers, silence gating, and panned stereo output from a generated sustained vocal-like fixture.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/specs/vocal-fx.md`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated `harmony` to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-06 build/test/progress evidence.

Implemented behavior:
- Harmony uses one detector pass and two independent native pitch-shifter voices; voice 2 is not cascaded from voice 1.
- Diatonic intervals follow scale degree semantics: in C major, C +2 maps to E and +4 maps to G.
- Low-confidence/silent input does not create a false harmony target, and the dry input is preserved rather than muted.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 27/27 including `local-mixer-harmony-effect-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Harmony is not yet connected to production rack IPC/UI controls, preset rows, or monitor profile policy.
- Dry/voice latency alignment is reported at processor level but not yet reconciled in the realtime mixer graph.
- Real vocal audition, consonant behavior, CPU/xrun profiling with two voices, and final HARM QA remain pending.

## VFX-07 — Vocoder And Robot Voice
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-06 IMPLEMENTED_UNVERIFIED

### VFX-07 Checkpoint — Native Vocoder Foundation

Changed files:
- `native/engine/src/dsp/fx/VocoderEffect.hpp`, `native/engine/src/dsp/fx/VocoderEffect.cpp`: added a native 16-band vocoder/robot processor with log-spaced analysis/synthesis bands, envelope followers, internal fixed carrier, MIDI carrier mode, maximum 4 active notes, all-notes-off, panic, unvoiced/noise mix, output trim, and bounded tail reporting.
- `native/engine/tests/fx/VocoderEffectTest.cpp`: verifies modulator silence stays silent, fixed carrier note changes output pitch/color, MIDI notes produce output, all-notes-off prevents stuck carrier output, panic clears output, and rendered samples remain finite.
- `native/engine/src/dsp/fx/EffectRegistry.cpp`, `docs/specs/vocal-fx.md`, `docs/reports/vocal-fx-catalog-vfx00.md`: updated `vocoder` to `implemented_unverified`.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired VFX-07 build/test/progress evidence.

Implemented behavior:
- Vocoder processing is native C++ only and uses no Web Audio, renderer DSP, browser synthesizer, or network service.
- Robot preset foundation is available through internal fixed carrier note behavior; MIDI carrier mode supports up to four active notes.
- Panic and all-notes-off are explicit native controls for stuck-note prevention.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 28/28 including `local-mixer-vocoder-effect-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- Vocoder is not yet connected to production rack IPC/UI controls, preset browser, or MIDI device event plumbing.
- Anti-aliasing quality, articulation listening QA on real vocal recordings, spectral-band fixture detail, CPU profiling, and final VFX-09 quality reports remain pending.

## VFX-08 — Vocal FX UI, Presets, And Rack Hooks
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-07 IMPLEMENTED_UNVERIFIED

### VFX-08 Checkpoint — Desktop Vocal FX Rack Editor Surface

Changed files:
- `apps/desktop/src/features/vocal-fx/components/VocalFxPanel.tsx`: added a compact desktop modal for Vocal FX with effect library categories, rack slots, selected effect editor, bypass/on toggle, preset selector, latency summary, and Studio FX profile display.
- `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`, `apps/desktop/src/fixtures/approvedMixerSession.ts`: added preview/native-facing Vocal FX rack state, 12 starting-point presets, rack slot selection, slot enable/bypass, and preset application state.
- `apps/desktop/src/features/mixer/MixerPage.tsx`, `apps/desktop/src/styles/app.css`: wired the top-bar Vocal FX button to the modal and added compact tool-style layout CSS.
- `tests/ui/preview-adapter.test.ts`, `tests/ui/visual.visual.ts`: added unit and visual coverage for rack state mutations and opening/selecting the Vocal FX editor.

Implemented behavior:
- The UI now exposes all required Vocal FX families in a rack/editor surface instead of only the quick harmony strip.
- Preset names match the required starting points: Clean Voice, Warm Broadcast, Studio Pop, Karaoke Hall, Slapback, Wide Double, Low Character, Bright Character, Hard Tune, Harmony Duo, Telephone, and Robot.
- State changes are adapter-owned and do not use Web Audio, browser media APIs, renderer PCM, or fake browser audio processing.

Validation:
- command: `npm run typecheck`
- exit/result: `0`; TypeScript passed.
- command: `npm run test:ui`
- exit/result: `0`; Vitest 17/17 passed including Vocal FX rack adapter state.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 17/17, native CTest 28/28, engine smoke checks, and UI build passed.
- command: `npm run test:visual`
- exit/result: `0`; Playwright 6/6 passed including Vocal FX rack modal surface and refreshed mixer screenshots.

Known limitations:
- VFX-08 is UI/adapter foundation only; production native IPC commands for rack slot add/remove/reorder, parameter automation, A/B snapshots, and engine CPU/latency telemetry are still pending integration phases.
- Visual Playwright evidence is refreshed, but full native rack IPC, persistence, and automation acceptance remain pending.

## MIXFX-00 — Registry 99 Program And Contracts
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: VFX-08 IMPLEMENTED_UNVERIFIED

### MIXFX-00 Checkpoint — Native Factory FX Bank Contract

Changed files:
- `native/engine/src/engine/FxProgramRegistry.hpp`, `native/engine/src/engine/FxProgramRegistry.cpp`: added native factory bank version 1 with programs 01-99 from spec section 8B.5, family metadata, macro labels/units/values, and expanded wet-only recipe parameters.
- `native/engine/tests/FxProgramRegistryTest.cpp`: verifies exactly 99 unique contiguous IDs, non-empty names, stable family labels, wet-only/output-trim expanded recipes, and key contracts for 12 Vocal Plate, 50 Stereo 320, and 99 Infinite Mood.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired MIXFX-00 build/test/progress evidence.

Implemented behavior:
- The native engine now owns a checked-in deterministic 99-program registry instead of relying only on React fixtures.
- Program recipes encode the required family constants: room/plate/hall damping and width, delay filters, chorus/phaser constants, and bounded Delay + Plate branch gains.
- `Infinite Mood` remains bounded at 6-second plate decay and 20% delay feedback; it is not an infinite/freeze effect.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 29/29 including `local-mixer-fx-program-registry-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- MIXFX-00 is contract/registry work only; no dedicated FX A/B wet-only bus processing is implemented in this checkpoint.
- Program switching crossfade, ACK state, session persistence, automation/MIDI mapping, and return stem export remain pending in MIXFX-01 through MIXFX-05.

## MIXFX-01 — Two FX Buses And Per-Channel Sends
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: MIXFX-00 IMPLEMENTED_UNVERIFIED

### MIXFX-01 Checkpoint — Native Wet-Only Send/Return Bus Foundation

Changed files:
- `native/engine/src/engine/FxSendReturnBus.hpp`, `native/engine/src/engine/FxSendReturnBus.cpp`: added a native two-bus FX send/return mixer with per-channel send A/B state, independent unit enable/mute/return gain, preallocated send/wet buffers, post-fader send gain, wet-only return summing, and independent input/return meters.
- `native/engine/tests/FxSendReturnBusTest.cpp`: verifies VOICE send A does not affect B, MUSIC send OFF does not feed return, unit OFF produces wet silence without dry bypass/leak, FX B remains independent from FX A state, and muted channels feed neither main nor sends.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired MIXFX-01 build/test/progress evidence.

Implemented behavior:
- FX A and FX B are represented as independent native wet-only buses.
- Unit OFF does not pass dry signal through the return path; dry channel contribution remains owned by the main mix.
- Channel mute cuts both main and FX send contribution for that channel.

Validation:
- command: `npm run test:native`
- exit/result: `0`; native build, CTest 30/30 including `local-mixer-fx-send-return-bus-tests`, engine self-test, device enumeration smoke, and protocol smoke passed.

Known limitations:
- The bus module is not yet wired into `MixerGraph` realtime processing, Electron IPC, compact FX rows, or the offline exporter.
- Feedback-edge rejection through the route editor and return stem export remain pending in MIXFX-02 through MIXFX-05.

## MIXFX-02 — Compact Mixer FX UI Native Binding
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: MIXFX-01 IMPLEMENTED_UNVERIFIED

### MIXFX-02 Checkpoint — FX Program Bank And Insert Editor Binding

Changed files:
- `native/engine/src/engine/FxProgramRegistryJson.hpp`, `native/engine/src/engine/FxProgramRegistryJson.cpp`: added native JSON serialization for the factory FX bank, including 99 program IDs, names, families, processor types, and macro display values.
- `native/engine/src/main.cpp`, `apps/desktop/electron/main.ts`, `apps/desktop/electron/EngineProtocol.ts`: exposed the `fx-program-bank` stdio IPC command and raised bounded protocol payload size to 64 KB for metadata responses only.
- `apps/desktop/src/features/fx/components/CompactFxRow.tsx`, `apps/desktop/src/features/mixer/MixerPage.tsx`, `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`, `apps/desktop/src/styles/app.css`: wired compact FX rows to load the native bank when the engine is running, edit program macros, keep returns/sends in shared mixer state, and open Pitch Correction from channel INSERT FX.
- `native/engine/tests/FxProgramRegistryJsonTest.cpp`, `tests/ui/preview-adapter.test.ts`, `native/engine/CMakeLists.txt`: added test coverage for native bank JSON and adapter bank/macro state.

Implemented behavior:
- From the compact Mixer FX rows, the app can request the native 99-program bank and keep the existing FX A/B selections when IDs remain valid.
- Program 12 Vocal Plate and program 50 Stereo 320 remain selectable through the existing search/numeric program field.
- FX macro edits mark the selected FX unit modified, return faders remain live, and channel SEND A/B state stays owned by the same adapter snapshot as the right processing panel.
- Clicking a channel INSERT FX button enables that processor, selects the channel, and opens the Vocal FX modal on Pitch Correction instead of creating a separate duplicate editor state.

Validation:
- command: `npm run test:ui -- preview-adapter`
- exit/result: `0`; Vitest preview adapter 9/9 passed including native bank load and macro modified state.
- command: `cmake --build native/engine/build --target local-mixer-fx-program-registry-json-tests`
- exit/result: `0`; native serializer test target built successfully.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 31/31, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.
- command: `node -e "... fx-program-bank ..."`
- exit/result: `0`; stdio command returned `{type:"fx-program-bank", ok:true, count:99, first:"Tiny Booth", p12:"Vocal Plate", last:"Infinite Mood"}`.

Known limitations:
- MIXFX-02 binds compact UI metadata and shared state only; native ACK/crossfade transitions are intentionally deferred to MIXFX-03.
- The send/return bus is still not integrated into `MixerGraph` realtime processing or export stems.
- Keyboard and 1280x800 acceptance remain covered by existing UI gate/visual tests; no new full desktop screenshot was recorded in this checkpoint.

## MIXFX-03 — Switching, Macro Overrides, And Preset Recall
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: MIXFX-02 IMPLEMENTED_UNVERIFIED

### MIXFX-03 Checkpoint — Program ACK And Coalesced Transition Contract

Changed files:
- `native/engine/src/engine/FxProgramController.hpp`, `native/engine/src/engine/FxProgramController.cpp`: added native FX A/B program transition state with exact revision validation, one pending desired program per unit, modified/reset macro state, bounded crossfade frame accounting, and old-program retention on failed prepare.
- `native/engine/src/main.cpp`, `apps/desktop/electron/main.ts`: exposed `fx-unit-select-program`, `fx-unit-set-macro`, `fx-unit-reset-macros`, and `fx-unit-snapshot` over the existing JSONL IPC whitelist.
- `apps/desktop/src/features/mixer/MixerPage.tsx`, `apps/desktop/src/features/fx/components/CompactFxRow.tsx`, `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`, `apps/desktop/src/fixtures/approvedMixerSession.ts`, `apps/desktop/src/styles/app.css`: added FX unit revision/pending/error metadata, ACK-based program selection when the native engine is running, coalesced latest-desired UI sends while one request is in flight, and compact Pending/Error row badges.
- `native/engine/tests/FxProgramControllerTest.cpp`, `native/engine/CMakeLists.txt`: added native coverage for 100 rapid program requests coalescing to one pending request, exact revision conflict rejection, bounded transition countdown, macro modified/reset, invalid program rejection, and failed prepare retaining the old program.

Implemented behavior:
- Program changes preserve FX sends, return level, enable state, and route state because the controller only owns program/revision/modified transition metadata.
- Native ACKs include actual unit, requested program, active program, revision, modified state, pending state, and crossfade frames.
- Stale UI revisions are rejected with `REVISION_CONFLICT`; failed prepare reports `PREPARE_FAILED` and leaves the previous program active.
- UI does not pretend a native program switch has applied while one is in flight; rapid changes coalesce to the latest desired program and reconcile after ACK.

Validation:
- command: `native/engine/build/native/engine/local-mixer-fx-program-controller-tests`
- exit/result: `0`; controller unit test passed.
- command: `node -e "... fx-unit-select-program ..."`
- exit/result: `0`; first select returned program 22 revision 1 with 960 crossfade frames, stale revision 0 follow-up returned `REVISION_CONFLICT` with active program 22 revision 1.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 32/32, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- MIXFX-03 implements transition/ACK contracts and bounded crossfade state, but the controller is not yet wired to actual FX DSP graph replacement inside realtime `MixerGraph`.
- Undo/redo and Custom preset migration are not fully persisted until MIXFX-04 save/open integration.
- Auditory click testing with real FX program swaps remains pending final mixer FX QA.

## MIXFX-04 — Save/Open, Automation/MIDI, And Export Contracts
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: MIXFX-03 IMPLEMENTED_UNVERIFIED

### MIXFX-04 Checkpoint — FX A/B Persistence And Export Stem Planning

Changed files:
- `native/engine/src/engine/SessionDocument.hpp`, `native/engine/src/engine/SessionDocument.cpp`: added persisted FX unit snapshots and send assignments, including unit ID, active program ID, processor type, revision, enabled/modified state, return dB, macro display values, channel ID, send unit ID, send enable, and send gain.
- `native/engine/tests/SessionDocumentTest.cpp`: verifies FX unit/send roundtrip persistence and backward-compatible loading of legacy sessions without FX state.
- `native/engine/src/engine/Automation.hpp`, `native/engine/src/engine/Automation.cpp`: added stable automation parameter IDs for FX sends, FX returns, and FX macros, plus MIDI mapping target IDs that survive device disconnect.
- `native/engine/tests/AutomationTest.cpp`: verifies stable FX automation IDs and a disconnected FX return MIDI CC mapping.
- `native/engine/src/engine/Export.hpp`, `native/engine/src/engine/Export.cpp`: added an export stem plan contract for master, FX A return, FX B return, and explicit monitor-volume exclusion.
- `native/engine/tests/ExportTest.cpp`: verifies FX A/B returns are planned as separate stems and monitor volume is not printed/exported.

Implemented behavior:
- Session save/open can now preserve the actual FX unit snapshot instead of recomputing state from the current factory catalog.
- Legacy schema-1 sessions without FX blocks remain loadable with empty FX vectors.
- Automation/MIDI IDs are explicit and stable for `fx.send.*`, `fx.return.*`, and `fx.macro.*` targets.
- Export planning represents FX A and FX B returns as separate stems and excludes monitor volume from offline export.

Validation:
- command: `native/engine/build/native/engine/local-mixer-session-document-tests`
- exit/result: `0`; session document tests passed with FX roundtrip and legacy load coverage.
- command: `native/engine/build/native/engine/local-mixer-automation-tests`
- exit/result: `0`; automation/MIDI tests passed with FX parameter IDs.
- command: `native/engine/build/native/engine/local-mixer-export-tests`
- exit/result: `0`; export tests passed with FX return stem planning.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 32/32, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- MIXFX-04 is persistence/automation/export contract foundation; actual realtime/offline FX return audio rendering remains pending integration with `MixerGraph` and final mixer FX QA.
- Session JSON parsing is intentionally minimal for current internal schema fixtures; hardened migration tooling and richer Custom preset migration are still pending.
- Full deterministic reopen-mix tolerance testing cannot pass until FX DSP graph rendering is wired end to end.

## HARM-00 — Audit Instance Binding And State
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: MIXFX-04 IMPLEMENTED_UNVERIFIED

### HARM-00 Checkpoint — Primary Harmony Binding Contract

Changed files:
- `native/engine/src/engine/ChannelHarmonyController.hpp`, `native/engine/src/engine/ChannelHarmonyController.cpp`: added primary harmony controller with content role, single primary instance ownership, desired/effective enabled state, revision ACKs, default quick parameters, 8-slot rack limit handling, multiple existing harmony candidate rejection, and unsupported-role rejection.
- `native/engine/tests/ChannelHarmonyControllerTest.cpp`: verifies first ON creates/binds one primary instance for vocal, OFF keeps the instance while disabling voices, stale revisions reject, non-vocal channels reject, full rack rejects, multiple candidates require explicit primary choice, and configure updates the same state.
- `native/engine/src/engine/SessionDocument.hpp`, `native/engine/src/engine/SessionDocument.cpp`, `native/engine/tests/SessionDocumentTest.cpp`: added persisted per-channel harmony role, primary instance ID, enabled state, key/scale/mode/voice intervals, and harmony level; legacy sessions without harmony state still load with empty vectors.
- `native/engine/src/main.cpp`, `apps/desktop/electron/main.ts`: exposed `channel-harmony-set-enabled`, `channel-harmony-configure`, and `channel-harmony-snapshot` over the native JSONL IPC whitelist.

Implemented behavior:
- Harmony shortcut state is now represented as a native primary channel instance contract, separate from shared FX A/B programs and sends.
- A vocal channel can bind one primary harmony instance with defaults `C`, `Major`, `Diatonic`, `+3rd`, `+5th`, `0.0 dB`.
- MUSIC/non-vocal roles reject shortcut enable with `ROLE_UNSUPPORTED`; full racks reject with `FX_RACK_FULL`; multiple existing harmony instances reject with `MULTIPLE_HARMONY_CANDIDATES`.
- OFF disables harmony voices while retaining the primary instance identity for fast re-enable.

Validation:
- command: `native/engine/build/native/engine/local-mixer-channel-harmony-controller-tests`
- exit/result: `0`; controller binding and error-policy tests passed.
- command: `native/engine/build/native/engine/local-mixer-session-document-tests`
- exit/result: `0`; session roundtrip passed with channel harmony state.
- command: `node -e "... channel-harmony-set-enabled ..."`
- exit/result: `0`; VOICE enable returned primary instance `harmony:voice:primary`, revision 1, key C; MUSIC enable returned `ROLE_UNSUPPORTED`.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 33/33, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- HARM-00 is the native binding/state contract; Mixer UI reconciliation with these native ACKs, monitor-profile effective status, and advanced primary-selection UX remain pending HARM-01/HARM-02.
- The primary harmony controller is not yet wired into realtime channel insert graph transitions.
- Real microphone audition and latency-alignment evidence remain pending final harmony QA.

## HARM-01 — Strip Button And Quick Panel Binding
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: HARM-00 IMPLEMENTED_UNVERIFIED

### HARM-01 Checkpoint — Mixer Harmony ACK UI

Changed files:
- `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`, `apps/desktop/src/fixtures/approvedMixerSession.ts`: added Harmony revision, pending, error, effective state, primary instance ID, and mode fields; preview default now follows spec with Harmony OFF and C Major visible.
- `apps/desktop/src/features/mixer/components/ChannelStrip.tsx`, `apps/desktop/src/features/mixer/components/ChannelBank.tsx`: wired the Harmony gear to open the quick tray without toggling ON/OFF, preserving keyboard-accessible button behavior.
- `apps/desktop/src/features/harmony/components/HarmonyQuickPanel.tsx`, `apps/desktop/src/styles/app.css`: added Mode, Close, pending/error/effective status display, disabled pending toggle state, and compact layout updates.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: connected Harmony ON/OFF and quick parameter changes to `channel-harmony-set-enabled` and `channel-harmony-configure` when the native engine is running, with preview fallback when unavailable.

Implemented behavior:
- User can stay on Mixer, click HARMONY ON/OFF, open the gear tray, adjust key/scale/mode/voices/level, close the tray, and keep state intact.
- Native ACKs reconcile desired/effective enabled, revision, primary instance ID, key, scale, mode, voice intervals, and level.
- Toggling Harmony on the vocal strip does not select or change FX A/B programs and does not enable Harmony on non-vocal channels.
- Gear opens settings only; it does not toggle Harmony.

Validation:
- command: `npm run typecheck`
- exit/result: `0`; TypeScript passed.
- command: `npm run test:ui -- preview-adapter`
- exit/result: `0`; preview adapter unit tests 9/9 passed.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 33/33, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- HARM-01 is UI/ACK binding only; native audio renderer insertion, voice-only ramp, lead alignment, Monitor Fast effective-state messaging, and recording/offline snapshot semantics remain pending HARM-02.
- Advanced multiple-primary chooser UX is still represented by native rejection rather than a full selection dialog.

## HARM-02 — Voice Transition And Sync Semantics
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: HARM-01 IMPLEMENTED_UNVERIFIED

### HARM-02 Checkpoint — Harmony Voice Ramp And Automation IDs

Changed files:
- `native/engine/src/dsp/fx/HarmonyEffect.hpp`, `native/engine/src/dsp/fx/HarmonyEffect.cpp`: added native enabled target/ramp for harmony voices, explicit `setEnabled`, `setHarmonyLevelDb`, and realtime parameter 5 for Harmony Level. The dry/lead samples remain untouched while voices are added or ramped out.
- `native/engine/tests/fx/HarmonyEffectTest.cpp`: verifies disabled Harmony keeps lead unchanged, silence still gates voices, and Harmony Level does not trim lead signal.
- `native/engine/src/engine/Automation.hpp`, `native/engine/src/engine/Automation.cpp`: added stable automation IDs for `harmony.enabled.<channel>` and `harmony.level.<channel>`.
- `native/engine/tests/AutomationTest.cpp`: verifies Harmony desired-state and level automation IDs.

Implemented behavior:
- OFF/disabled Harmony mutes generated voices only; lead remains present once and at the same level.
- Harmony Level is an additive voice-level trim and does not control lead gain.
- Enable/disable uses a bounded native ramp for voice contribution rather than abruptly muting the whole processor output.
- Automation naming now has explicit desired-state targets for Harmony enable and level.

Validation:
- command: `native/engine/build/native/engine/local-mixer-harmony-effect-tests`
- exit/result: `0`; HarmonyEffect tests passed including disabled-lead and level semantics.
- command: `native/engine/build/native/engine/local-mixer-automation-tests`
- exit/result: `0`; automation tests passed including Harmony IDs.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 33/33, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- HARM-02 adds processor semantics and automation IDs, but full channel insert graph wiring, monitor-profile effective status, recording/offline graph snapshots, and undo/redo integration remain pending later integration/QA gates.
- Auditory click testing and real vocal fixture acceptance are still NOT_RUN pending HARM-03/INT hardware QA.

## Phase17 — Plugin Hosting/runtime Integration
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: HARM-02 IMPLEMENTED_UNVERIFIED

### Phase17 Checkpoint — Out-of-process Scanner And Plugin State Contracts

Changed files:
- `native/engine/src/engine/PluginRegistry.hpp`, `native/engine/src/engine/PluginRegistry.cpp`: added plugin descriptor/cache contracts, AU/VST3 format classification, allowed-root validation, blacklist rejection, missing-plugin placeholder state, and deterministic scan-result handling.
- `native/engine/src/plugin_scanner_main.cpp`: added a separate scanner executable with `--self-test` and single-path metadata scan commands.
- `native/engine/tests/PluginRegistryTest.cpp`: verifies accepted plugins, unsupported formats, blacklisted IDs, disallowed roots, cache replacement, and missing placeholders.
- `native/engine/src/engine/SessionDocument.hpp`, `native/engine/src/engine/SessionDocument.cpp`, `native/engine/tests/SessionDocumentTest.cpp`: added persisted plugin instance identifier/version/missing/state snapshot fields and roundtrip coverage.
- `native/engine/CMakeLists.txt`: wires the registry, scanner executable, scanner self-test, and plugin registry native test into the CMake/CTest graph.

Implemented behavior:
- Plugin scans are represented by native registry data instead of UI-only placeholders.
- AU `.component` and VST3 `.vst3` paths are classified, cached, and rejected when outside configured plugin roots.
- Blacklisted plugin identifiers are blocked at registry ingestion time.
- Sessions can retain plugin instance identity and opaque base64 state, including missing-plugin placeholders for later restore.
- The scanner runs as a separate native process contract and reports JSON metadata for command integration.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-plugin-scanner local-mixer-plugin-registry-tests local-mixer-session-document-tests`
- exit/result: `0`; scanner, registry tests, and session document tests built successfully.
- command: `native/engine/build/native/engine/local-mixer-plugin-registry-tests`
- exit/result: `0`; plugin registry tests passed.
- command: `native/engine/build/native/engine/local-mixer-plugin-scanner --self-test`
- exit/result: `0`; scanner reported `{"scanner":"ok","selfTest":true}`.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 35/35, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- This checkpoint is a safe plugin foundation only; actual AU/VST3 instantiation, realtime processing insertion, plugin editor windows, latency compensation, crash recovery, and broad third-party compatibility QA remain pending later integration work.
- Scanner JSON is intentionally minimal and is not yet a full plugin metadata database.

## Phase18 — Per-app System Capture
Status: IMPLEMENTED_UNVERIFIED_FOUNDATION
Prerequisites: Phase17 IMPLEMENTED_UNVERIFIED

### Phase18 Checkpoint — Tap Capability And Assignment Audit

Changed files:
- `native/engine/src/engine/PerAppCapture.hpp`, `native/engine/src/engine/PerAppCapture.cpp`: added native per-app capture capability, process-source identity, assignment validation, permission/support status, system-mix exclusivity, and own-process recapture rejection.
- `native/engine/tests/PerAppCaptureTest.cpp`: verifies independent two-app assignments, original mute count, system-mix conflict, own-process rejection, permission-required state, and JSON audit output.
- `native/engine/src/main.cpp`: added `--per-app-capture-capability` and stdio `per-app-capture-capability` command.
- `apps/desktop/electron/EngineSupervisor.ts`: normalizes closed-stdin/EPIPE writes into a controlled engine-unavailable rejection so desktop shutdown does not surface a raw JavaScript main-process error.
- `native/engine/CMakeLists.txt`: wires Phase18 source and native CTest target.

Implemented behavior:
- The engine has a native-only capability contract for Core Audio tap style per-app capture.
- Per-app assignments are explicitly exclusive with system-mix capture on overlapping sources.
- Each selected process source must be present, running, capturable, and not the mixer engine/app itself.
- Permission-required and unsupported states are first-class statuses rather than a checked UI box.
- Current CLI reports macOS tap capability as a foundation with runtime source enumeration disabled until the real tap backend is completed.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-per-app-capture-tests`
- exit/result: `0`; engine and per-app capture tests built successfully. macOS libsamplerate deployment warning remains non-fatal and pre-existing.
- command: `native/engine/build/native/engine/local-mixer-per-app-capture-tests`
- exit/result: `0`; per-app capture audit tests passed.
- command: `native/engine/build/native/engine/local-mixer-engine --per-app-capture-capability`
- exit/result: `0`; returned `platformSupported:true`, `backend:"CoreAudioTap"`, `permissionGranted:false`, `sources:[]`, and note that runtime source enumeration is not enabled in this build yet.
- command: `npm run test:ui -- engine-supervisor`
- exit/result: `0`; EngineSupervisor 9/9 tests passed, including closed-stdin/EPIPE rejection.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- Real Core Audio process tap enumeration, permission request UI, tap creation/cleanup, adaptive clock bridge, app restart recovery, and actual two-app audio capture are NOT_RUN and not claimed as VERIFIED.
- `canMuteOriginal` is currently false in the runtime capability command until a tested tap backend can prove original muting/exclusion behavior.

## INT-00 — Connected Product Audit
Status: IMPLEMENTED_UNVERIFIED_AUDIT
Prerequisites: Phase18 IMPLEMENTED_UNVERIFIED

### INT-00 Checkpoint — Feature Traceability And Command Exposure

Changed files:
- `docs/feature-traceability.md`: added the required row-per-feature audit for setup/devices, source management, import, transport, strip controls, DSP, routing, Vocal FX, 99 programs, Harmony, recording, export, session, automation/MIDI, plugins, per-app capture, recovery, and packaging.
- `apps/desktop/electron/main.ts`: whitelisted `per-app-capture-capability` so the desktop shell can call the native Phase18 audit command.
- `scripts/test-native.mjs`: extended the native stdio protocol smoke test to cover `per-app-capture-capability`.

Implemented behavior:
- The project now has an explicit traceability matrix with UI entrypoint, command/API, native owner, save field, tests, evidence, and status.
- INT-00 no longer hides unfinished product areas behind generic build success; partial and not-run items are visible.
- Per-app capture capability is exposed through the same guarded Electron command bridge as other engine commands.

Validation:
- command: `npm run check:plan`
- exit/result: `0`; 49-phase plan remains valid with INT-00 evidence paths.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke including `per-app-capture-capability`, and UI/desktop build passed.

Known limitations:
- INT-00 is a connection/audit checkpoint. It does not complete INT-01 daily journey, INT-02 packaging smoke, VFX-09/MIXFX-05/HARM-03 listening QA, or final PRODUCT_VERIFIED status.
- Several traceability rows remain `PARTIAL` by design until their real UI flows, packaged app checks, hardware tests, or listening tests are executed.

## INT-01 — Daily Journey And Data Integrity
Status: IMPLEMENTED_UNVERIFIED_CONTRACT
Prerequisites: INT-00 IMPLEMENTED_UNVERIFIED

### INT-01 Checkpoint — Automated Contract Journey

Changed files:
- `scripts/run-int01-journey.mjs`: added a repeatable integration script that builds native targets, runs recording/export/session data-integrity CTest targets, drives the native stdio engine from an empty fixture journey, and writes a report.
- `docs/reports/int01-daily-journey.md`: generated evidence report with scenario coverage and explicit partial/not-run items.
- `package.json`: added `npm run test:int01`.

Implemented behavior:
- The automated journey imports a generated backing WAV through native media jobs, polls waveform completion, starts transport, loads the FX bank, selects FX A program 12, enables Harmony for the vocal channel, exercises invalid-file fault handling, checks per-app capture capability, stops transport, and shuts down cleanly.
- Recording/export/session native integrity tests are grouped into a repeatable INT-01 command.
- The report separates contract PASS evidence from live hardware, live record/replay, audible FX, and packaged export playback work that has not run.

Validation:
- command: `npm run test:int01`
- exit/result: `0`; generated `docs/reports/int01-daily-journey.md` with PASS for native build, record/export/session CTest subset, and stdio daily journey contract.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- This is not a full daily-use PRODUCT_VERIFIED result. Real mic monitor, audible A/B effects, real Harmony audition, record-stop-playback take, save/open through desktop menu, export dialog, and offline playback of an exported result are still partial or NOT_RUN.
- Slow disk, device disconnect, and packaged-app fault injection remain pending later gates.

## INT-02 — Development Package And Permission Smoke
Status: IMPLEMENTED_UNVERIFIED_PACKAGE_SMOKE
Prerequisites: INT-01 IMPLEMENTED_UNVERIFIED

### INT-02 Checkpoint — Unsigned macOS Dev App Bundle

Changed files:
- `scripts/package-mac-unsigned.mjs`: replaced the packaging stub with an unsigned development `.app` builder that copies Electron runtime, `dist/ui`, `dist/electron`, native engine, plugin scanner, sound-pad helper, and sound-pad assets into app resources; writes bundle identity and microphone/screen-capture usage descriptions; creates a zip archive; and writes a smoke report.
- `docs/reports/int02-package-smoke.md`: generated package smoke evidence.
- `docs/task-plan.json`, `docs/progress.md`: updated INT-02 status and evidence paths.

Implemented behavior:
- `npm run package:mac:unsigned` builds UI, desktop main, native engine, native sound-pad helper, then creates `build/package/Local Audio Mixer.app`.
- The script also creates `build/package/Local-Audio-Mixer-dev.zip`.
- Packaged resource smoke checks verify arm64 Electron/runtime binaries, packaged native engine `--version`, plugin scanner `--self-test`, and sound-pad helper `--validate`.
- The generated `Info.plist` contains `NSMicrophoneUsageDescription` for packaged-app TCC identity.

Validation:
- command: `npm run package:mac:unsigned`
- exit/result: `0`; dev `.app`, archive, and `docs/reports/int02-package-smoke.md` were generated. Smoke report shows PASS for Electron, native engine, plugin scanner, sound-pad helper architecture, packaged engine version, plugin scanner self-test, and sound-pad validation.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- Runtime GUI launch from the packaged `.app`, macOS microphone/capture permission prompt behavior, offline import/record/export from installed location, plugin editor opening, disconnect recovery, and clean-location manual acceptance are NOT_RUN in this command flow.
- Package is unsigned and not notarized; public distribution remains conditional.

## VFX-09 — Vocal FX Integration Release Gate
Status: IMPLEMENTED_UNVERIFIED_QA_REPORT
Prerequisites: INT-02 IMPLEMENTED_UNVERIFIED

### VFX-09 Checkpoint — Quality And Latency Reports

Changed files:
- `docs/reports/vocal-fx-quality.md`: added VFX quality evidence matrix for all 12 native effect families plus required human/hardware QA gaps.
- `docs/reports/vocal-fx-latency.md`: added latency model and open-measurement report for rack, pitch, harmony, vocoder, record/export alignment, and live monitoring.
- `docs/task-plan.json`, `docs/progress.md`: updated VFX-09 status and evidence paths.

Implemented behavior:
- The VFX final gate now has the two required report files.
- Reports distinguish portable native tests from missing vocal recordings, listening QA, persistent wet WAV renders, heavy-chain benchmark, and packaged permission/device evidence.
- No report claims that labels, mockups, or unit tests alone complete final vocal FX release acceptance.

Validation:
- command: `ctest --test-dir native/engine/build -R 'local-mixer-(effect|time|character|pitch|harmony|vocoder)' --output-on-failure`
- exit/result: `0`; 10/10 focused tests passed, including effect registry/rack/time/character/pitch/harmony/vocoder coverage.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- VFX-09 remains `IMPLEMENTED_UNVERIFIED_QA_REPORT`, not `VERIFIED`, because real vocal fixtures, listening rubric, persistent dry/wet render files, callback p99/xrun metrics, live monitoring comfort, and record/export graph comparisons are NOT_RUN or PARTIAL.

## MIXFX-05 — 99 Program And Dual-unit QA
Status: IMPLEMENTED_UNVERIFIED_QA_REPORT
Prerequisites: VFX-09 IMPLEMENTED_UNVERIFIED

### MIXFX-05 Checkpoint — DSP99 Report

Changed files:
- `docs/reports/mixer-dsp99.md`: added required QA report for 99 factory programs, dual-unit controller behavior, session/export contracts, 8B.9 user scenario coverage, and open soak/audition work.
- `docs/task-plan.json`, `docs/progress.md`: updated MIXFX-05 status and evidence paths.

Implemented behavior:
- The project now records concrete automated evidence for exact 99-program bank size, contiguous IDs, program 12/50/99 anchors, wet-only recipe expansion, dual-unit defaults, transitions, revision conflicts, macro override/reset, prepare rollback, session persistence, and FX return stem planning.
- The report is explicit that audible plate/delay behavior, all-99 render files, batch audition, and 30-minute soak are still not verified.

Validation:
- command: `ctest --test-dir native/engine/build -R 'local-mixer-fx-(program|send|tests)|local-mixer-export-tests|local-mixer-session-document-tests' --output-on-failure`
- exit/result: `0`; 7/7 focused tests passed for FX processors/program registry/program JSON/controller/send-return bus/export/session document.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- MIXFX-05 remains `IMPLEMENTED_UNVERIFIED_QA_REPORT`; 99-program impulse/sine/vocal renders, level-matched audition, hardware system+mic+file soak, CPU/deadline/xrun metrics, and realtime fade listening evidence are NOT_RUN.

## HARM-03 — Harmony Button End-to-end QA
Status: IMPLEMENTED_UNVERIFIED_QA_REPORT
Prerequisites: MIXFX-05 IMPLEMENTED_UNVERIFIED

### HARM-03 Checkpoint — Harmony Checklist Report

Changed files:
- `docs/reports/harmony-button.md`: added required Harmony button report with automated evidence, 8C.6 checklist status, and manual evidence gaps.
- `docs/task-plan.json`, `docs/progress.md`: updated HARM-03 status and evidence paths.

Implemented behavior:
- HARM-03 now has explicit evidence for primary instance binding, duplicate/capacity protection, session roundtrip, automation IDs, generated voice semantics, disabled-lead preservation, and Harmony Level voice-only behavior.
- The report records UI contract coverage for Mixer shortcut/gear/key/scale behavior while keeping hardware/listening claims separate.

Validation:
- command: `ctest --test-dir native/engine/build -R 'local-mixer-(harmony|channel-harmony|automation|session)' --output-on-failure`
- exit/result: `0`; 4/4 focused tests passed for session document, automation, channel harmony controller, and harmony effect.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- HARM-03 is not final `VERIFIED`; real vocal recording, known-key auditory QA, packaged screenshot, audio-system+music+Harmony together, UI tab switching in packaged app, undo/redo, record/export, Monitor Fast, and backend-failure behavior remain PARTIAL or NOT_RUN.

## INT-03 — Product Acceptance Audit
Status: IMPLEMENTED_UNVERIFIED_AUDIT
Prerequisites: HARM-03 IMPLEMENTED_UNVERIFIED

### INT-03 Checkpoint — Acceptance Report

Changed files:
- `docs/reports/product-acceptance.md`: added overall acceptance verdict, automated gate summary, 8E.11 E2E matrix, conditional feature statuses, critical unresolved work, and Phase19 inputs.
- `docs/task-plan.json`, `docs/progress.md`: updated INT-03 status and evidence paths.

Implemented behavior:
- The project now has the required INT-03 acceptance report.
- The report explicitly sets `NOT_PRODUCT_VERIFIED` because mandatory hardware/listening/plugin/runtime/record/performance/package checks remain incomplete.
- Conditional items are separated from mandatory gaps, following 8E.12.

Validation:
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- INT-03 is an honest audit, not product completion. Critical gaps remain before Phase19 can produce a final handoff with `PRODUCT_VERIFIED`.

## Phase19 — Hardening, Packaging, And Handoff
Status: IMPLEMENTED_UNVERIFIED_HANDOFF
Prerequisites: INT-03 IMPLEMENTED_UNVERIFIED

### Phase19 Checkpoint — Development Handoff Materials

Changed files:
- `README.md`: added setup, development, native test, packaging, system-audio, and known-gap instructions.
- `docs/reports/phase19-handoff.md`: added artifact path, archive checksum, package source revision, bundle identity, command list, report index, and final blocker table.
- `scripts/package-mac-unsigned.mjs`: fixed packaged `Info.plist` updates so the dev app uses bundle ID `audio.local-mixer.dev` and a single microphone usage description.
- `docs/reports/int02-package-smoke.md`: regenerated package smoke after the `Info.plist` fix.
- `docs/task-plan.json`, `docs/progress.md`: updated Phase19 status and evidence paths.

Implemented behavior:
- Unsigned development app and archive are generated at `build/package/Local Audio Mixer.app` and `build/package/Local-Audio-Mixer-dev.zip`.
- Archive SHA-256 is recorded as `1f76af5198503a55866d6b82b234804912e650905bda91b12f3befe97d8306e8`.
- Packaged app `Info.plist` now reports `CFBundleIdentifier` as `audio.local-mixer.dev` and includes microphone/screen-capture usage descriptions.
- Handoff docs point users to the exact reports that still block final acceptance.

Validation:
- command: `npm run package:mac:unsigned`
- exit/result: `0`; rebuilt unsigned development `.app`, archive, and package smoke report.
- command: `plutil -p 'build/package/Local Audio Mixer.app/Contents/Info.plist' | rg 'CFBundleIdentifier|CFBundleName|CFBundleDisplayName|NSMicrophone|NSScreenCapture'`
- exit/result: `0`; bundle ID, app names, microphone usage description, and screen-capture usage description were present and correct.
- command: `shasum -a 256 build/package/Local-Audio-Mixer-dev.zip`
- exit/result: `0`; checksum `1f76af5198503a55866d6b82b234804912e650905bda91b12f3befe97d8306e8`.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- Phase19 is not `PRODUCT_VERIFIED`. Packaged GUI launch, packaged TCC prompt, clean-location offline workflow, plugin editor, device disconnect/sleep/wake, live record/replay/export, AU/VST3 runtime hosting, per-app taps, and stress benchmarks remain PARTIAL or NOT_RUN.
- The archive was built from source revision `0f7acca`; rerun `npm run package:mac:unsigned` after committing Phase19 docs if an artifact stamped with the final documentation commit is required.

### Phase19 Hardening Checkpoint — Engine Response Serializer Split

Changed files:
- `native/engine/src/engine/EngineResponseJson.hpp`, `native/engine/src/engine/EngineResponseJson.cpp`: moved reusable native protocol response serializers out of the CLI entry point.
- `native/engine/src/main.cpp`: now delegates device/status/persistent-monitor/FX-program/harmony response JSON formatting to the engine protocol helper.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired the helper into the native library and recorded evidence paths.

Implemented behavior:
- No audio behavior is intentionally changed.
- The native CLI entry point shrank from 915 lines to 811 lines, reducing pressure on the 1,000-line hard limit before deeper realtime DSP graph integration.

Validation:
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.
- command: `git diff --check`
- exit/result: `0`; no whitespace errors.

Known limitations:
- `native/engine/src/main.cpp` still reports a file-size warning at 812 lines and should be split further before adding more command handlers.
- This checkpoint does not resolve the remaining product gaps in realtime DSP graph wiring, plugin hosting, per-app taps, record/replay/export, packaged TCC, or stress QA.

### Phase19 Hardening Checkpoint — Native Channel Processor Chain Wiring

Changed files:
- `native/engine/src/engine/ChannelProcessorChain.hpp`, `native/engine/src/engine/ChannelProcessorChain.cpp`: added a reusable native channel processor chain using the existing EQ, noise gate, compressor, and de-esser DSP modules.
- `native/engine/src/engine/MixerGraph.hpp`, `native/engine/src/engine/MixerGraph.cpp`: added per-strip processor configuration and invoked the processor chain inside graph processing before fader/pan/meter output.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: sends `EQ`, `COMP`, and `NOISE` processor flags through `sync-mixer-graph`.
- `native/engine/tests/MixerGraphTest.cpp`: added in-graph native processor evidence proving bypassed processors preserve signal and active native noise processing attenuates below-threshold audio.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired build evidence and updated remaining-gap wording.

Implemented behavior:
- Channel strip processor buttons now reach native `MixerGraph` state instead of being UI-only flags.
- The first realtime channel processor path is exercised in native tests without Web Audio, renderer DSP, browser capture, or PCM IPC.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-graph-tests`
- exit/result: `0`; native engine and mixer graph test target built.
- command: `ctest --test-dir native/engine/build -R local-mixer-graph-tests --output-on-failure`
- exit/result: `0`; focused mixer graph test passed, including native processor bypass/active behavior.
- command: `npm run check:file-size`
- exit/result: `0`; file-size gate passed with warning `native/engine/src/main.cpp 818`.

Known limitations:
- EQ still uses the default approved-UI curve; per-band parameter values from the right processing panel are not synced to native yet.
- De-esser has native DSP and chain support but no dedicated mixer UI toggle/parameter sync in this checkpoint.
- Vocal FX rack, FX A/B return processing, Harmony insertion, record/export parity, hardware listening, and stress QA remain partial.

### Phase19 Hardening Checkpoint — EQ Band Parameter Sync To Native Graph

Changed files:
- `apps/desktop/src/features/mixer/MixerPage.tsx`: includes four per-channel EQ band frequency, gain, Q, and type fields in the `sync-mixer-graph` payload.
- `native/engine/src/main.cpp`: reads the EQ band fields into `ChannelProcessorConfig` while keeping old payloads compatible with default values.
- `native/engine/tests/MixerGraphTest.cpp`: added native graph evidence that an active custom EQ band changes channel output after filter settle.
- `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: updated DSP traceability and remaining blocker wording.

Implemented behavior:
- The channel processing panel's per-channel EQ values are no longer UI-only state; they can be consumed by the native mixer graph through `sync-mixer-graph`.
- Native graph tests now cover both processor bypass/active noise behavior and custom EQ behavior.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-graph-tests`
- exit/result: `0`; native engine and graph test target built.
- command: `ctest --test-dir native/engine/build -R local-mixer-graph-tests --output-on-failure`
- exit/result: `0`; focused mixer graph test passed.
- command: `npm run check:file-size`
- exit/result: `0`; file-size gate passed with warning `native/engine/src/main.cpp 842`.

Known limitations:
- `native/engine/src/main.cpp` grew to 842 lines; command parsing should be split before adding more protocol surface.
- Compressor parameters, de-esser parameters, Vocal FX rack insertion, FX A/B returns, Harmony insertion, export parity, live listening, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Mixer Graph Sync Parser Split

Changed files:
- `native/engine/src/engine/EngineGraphSyncJson.hpp`, `native/engine/src/engine/EngineGraphSyncJson.cpp`: moved `sync-mixer-graph` payload parsing, EQ field mapping, monitor selection extraction, and graph publication response formatting out of the CLI entry point.
- `native/engine/src/main.cpp`: now keeps the stdio loop and delegates graph sync parsing to the engine protocol helper.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/progress.md`: wired the helper into the native library and recorded evidence paths.

Implemented behavior:
- No audio behavior is intentionally changed.
- `native/engine/src/main.cpp` shrank from 842 lines to 718 lines, removing the file-size warning while preserving the new EQ/processor sync path.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-graph-tests`
- exit/result: `0`; native engine and graph test target built.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 18/18, native CTest 36/36, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- This is modular hardening only; compressor parameter sync, de-esser UI control, Vocal FX rack insertion, FX A/B returns, Harmony insertion, export parity, live listening, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Channel Processor Order And Compressor Defaults

Changed files:
- `native/engine/src/engine/ChannelProcessorChain.cpp`: aligned channel-strip processor order with the spec path by running noise/gate before EQ, then compressor, then de-esser; updated default compressor settings to the approved panel values.
- `native/engine/tests/MixerGraphTest.cpp`: added default compressor assertions alongside native processor graph coverage.
- `docs/progress.md`: recorded evidence and remaining limitations.

Implemented behavior:
- Default native channel compressor settings now match the visible channel processing panel: threshold `-18 dB`, ratio `3:1`, attack `10 ms`, release `120 ms`.
- Native channel processor ordering is closer to the specified strip path: noise/gate -> EQ -> compressor -> de-esser.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-graph-tests && ctest --test-dir native/engine/build -R local-mixer-graph-tests --output-on-failure`
- exit/result: `0`; focused graph test passed.

Known limitations:
- Compressor parameters are still fixed starting values from UI/native defaults; editable compressor/noise/de-esser parameter state is not yet persisted or synced from dedicated controls.
- Vocal FX rack insertion, FX A/B returns, Harmony insertion, export parity, live listening, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Monitor Processor Config Wiring

Changed files:
- `native/engine/src/engine/EngineGraphSyncJson.hpp`, `native/engine/src/engine/EngineGraphSyncJson.cpp`: retained the monitored channel's native `ChannelProcessorConfig` in `SyncedMonitorSelection`.
- `native/engine/src/platform/macos/CoreAudioPassthrough.hpp`, `native/engine/src/platform/macos/CoreAudioPassthrough.mm`: added processor config to the passthrough request and applied it to the preallocated monitor `MixerGraph` before callbacks start.
- `native/engine/src/main.cpp`: passes monitored channel processor state to `PersistentPassthroughMonitor`.
- `native/engine/tests/EngineGraphSyncJsonTest.cpp`: added parser coverage proving processor flags and EQ band values are preserved for the monitor path.
- `native/engine/CMakeLists.txt`, `docs/task-plan.json`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired the new test and updated evidence.

Implemented behavior:
- Starting desktop monitor after `sync-mixer-graph` can now use the same native channel processor config selected in the UI, instead of monitoring dry gain/pan only.
- Processor config is applied before the Core Audio callback starts; the callback still uses preallocated buffers and existing `MixerGraph` processing.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-engine-graph-sync-json-tests && ctest --test-dir native/engine/build -R local-mixer-engine-graph-sync-json-tests --output-on-failure`
- exit/result: `0`; native engine and graph-sync parser test built, and the focused parser test passed.

Known limitations:
- Hardware listening for processed monitoring is still NOT_RUN in this environment.
- De-esser UI control, Vocal FX rack insertion, FX A/B returns, Harmony insertion, export parity, live callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Dynamics Parameter Sync

Changed files:
- `apps/desktop/src/adapters/MixerControlPort.ts`, `apps/desktop/src/fixtures/approvedMixerSession.ts`, `apps/desktop/src/adapters/preview/PreviewAdapter.ts`: added per-channel compressor and de-esser state with clamped preview updates.
- `apps/desktop/src/features/processing/components/ChannelProcessingPanel.tsx`: made compressor and de-esser controls interactive via existing rotary controls.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: sends compressor and de-esser parameters through `sync-mixer-graph`.
- `native/engine/src/engine/EngineGraphSyncJson.cpp`, `native/engine/tests/EngineGraphSyncJsonTest.cpp`: reads compressor/de-esser parameter fields into native `ChannelProcessorConfig` and verifies monitor selection preserves them.
- `tests/ui/preview-adapter.test.ts`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: added UI state coverage and updated remaining-gap wording.

Implemented behavior:
- Compressor threshold, ratio, attack, and release are editable per channel and carried to native graph sync.
- De-esser detector frequency, threshold, and max reduction are represented per channel; visible de-esser frequency/threshold controls now update state and sync to native.
- Existing command payloads remain backward compatible by falling back to native defaults when fields are absent.

Validation:
- command: `npm run typecheck`
- exit/result: `0`; TypeScript check passed.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 19/19, native CTest 39/39, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.
- command: `cmake --build native/engine/build --target local-mixer-engine-graph-sync-json-tests && ctest --test-dir native/engine/build -R local-mixer-engine-graph-sync-json-tests --output-on-failure`
- exit/result: `0`; native graph-sync parser test passed.
- command: `npm run test:ui`
- exit/result: `0`; Vitest 19/19 passed.

Known limitations:
- Noise gate threshold/timing is still native-default only; a dedicated UI parameter surface is pending.
- Vocal FX rack insertion, FX A/B returns, Harmony insertion, export parity, live listening, callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — FX A/B Send-return In MixerGraph

Changed files:
- `native/engine/src/engine/MixerGraph.hpp`, `native/engine/src/engine/MixerGraph.cpp`: added per-strip FX A/B send state, FX unit enable/return state, preallocated send/wet buffers, FX meters, and `processWithFx` wet-return rendering.
- `native/engine/tests/MixerGraphTest.cpp`: added graph-level evidence that enabled FX A returns add wet-only signal, disabled units preserve dry-only output, and FX meters reflect input/return peaks.
- `apps/desktop/src/features/mixer/MixerPage.tsx`: sends FX A/B unit enable/return values plus per-channel send enable/gain values through `sync-mixer-graph`.
- `native/engine/src/engine/EngineGraphSyncJson.cpp`, `native/engine/tests/EngineGraphSyncJsonTest.cpp`: parses and verifies FX send state in the published native graph.
- `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: updated integration evidence and remaining-gap wording.

Implemented behavior:
- FX A/B send and return state now reaches the native `MixerGraph` and can render wet-only returns into the main stereo output using native wet processors.
- The default `process` path remains backward compatible and renders no FX return unless `processWithFx` is used with active units and processors.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine-graph-sync-json-tests local-mixer-graph-tests && ctest --test-dir native/engine/build -R 'local-mixer-(engine-graph-sync-json|graph)-tests' --output-on-failure`
- exit/result: `0`; graph-sync and mixer graph focused tests passed.
- command: `npm run typecheck`
- exit/result: `0`; TypeScript check passed.
- command: `npm run test:ui`
- exit/result: `0`; Vitest 19/19 passed.
- command: `npm run check:file-size`
- exit/result: `0`; file-size gate passed.

Known limitations:
- The graph accepts wet processors, but compact FX A/B program IDs are not yet mapped to the 99 program-specific native DSP recipes in the realtime graph.
- Vocal FX rack insertion, Harmony insertion, export parity, live listening, callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Factory FX Program Wet Renderer

Changed files:
- `native/engine/src/engine/FxProgramWetProcessor.hpp`, `native/engine/src/engine/FxProgramWetProcessor.cpp`: added a native wet processor adapter that configures program recipes from `FxProgramRegistry` into bounded reverb, delay, delay+plate, and modulation-style wet output.
- `native/engine/tests/FxProgramWetProcessorTest.cpp`: verifies program 12 Vocal Plate, program 50 Stereo 320, invalid program rejection, and use as a `MixerGraph::processWithFx` callback.
- `native/engine/CMakeLists.txt`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired build/test evidence and updated remaining-gap wording.

Implemented behavior:
- Factory FX program IDs can now produce native wet audio through a reusable processor instead of only being registry metadata.
- The wet processor can be passed directly to `MixerGraph::processWithFx`, connecting the 99-program recipe layer to the native FX send-return graph surface.
- Scratch buffers are preallocated with `prepare`; the processor does not allocate for the tested steady callback path.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-fx-program-wet-processor-tests && ctest --test-dir native/engine/build -R local-mixer-fx-program-wet-processor-tests --output-on-failure`
- exit/result: `0`; factory FX wet processor test passed.

Known limitations:
- The desktop/native runtime still needs persistent FX program processor instances bound to FX A/B unit snapshots during live graph processing.
- Full 99-program rendered fixture comparison, audible QA, transition crossfade listening, export parity, callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Mixer Render Runtime FX Binding

Changed files:
- `native/engine/src/engine/MixerRenderRuntime.hpp`, `native/engine/src/engine/MixerRenderRuntime.cpp`: added a small native render runtime that owns `MixerGraph` plus persistent FX A/B program wet processors.
- `native/engine/tests/MixerRenderRuntimeTest.cpp`: verifies default FX A/B program IDs, invalid program rejection without replacing the active program, and processing a graph with an FX A factory wet return.
- `native/engine/CMakeLists.txt`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired focused build/test evidence and updated remaining-gap wording.

Implemented behavior:
- Native graph rendering can now use persistent factory FX processors for FX A and FX B instead of requiring ad hoc callbacks at each call site.
- The runtime defaults match the compact FX rows: FX A program 12 Vocal Plate and FX B program 50 Stereo 320.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-render-runtime-tests`
- exit/result: `0`; render runtime test target built.
- command: `ctest --test-dir native/engine/build -R local-mixer-render-runtime-tests --output-on-failure`
- exit/result: `0`; focused render runtime test passed.

Known limitations:
- This checkpoint verifies the native render module, not yet the desktop live Core Audio callback path.
- Full 99-program rendered fixture comparison, audible QA, transition crossfade listening, export parity, callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Core Audio Monitor FX Runtime Binding

Changed files:
- `apps/desktop/src/features/mixer/MixerPage.tsx`: includes compact FX A/B program IDs in the `sync-mixer-graph` payload.
- `native/engine/src/engine/EngineGraphSyncJson.hpp`, `native/engine/src/engine/EngineGraphSyncJson.cpp`: preserves FX program IDs, FX unit return state, and the monitored channel's FX send state in `SyncedMonitorSelection`.
- `native/engine/src/platform/macos/CoreAudioPassthrough.hpp`, `native/engine/src/platform/macos/CoreAudioPassthrough.mm`: adds FX program/unit/send state to monitor requests and renders the callback through `MixerRenderRuntime`.
- `native/engine/src/main.cpp`: passes synced FX monitor state into the persistent monitor start request.
- `native/engine/tests/EngineGraphSyncJsonTest.cpp`: verifies monitor selection keeps FX program, unit, and send state.

Implemented behavior:
- The desktop monitor start path can now carry the selected FX A/B programs, returns, and monitored-channel sends from React state into native monitor rendering.
- The Core Audio output callback uses the preprepared native render runtime, so channel processing and FX A/B wet returns share the same graph renderer.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-engine local-mixer-engine-graph-sync-json-tests`
- exit/result: `0`; native engine and graph-sync parser target built.
- command: `ctest --test-dir native/engine/build -R local-mixer-engine-graph-sync-json-tests --output-on-failure`
- exit/result: `0`; focused parser test passed.
- command: `npm run typecheck`
- exit/result: `0`; TypeScript check passed.

Known limitations:
- Hardware listening of FX returns from the packaged/desktop monitor path is NOT_RUN in this environment.
- Full 99-program rendered fixture comparison, audible QA, transition crossfade listening, export parity, callback timing metrics, and stress QA remain partial.

### Phase19 Hardening Checkpoint — Vocal FX Processor Factory

Changed files:
- `native/engine/src/dsp/fx/EffectProcessorFactory.hpp`, `native/engine/src/dsp/fx/EffectProcessorFactory.cpp`: added a production factory for native effect processors used by the rack.
- `native/engine/tests/fx/EffectProcessorFactoryTest.cpp`: verifies construct/prepare/process coverage for the 10 effect types that already have concrete processor classes, explicit pending behavior for `pitch_shift` and `formant_shift`, and rack replacement using the production factory.
- `native/engine/CMakeLists.txt`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired build/test evidence and updated remaining-gap wording.

Implemented behavior:
- The native Vocal FX rack can now be backed by production processor construction for reverb, delay, chorus, doubler, pitch correction, harmony, saturation, flanger, phaser, and vocoder.
- Pitch-shift and formant-shift remain explicit gaps instead of silently mapping to the wrong processor.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-effect-processor-factory-tests && ctest --test-dir native/engine/build -R local-mixer-effect-processor-factory-tests --output-on-failure`
- exit/result: `0`; focused processor factory test passed.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 19/19, native CTest 40/40, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- The Vocal FX rack still needs live channel insertion, UI slot state sync into native graph, and real vocal listening QA.
- Dedicated `pitch_shift` and `formant_shift` EffectProcessor wrappers are pending.

### Phase19 Hardening Checkpoint — Pitch And Formant Vocal FX Wrappers

Changed files:
- `native/engine/src/dsp/fx/PitchShiftEffects.hpp`, `native/engine/src/dsp/fx/PitchShiftEffects.cpp`: added native `PitchShiftEffect` and `FormantShiftEffect` wrappers over the existing Rubber Band pitch backend.
- `native/engine/src/dsp/fx/EffectProcessorFactory.cpp`: now constructs all 12 Vocal FX catalog entries.
- `native/engine/tests/fx/EffectProcessorFactoryTest.cpp`, `native/engine/tests/fx/PitchShiftEffectsTest.cpp`: expanded factory coverage to 12/12 effects and verifies +12 semitone pitch shifting from 220 Hz to 440 Hz within tolerance.
- `native/engine/CMakeLists.txt`, `docs/feature-traceability.md`, `docs/reports/product-acceptance.md`, `docs/progress.md`: wired build/test evidence and updated remaining-gap wording.

Implemented behavior:
- `pitch_shift` and `formant_shift` are no longer registry-only entries; they have native `EffectProcessor` wrappers.
- Factory construction now covers the full 12-effect Vocal FX catalog.

Validation:
- command: `cmake --build native/engine/build --target local-mixer-pitch-shift-effects-tests local-mixer-effect-processor-factory-tests && ctest --test-dir native/engine/build -R 'local-mixer-(pitch-shift-effects|effect-processor-factory)-tests' --output-on-failure`
- exit/result: `0`; focused factory and pitch-shift wrapper tests passed.
- command: `npm run verify`
- exit/result: `0`; plan, file-size, architecture, typecheck, Vitest 19/19, native CTest 41/41, engine self-test, device enumeration smoke, protocol smoke, and UI/desktop build passed.

Known limitations:
- The wrappers use the existing Rubber Band backend and inherit its latency; this does not satisfy low-latency live-singing Harmony acceptance by itself.
- The Vocal FX rack still needs live channel insertion, UI slot state sync into native graph, and real vocal listening QA.
