# Feature Traceability

Source: `docs/specs/AUDIO-MIXER-AI-IMPLEMENTATION.md` sections 8E.2 and 8E.8.

Status values:
- `CONNECTED`: UI or command path reaches a native owner and has automated evidence.
- `PARTIAL`: contract/module exists, but final product flow is not fully connected or tested.
- `NOT_RUN`: requires manual hardware/package/listening verification not run in this environment.
- `BLOCKED_ENVIRONMENT`: environment lacks the required device, permission, signing credential, or OS condition.

| Requirement | UI entrypoint | Command/API | Native owner | Save field | Unit test | Integration/manual evidence | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| setup/devices/permissions | HW modal, footer output | `list-devices`, `request-mic-permission`, `engine-status` | `CoreAudioDevices`, `DeviceService`, `EngineRuntime` | device UID in session/channel source | `DeviceServiceTest`, `EngineRuntimeTest` | `npm run verify`; user-run mic tests succeeded, CI-like shell still enumerates 0 devices | `PARTIAL` |
| add/remove/rename/source roles | Mixer strips and source selectors | `sync-mixer-graph` | `MixerGraph`, `MixerGraphController` | `SessionChannelState` | `MixerGraphTest`, `SessionDocumentTest` | protocol smoke covers graph sync; remove/rename dialogs are not final | `PARTIAL` |
| import/drop file | Media import panel | `media-inspect`, `media-import-start/status/cancel` | `MediaFile`, `MediaImportJob`, `WaveformPyramid` | media path/metadata | `MediaTransportTest` | protocol smoke imports WAV fixture | `CONNECTED` |
| transport/timeline/editing | Top transport, Timeline tab preview | `transport-play/pause/stop/seek/status` | `TransportClock`, `Timeline`, `TimelineEdit` | transport/timeline document fields | `TimelineTest`, `TimelineEditTest` | protocol smoke covers play/seek/stop; full clip editing UI is partial | `PARTIAL` |
| strip gain/pan/mute/solo/MON/REC | Channel strips | `sync-mixer-graph`, monitor commands | `MixerGraph`, `CoreAudioPassthrough` | channel state | `MixerGraphTest` | persistent monitor commands smoke-tested; live hardware not run here | `PARTIAL` |
| EQ/HPF/LPF/noise/compressor/de-esser | Channel Processing panel, strip shortcuts | `sync-mixer-graph`; native DSP modules | `ChannelProcessorChain`, `MixerGraph`, `EngineGraphSyncJson`, `Eq`, `Dynamics`, `DeEsser` | channel processor state | `EqTest`, `DynamicsTest`, `DeEsserTest`, `MixerGraphTest`, `EngineGraphSyncJsonTest`, `preview-adapter.test.ts` | UI processor booleans, per-channel EQ bands, compressor knobs, and de-esser knobs now reach native graph/monitor selection; native EQ/noise behavior is verified in-graph; export parity and live listening remain partial | `PARTIAL` |
| aux/subgroup/DCA/routing | Routing tab preview, sends | routing graph APIs; `sync-mixer-graph` | `RoutingGraph`, `FxSendReturnBus`, `MixerGraph` | routing/session fields | `RoutingGraphTest`, `FxSendReturnBusTest`, `MixerGraphTest` | tests cover cycles/sends/DCA metadata; FX A/B sends and wet-only returns now render inside `MixerGraph`; UI final routing editor partial | `PARTIAL` |
| 12 native Vocal FX families | Vocal FX tab | `sync-mixer-graph`; VFX rack contracts | `EffectRegistry`, `EffectProcessorFactory`, `EffectRack`, `VocalFxRackRuntime`, `CoreAudioPassthrough`, individual FX processors | vocal FX slots/presets | VFX CTest suite, `EffectProcessorFactoryTest`, `PitchShiftEffectsTest`, `VocalFxRackRuntimeTest`, `EngineGraphSyncJsonTest` | 12/12 catalog effects have production processor factory coverage; INSERT FX and rack slots reach the Core Audio monitor path; final listening QA pending | `PARTIAL` |
| 99 FX A/B programs | Compact FX rows | `fx-program-bank`, `fx-unit-*`, `sync-mixer-graph` | `FxProgramRegistry`, `FxProgramController`, `FxProgramWetProcessor`, `MixerRenderRuntime`, `MixerGraph`, `CoreAudioPassthrough` | FX unit snapshot | `FxProgramRegistry*Test`, `FxProgramControllerTest`, `FxProgramWetProcessorTest`, `MixerRenderRuntimeTest`, `MixerGraphTest`, `EngineGraphSyncJsonTest` | UI rows send program IDs, FX send/return state reaches native graph and monitor request, factory recipes have native wet processors, and Core Audio monitor rendering uses the runtime path; full 99-program auditory QA and hardware listening remain pending | `PARTIAL` |
| Harmony button | Channel Harmony button/tray | `channel-harmony-*` | `ChannelHarmonyController`, `HarmonyEffect` | channel harmony state | `ChannelHarmonyControllerTest`, `HarmonyEffectTest` | ACK UI wired; real vocal end-to-end pending HARM-03 | `PARTIAL` |
| record dry/processed/master | REC buttons, global REC preview | recording module contract | `Recording` | recording take metadata | `RecordingTest` | writer tests pass; live record/replay journey pending INT-01 | `PARTIAL` |
| codecs/export/stems | Export workflow pending | export module contract | `Export`, codec probe script | export job/session fields | `ExportTest` | codec probe and native tests exist; packaged import/export not run | `PARTIAL` |
| session/undo/autosave/relink | Project menu pending | session document APIs | `SessionDocument` | full session JSON | `SessionDocumentTest` | persistence roundtrip tested; desktop menu flow pending | `PARTIAL` |
| automation/MIDI | MIDI/settings pending | automation module contract | `Automation` | automation lanes/mapping | `AutomationTest` | native IDs tested; device MIDI learn not run | `PARTIAL` |
| AU/VST3 plugins | Plugins/settings pending | scanner executable; registry contract | `PluginRegistry`, `local-mixer-plugin-scanner` | plugin instance/state | `PluginRegistryTest`, scanner self-test | scanner/registry verified; runtime hosting/editor pending | `PARTIAL` |
| per-app capture capability | Preferences/diagnostics pending | `per-app-capture-capability` | `PerAppCapture` | per-app source assignments pending | `PerAppCaptureTest` | command reports capability honestly; real taps NOT_RUN | `PARTIAL` |
| status/error/recovery | engine status, route recovery docs | route/status/recover commands, monitor status | `EngineSupervisor`, `SystemRouteRecovery`, `RealtimeMetrics` | recovery marker | `engine-supervisor.test.ts`, `SystemRoutingTest`, `RealtimeMetricsTest` | EPIPE normalized; recovery smoke tested; monitor status exposes callback/deadline counters | `CONNECTED` |
| packaging/offline use | package scripts pending | dev package scripts pending | Electron main/resource paths | N/A | build checks | `.app` smoke from clean install location pending INT-02 | `PARTIAL` |

## INT-00 Audit Notes

- Browser/Web Audio capture or DSP is still absent by architecture check; audio commands go through native helpers or the native engine.
- Several rows are intentionally `PARTIAL` because INT-00 is the connection audit, not final product verification.
- Hardware and packaged-app evidence must remain `NOT_RUN` or `BLOCKED_ENVIRONMENT` until executed on a real target Mac/package.
