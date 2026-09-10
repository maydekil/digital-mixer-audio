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
