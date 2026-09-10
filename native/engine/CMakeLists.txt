find_package(PkgConfig REQUIRED)
pkg_check_modules(RUBBERBAND REQUIRED rubberband)

add_library(local_mixer_dsp STATIC
  src/dsp/DeEsser.cpp
  src/dsp/Dynamics.cpp
  src/dsp/Eq.cpp
  src/dsp/Fx.cpp
  src/dsp/Gain.cpp
  src/dsp/Meter.cpp
  src/dsp/OutputProtection.cpp
  src/dsp/fx/EffectRack.cpp
  src/dsp/fx/EffectRegistry.cpp
  src/dsp/fx/CharacterEffects.cpp
  src/dsp/fx/DelayEffect.cpp
  src/dsp/fx/HarmonyEffect.cpp
  src/dsp/fx/ModulationEffects.cpp
  src/dsp/fx/PitchBackend.cpp
  src/dsp/fx/PitchCorrectionEffect.cpp
  src/dsp/fx/PitchDetector.cpp
  src/dsp/fx/ReverbEffect.cpp
  src/dsp/fx/VocoderEffect.cpp
  src/engine/Automation.cpp
  src/engine/ChannelHarmonyController.cpp
  src/engine/ChannelProcessorChain.cpp
  src/engine/DeviceService.cpp
  src/engine/EngineGraphSyncJson.cpp
  src/engine/EngineResponseJson.cpp
  src/engine/EngineRuntime.cpp
  src/engine/Export.cpp
  src/engine/FxProgramController.cpp
  src/engine/FxProgramRegistry.cpp
  src/engine/FxProgramRegistryJson.cpp
  src/engine/FxSendReturnBus.cpp
  src/engine/JsonProtocol.cpp
  src/engine/Metronome.cpp
  src/engine/MediaFile.cpp
  src/engine/MediaImportJob.cpp
  src/engine/MediaResampler.cpp
  src/engine/MixerControlQueue.cpp
  src/engine/MixerGraph.cpp
  src/engine/MixerGraphController.cpp
  src/engine/PerAppCapture.cpp
  src/engine/PluginRegistry.cpp
  src/engine/Recording.cpp
  src/engine/RoutingGraph.cpp
  src/engine/SessionDocument.cpp
  src/engine/SystemRouteRecovery.cpp
  src/engine/SystemRouting.cpp
  src/engine/Timeline.cpp
  src/engine/TimelineEdit.cpp
  src/engine/Transport.cpp
  src/engine/VocalStripPreset.cpp
  src/engine/WaveformPyramid.cpp
)

target_include_directories(local_mixer_dsp PUBLIC src)
target_include_directories(local_mixer_dsp PRIVATE ${RUBBERBAND_INCLUDE_DIRS})
target_link_directories(local_mixer_dsp PUBLIC ${RUBBERBAND_LIBRARY_DIRS})
target_link_libraries(local_mixer_dsp PUBLIC ${RUBBERBAND_LIBRARIES} ${RUBBERBAND_LDFLAGS_OTHER})
target_compile_features(local_mixer_dsp PUBLIC cxx_std_20)

add_executable(local-mixer-engine
  src/main.cpp
  src/engine/MediaTransportJson.cpp
  src/engine/SystemRoutingJson.cpp
)

target_link_libraries(local-mixer-engine PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-engine PRIVATE cxx_std_20)

add_executable(local-mixer-plugin-scanner
  src/plugin_scanner_main.cpp
)

target_link_libraries(local-mixer-plugin-scanner PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-plugin-scanner PRIVATE cxx_std_20)

add_test(NAME local-mixer-plugin-scanner-self-test COMMAND local-mixer-plugin-scanner --self-test)

if(APPLE)
  target_sources(local-mixer-engine PRIVATE
    src/platform/macos/CoreAudioDevices.mm
    src/platform/macos/CoreAudioInputMeter.mm
    src/platform/macos/CoreAudioOutputStream.mm
    src/platform/macos/CoreAudioPassthrough.mm
    src/platform/macos/CoreAudioSystemRoute.mm
  )
  target_link_libraries(local-mixer-engine PRIVATE "-framework AVFoundation" "-framework CoreAudio" "-framework CoreFoundation")
endif()

add_executable(local-mixer-dsp-tests
  tests/GainTest.cpp
)

target_link_libraries(local-mixer-dsp-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-dsp-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-dsp-tests COMMAND local-mixer-dsp-tests)

