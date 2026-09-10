add_library(local_mixer_dsp STATIC
  src/dsp/Gain.cpp
  src/dsp/OutputProtection.cpp
  src/engine/DeviceService.cpp
  src/engine/EngineRuntime.cpp
  src/engine/MixerGraph.cpp
)

target_include_directories(local_mixer_dsp PUBLIC src)
target_compile_features(local_mixer_dsp PUBLIC cxx_std_20)

add_executable(local-mixer-engine
  src/main.cpp
)

target_link_libraries(local-mixer-engine PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-engine PRIVATE cxx_std_20)

if(APPLE)
  target_sources(local-mixer-engine PRIVATE
    src/platform/macos/CoreAudioDevices.mm
    src/platform/macos/CoreAudioInputMeter.mm
    src/platform/macos/CoreAudioOutputStream.mm
    src/platform/macos/CoreAudioPassthrough.mm
  )
  target_link_libraries(local-mixer-engine PRIVATE "-framework AVFoundation" "-framework CoreAudio" "-framework CoreFoundation")
endif()

add_executable(local-mixer-dsp-tests
  tests/GainTest.cpp
)

target_link_libraries(local-mixer-dsp-tests PRIVATE local_mixer_dsp)
target_compile_features(local-mixer-dsp-tests PRIVATE cxx_std_20)

add_test(NAME local-mixer-dsp-tests COMMAND local-mixer-dsp-tests)

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
