add_library(local_mixer_dsp STATIC
  src/dsp/DeEsser.cpp
  src/dsp/Dynamics.cpp
  src/dsp/Eq.cpp
  src/dsp/Fx.cpp
  src/dsp/Gain.cpp
  src/dsp/Meter.cpp
  src/dsp/OutputProtection.cpp
  src/engine/DeviceService.cpp
  src/engine/EngineRuntime.cpp
  src/engine/Export.cpp
  src/engine/Metronome.cpp
  src/engine/MediaFile.cpp
  src/engine/MediaImportJob.cpp
  src/engine/MediaResampler.cpp
  src/engine/MixerControlQueue.cpp
  src/engine/MixerGraph.cpp
  src/engine/MixerGraphController.cpp
  src/engine/Recording.cpp
  src/engine/RoutingGraph.cpp
  src/engine/SystemRouteRecovery.cpp
  src/engine/SystemRouting.cpp
  src/engine/Timeline.cpp
  src/engine/TimelineEdit.cpp
  src/engine/Transport.cpp
  src/engine/VocalStripPreset.cpp
  src/engine/WaveformPyramid.cpp
)

target_include_directories(local_mixer_dsp PUBLIC src)
target_compile_features(local_mixer_dsp PUBLIC cxx_std_20)

add_executable(local-mixer-engine
  src/main.cpp
  src/engine/JsonProtocol.cpp
  src/engine/MediaTransportJson.cpp
  src/engine/SystemRoutingJson.cpp
)

target_link_libraries(local-mixer-engine PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-engine PRIVATE cxx_std_20)

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