add_executable(local-mixer-meter-tests
  tests/MeterTest.cpp
)

target_link_libraries(local-mixer-meter-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-meter-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-meter-tests COMMAND local-mixer-meter-tests)

add_executable(local-mixer-eq-tests
  tests/EqTest.cpp
)

target_link_libraries(local-mixer-eq-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-eq-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-eq-tests COMMAND local-mixer-eq-tests)

add_executable(local-mixer-dynamics-tests
  tests/DynamicsTest.cpp
)

target_link_libraries(local-mixer-dynamics-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-dynamics-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-dynamics-tests COMMAND local-mixer-dynamics-tests)

add_executable(local-mixer-de-esser-tests
  tests/DeEsserTest.cpp
)

target_link_libraries(local-mixer-de-esser-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-de-esser-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-de-esser-tests COMMAND local-mixer-de-esser-tests)

add_executable(local-mixer-vocal-strip-preset-tests
  tests/VocalStripPresetTest.cpp
)

target_link_libraries(local-mixer-vocal-strip-preset-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-vocal-strip-preset-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-vocal-strip-preset-tests COMMAND local-mixer-vocal-strip-preset-tests)

add_executable(local-mixer-fx-tests
  tests/FxTest.cpp
)

target_link_libraries(local-mixer-fx-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-fx-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-fx-tests COMMAND local-mixer-fx-tests)

add_executable(local-mixer-fx-program-registry-tests
  tests/FxProgramRegistryTest.cpp
)

target_link_libraries(local-mixer-fx-program-registry-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-fx-program-registry-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-fx-program-registry-tests COMMAND local-mixer-fx-program-registry-tests)

add_executable(local-mixer-fx-program-registry-json-tests
  tests/FxProgramRegistryJsonTest.cpp
)

target_link_libraries(local-mixer-fx-program-registry-json-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-fx-program-registry-json-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-fx-program-registry-json-tests COMMAND local-mixer-fx-program-registry-json-tests)

add_executable(local-mixer-fx-program-controller-tests
  tests/FxProgramControllerTest.cpp
)

target_link_libraries(local-mixer-fx-program-controller-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-fx-program-controller-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-fx-program-controller-tests COMMAND local-mixer-fx-program-controller-tests)

add_executable(local-mixer-fx-send-return-bus-tests
  tests/FxSendReturnBusTest.cpp
)

target_link_libraries(local-mixer-fx-send-return-bus-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-fx-send-return-bus-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-fx-send-return-bus-tests COMMAND local-mixer-fx-send-return-bus-tests)

add_executable(local-mixer-output-protection-tests
  tests/OutputProtectionTest.cpp
)

target_link_libraries(local-mixer-output-protection-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-output-protection-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-output-protection-tests COMMAND local-mixer-output-protection-tests)

add_executable(local-mixer-device-service-tests
  tests/DeviceServiceTest.cpp
)

target_link_libraries(local-mixer-device-service-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-device-service-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-device-service-tests COMMAND local-mixer-device-service-tests)

add_executable(local-mixer-engine-runtime-tests
  tests/EngineRuntimeTest.cpp
)

target_link_libraries(local-mixer-engine-runtime-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-engine-runtime-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-engine-runtime-tests COMMAND local-mixer-engine-runtime-tests)

add_executable(local-mixer-engine-graph-sync-json-tests
  tests/EngineGraphSyncJsonTest.cpp
)

target_link_libraries(local-mixer-engine-graph-sync-json-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-engine-graph-sync-json-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-engine-graph-sync-json-tests COMMAND local-mixer-engine-graph-sync-json-tests)

add_executable(local-mixer-graph-tests
  tests/MixerGraphTest.cpp
)

target_link_libraries(local-mixer-graph-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-graph-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-graph-tests COMMAND local-mixer-graph-tests)

add_executable(local-mixer-routing-graph-tests
  tests/RoutingGraphTest.cpp
)

target_link_libraries(local-mixer-routing-graph-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-routing-graph-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-routing-graph-tests COMMAND local-mixer-routing-graph-tests)

add_executable(local-mixer-system-routing-tests
  tests/SystemRoutingTest.cpp
)

target_link_libraries(local-mixer-system-routing-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-system-routing-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-system-routing-tests COMMAND local-mixer-system-routing-tests)

add_executable(local-mixer-media-transport-tests
  tests/MediaTransportTest.cpp
)

target_link_libraries(local-mixer-media-transport-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-media-transport-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-media-transport-tests COMMAND local-mixer-media-transport-tests)

add_executable(local-mixer-timeline-tests
  tests/TimelineTest.cpp
)

target_link_libraries(local-mixer-timeline-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-timeline-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-timeline-tests COMMAND local-mixer-timeline-tests)

add_executable(local-mixer-timeline-edit-tests
  tests/TimelineEditTest.cpp
)

target_link_libraries(local-mixer-timeline-edit-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-timeline-edit-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-timeline-edit-tests COMMAND local-mixer-timeline-edit-tests)

add_executable(local-mixer-recording-tests
  tests/RecordingTest.cpp
)

target_link_libraries(local-mixer-recording-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-recording-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-recording-tests COMMAND local-mixer-recording-tests)

add_executable(local-mixer-export-tests
  tests/ExportTest.cpp
)

target_link_libraries(local-mixer-export-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-export-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-export-tests COMMAND local-mixer-export-tests)

add_executable(local-mixer-session-document-tests
  tests/SessionDocumentTest.cpp
)

target_link_libraries(local-mixer-session-document-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-session-document-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-session-document-tests COMMAND local-mixer-session-document-tests)

add_executable(local-mixer-automation-tests
  tests/AutomationTest.cpp
)

target_link_libraries(local-mixer-automation-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-automation-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-automation-tests COMMAND local-mixer-automation-tests)

add_executable(local-mixer-channel-harmony-controller-tests
  tests/ChannelHarmonyControllerTest.cpp
)

target_link_libraries(local-mixer-channel-harmony-controller-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-channel-harmony-controller-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-channel-harmony-controller-tests COMMAND local-mixer-channel-harmony-controller-tests)

add_executable(local-mixer-plugin-registry-tests
  tests/PluginRegistryTest.cpp
)

target_link_libraries(local-mixer-plugin-registry-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-plugin-registry-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-plugin-registry-tests COMMAND local-mixer-plugin-registry-tests)

add_executable(local-mixer-per-app-capture-tests
  tests/PerAppCaptureTest.cpp
)

target_link_libraries(local-mixer-per-app-capture-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-per-app-capture-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-per-app-capture-tests COMMAND local-mixer-per-app-capture-tests)

add_executable(local-mixer-effect-registry-tests
  tests/fx/EffectRegistryTest.cpp
)

target_link_libraries(local-mixer-effect-registry-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-effect-registry-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-effect-registry-tests COMMAND local-mixer-effect-registry-tests)

add_executable(local-mixer-effect-rack-tests
  tests/fx/EffectRackTest.cpp
)

target_link_libraries(local-mixer-effect-rack-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-effect-rack-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-effect-rack-tests COMMAND local-mixer-effect-rack-tests)

add_executable(local-mixer-time-modulation-effects-tests
  tests/fx/TimeModulationEffectsTest.cpp
)

target_link_libraries(local-mixer-time-modulation-effects-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-time-modulation-effects-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-time-modulation-effects-tests COMMAND local-mixer-time-modulation-effects-tests)

add_executable(local-mixer-character-effects-tests
  tests/fx/CharacterEffectsTest.cpp
)

target_link_libraries(local-mixer-character-effects-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-character-effects-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-character-effects-tests COMMAND local-mixer-character-effects-tests)

add_executable(local-mixer-pitch-backend-tests
  tests/fx/PitchBackendTest.cpp
)

target_link_libraries(local-mixer-pitch-backend-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-pitch-backend-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-pitch-backend-tests COMMAND local-mixer-pitch-backend-tests)

add_executable(local-mixer-pitch-correction-tests
  tests/fx/PitchCorrectionTest.cpp
)

target_link_libraries(local-mixer-pitch-correction-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-pitch-correction-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-pitch-correction-tests COMMAND local-mixer-pitch-correction-tests)

add_executable(local-mixer-harmony-effect-tests
  tests/fx/HarmonyEffectTest.cpp
)

target_link_libraries(local-mixer-harmony-effect-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-harmony-effect-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-harmony-effect-tests COMMAND local-mixer-harmony-effect-tests)

add_executable(local-mixer-vocoder-effect-tests
  tests/fx/VocoderEffectTest.cpp
)

target_link_libraries(local-mixer-vocoder-effect-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-vocoder-effect-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-vocoder-effect-tests COMMAND local-mixer-vocoder-effect-tests)
