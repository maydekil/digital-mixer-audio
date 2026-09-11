#include "dsp/Gain.hpp"
#include "dsp/OutputProtection.hpp"
#include "engine/ChannelHarmonyController.hpp"
#include "engine/EngineGraphSyncJson.hpp"
#include "engine/EngineResponseJson.hpp"
#include "engine/EngineRuntime.hpp"
#include "engine/ExportCommandHandlers.hpp"
#include "engine/FxProgramController.hpp"
#include "engine/FxProgramRegistryJson.hpp"
#include "engine/JsonProtocol.hpp"
#include "engine/MediaTransportJson.hpp"
#include "engine/MixerGraph.hpp"
#include "engine/MixerGraphController.hpp"
#include "engine/PerAppCapture.hpp"
#include "engine/Recording.hpp"
#include "engine/RecordingCommandHandlers.hpp"
#include "engine/SystemRouteRecovery.hpp"
#include "engine/SystemRouting.hpp"
#include "engine/SystemRoutingJson.hpp"
#include "engine/Transport.hpp"
#if defined(__APPLE__)
#include "platform/macos/CoreAudioDevices.hpp"
#include "platform/macos/CoreAudioInputMeter.hpp"
#include "platform/macos/CoreAudioOutputStream.hpp"
#include "platform/macos/CoreAudioPassthrough.hpp"
#include "platform/macos/CoreAudioSystemRoute.hpp"
#endif

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr int kProtocolVersion = 1;
constexpr std::size_t kMaxMessageBytes = 65536;

using localmixer::engine::protocol::escapeJson;
using localmixer::engine::protocol::devicesJson;
using localmixer::engine::protocol::fxProgramAckJson;
using localmixer::engine::protocol::fxProgramSnapshotJson;
using localmixer::engine::protocol::harmonyAckJson;
using localmixer::engine::protocol::harmonyStateJson;
using localmixer::engine::protocol::mediaInfoJson;
using localmixer::engine::protocol::mediaImportStatusJson;
using localmixer::engine::protocol::persistentMonitorStatusJson;
using localmixer::engine::protocol::readJsonBoolField;
using localmixer::engine::protocol::readJsonNumberField;
using localmixer::engine::protocol::readJsonStringField;
using localmixer::engine::protocol::statusJson;
using localmixer::engine::protocol::SyncedMonitorSelection;
using localmixer::engine::protocol::syncMixerGraphResultJson;
using localmixer::engine::protocol::systemRouteDiagnosticsJson;
using localmixer::engine::protocol::systemRouteTransactionJson;
using localmixer::engine::protocol::transportJson;
using localmixer::engine::protocol::writeRawResponse;
using localmixer::engine::protocol::writeResponse;

std::filesystem::path routeRecoveryMarkerPath() {
  if (const auto* path = std::getenv("LOCAL_MIXER_ROUTE_RECOVERY_MARKER")) return path;
  if (const auto* home = std::getenv("HOME")) {
    return std::filesystem::path(home) / "Library" / "Application Support" / "Local Audio Mixer" / "route-recovery.marker";
  }
  return std::filesystem::temp_directory_path() / "local-mixer-route-recovery.marker";
}

std::string argumentValue(int argc, char** argv, const std::string& name, const std::string& fallback = "") {
  for (int index = 2; index + 1 < argc; index += 1) {
    if (argv[index] == name) return argv[index + 1];
  }
  return fallback;
}

double argumentDouble(int argc, char** argv, const std::string& name, double fallback) {
  const auto value = argumentValue(argc, argv, name);
  if (value.empty()) return fallback;
  try {
    return std::stod(value);
  } catch (...) {
    return fallback;
  }
}

std::uint32_t argumentUint(int argc, char** argv, const std::string& name, std::uint32_t fallback) {
  const auto value = argumentValue(argc, argv, name);
  if (value.empty()) return fallback;
  try {
    return static_cast<std::uint32_t>(std::stoul(value));
  } catch (...) {
    return fallback;
  }
}

bool hasArgument(int argc, char** argv, const std::string& name) {
  for (int index = 2; index < argc; index += 1) {
    if (argv[index] == name) return true;
  }
  return false;
}

bool runSelfTest() {
  std::array<float, 4> samples{1.0f, -1.0f, 0.5f, -0.5f};
  localmixer::dsp::applyGain(samples, localmixer::dsp::decibelsToLinear(-6.0f));
  localmixer::dsp::applyOutputProtection(samples, {.masterGainDb = 0.0f, .muted = false, .limitCeiling = 0.5f});
  if (std::fabs(samples[0] - 0.5f) >= 0.0001f || std::fabs(samples[1] + 0.5f) >= 0.0001f) return false;

  localmixer::engine::MixerGraph graph;
  const auto strip = graph.createStrip("Self Test", "#18d6e7");
  if (strip.error != localmixer::engine::MixerError::none) return false;
  std::array<float, 2> input{0.25f, 0.25f};
  std::array<float, 2> left{};
  std::array<float, 2> right{};
  std::array<localmixer::engine::SourceBuffer, 1> sources{
    localmixer::engine::SourceBuffer{.stripId = strip.id, .samples = input, .channels = 1},
  };
  if (graph.process(sources, {.left = left, .right = right}) != localmixer::engine::MixerError::none) return false;
  return std::fabs(left[0] - 0.25f) < 0.0001f && std::fabs(right[0] - 0.25f) < 0.0001f;
}

void printVersion() {
  std::cout
    << "{\"name\":\"local-mixer-engine\","
    << "\"version\":\"0.1.0-phase04-routing-foundation\","
    << "\"protocol\":" << kProtocolVersion << ","
    << "\"audio\":\"not-started\","
    << "\"juce\":\"pinned-8.0.15-not-linked\"}"
    << std::endl;
}

std::vector<localmixer::engine::DeviceDescriptor> loadNativeDevices() {
  std::vector<localmixer::engine::DeviceDescriptor> devices;
#if defined(__APPLE__)
  for (const auto& device : localmixer::platform::macos::listAudioDevices()) {
    devices.push_back(localmixer::engine::DeviceDescriptor{
      .uid = device.uid,
      .name = device.name,
      .isDefaultInput = device.isDefaultInput,
      .isDefaultOutput = device.isDefaultOutput,
      .inputChannels = device.inputChannels,
      .outputChannels = device.outputChannels,
      .sampleRate = device.nominalSampleRate,
      .maxBlockSize = 512,
    });
  }
#endif
  return devices;
}

std::string microphonePermissionState() {
#if defined(__APPLE__)
  return localmixer::platform::macos::microphonePermissionState();
#else
  return "UNSUPPORTED_PLATFORM";
#endif
}

std::string requestMicrophonePermission() {
#if defined(__APPLE__)
  return localmixer::platform::macos::requestMicrophonePermission();
#else
  return "UNSUPPORTED_PLATFORM";
#endif
}

std::string testToneResultJson(
  const std::string& outputUid,
  double sampleRate,
  std::uint32_t durationMs,
  float monitorGainDb
) {
#if defined(__APPLE__)
  const auto result = localmixer::platform::macos::playProtectedTestTone({
    .outputUid = outputUid,
    .projectSampleRate = sampleRate,
    .monitorGainDb = monitorGainDb,
    .durationMs = durationMs,
  });
  std::string json = "\"error\":\"" + escapeJson(result.error) + "\"";
  json += ",\"outputChannels\":" + std::to_string(result.outputChannels);
  json += ",\"actualSampleRate\":" + std::to_string(result.actualSampleRate);
  return "\"played\":" + std::string(result.ok ? "true" : "false") + "," + json;
#else
  return "\"played\":false,\"error\":\"UNSUPPORTED_PLATFORM\",\"outputChannels\":0,\"actualSampleRate\":0";
#endif
}

std::string inputMeterResultJson(const std::string& inputUid, double sampleRate, std::uint32_t durationMs) {
#if defined(__APPLE__)
  const auto result = localmixer::platform::macos::measureInputPeak({
    .inputUid = inputUid,
    .projectSampleRate = sampleRate,
    .durationMs = durationMs,
  });
  std::string json = "\"measured\":" + std::string(result.ok ? "true" : "false");
  json += ",\"error\":\"" + escapeJson(result.error) + "\"";
  json += ",\"inputChannels\":" + std::to_string(result.inputChannels);
  json += ",\"actualSampleRate\":" + std::to_string(result.actualSampleRate);
  json += ",\"peak\":" + std::to_string(result.peak);
  return json;
#else
  return "\"measured\":false,\"error\":\"UNSUPPORTED_PLATFORM\",\"inputChannels\":0,\"actualSampleRate\":0,\"peak\":0";
#endif
}

std::string monitorPassthroughResultJson(
  const std::string& inputUid,
  const std::string& outputUid,
  double sampleRate,
  std::uint32_t inputChannel,
  std::uint32_t outputChannel,
  bool mirrorToAllOutputChannels,
  std::uint32_t durationMs,
  float monitorGainDb
) {
#if defined(__APPLE__)
  const auto result = localmixer::platform::macos::monitorPassthrough({
    .inputUid = inputUid,
    .outputUid = outputUid,
    .projectSampleRate = sampleRate,
    .inputChannel = inputChannel,
    .outputChannel = outputChannel,
    .mirrorToAllOutputChannels = mirrorToAllOutputChannels,
    .durationMs = durationMs,
    .monitorGainDb = monitorGainDb,
  });
  std::string json = "\"monitored\":" + std::string(result.ok ? "true" : "false");
  json += ",\"error\":\"" + escapeJson(result.error) + "\"";
  json += ",\"inputChannels\":" + std::to_string(result.inputChannels);
  json += ",\"outputChannels\":" + std::to_string(result.outputChannels);
  json += ",\"mirrorToAllOutputChannels\":" + std::string(mirrorToAllOutputChannels ? "true" : "false");
  json += ",\"inputSampleRate\":" + std::to_string(result.inputSampleRate);
  json += ",\"outputSampleRate\":" + std::to_string(result.outputSampleRate);
  json += ",\"inputPeak\":" + std::to_string(result.inputPeak);
  return json;
#else
  return "\"monitored\":false,\"error\":\"UNSUPPORTED_PLATFORM\",\"inputChannels\":0,\"outputChannels\":0,\"inputSampleRate\":0,\"outputSampleRate\":0,\"inputPeak\":0";
#endif
}

std::string perAppCaptureCapabilityResultJson() {
  return localmixer::engine::perAppCaptureCapabilityJson(localmixer::engine::PerAppCaptureCapability{
#if defined(__APPLE__)
    .platformSupported = true,
    .permissionGranted = false,
    .canMuteOriginal = false,
    .canExcludeOwnProcess = true,
    .backend = "CoreAudioTap",
    .note = "Runtime process-audio tap enumeration is not enabled in this build yet.",
#else
    .platformSupported = false,
    .backend = "Unavailable",
    .note = "Per-app capture requires macOS Core Audio taps.",
#endif
  });
}

std::optional<localmixer::engine::FxBusId> readFxUnitId(const std::string& line) {
  return localmixer::engine::fxUnitFromName(readJsonStringField(line, "unitId"));
}

void printDevices() {
  const auto devices = loadNativeDevices();
  std::cout << "{\"devices\":" << devicesJson(devices)
            << ",\"micPermission\":\"" << microphonePermissionState() << "\"}" << std::endl;
}

#if defined(__APPLE__)
std::string persistentMonitorStatusJson(const localmixer::platform::macos::PersistentMonitorStatus& status) {
  return persistentMonitorStatusJson(
    status.running,
    status.error,
    status.inputChannels,
    status.outputChannels,
    status.inputSampleRate,
    status.outputSampleRate,
    status.inputPeak,
    status.metrics.callbackCount,
    status.metrics.deadlineMissCount,
    status.metrics.maxCallbackNanos
  );
}
#endif

int runStdioProtocol() {
  localmixer::engine::EngineRuntime runtime;
  localmixer::engine::MediaImportJobManager mediaImportJobs;
  localmixer::engine::MixerGraphController graphController;
  localmixer::engine::FxProgramController fxProgramController;
  localmixer::engine::ChannelHarmonyController harmonyController;
  localmixer::engine::SystemRouteTransactionManager routeTransactionManager;
  localmixer::engine::SystemRouteRecoveryStore routeRecoveryStore(routeRecoveryMarkerPath());
  localmixer::engine::TransportClock transportClock;
  localmixer::engine::RecordingSession recordingSession;
  SyncedMonitorSelection monitorSelection;
#if defined(__APPLE__)
  localmixer::platform::macos::PersistentPassthroughMonitor persistentMonitor;
#endif
  runtime.refreshDevices(loadNativeDevices());
  harmonyController.registerChannel("voice", localmixer::engine::ChannelContentRole::vocal);
  harmonyController.registerChannel("music", localmixer::engine::ChannelContentRole::music);

  std::cout
    << "{\"type\":\"hello\","
    << "\"protocol\":" << kProtocolVersion << ","
    << "\"state\":\"RUNNING\","
    << "\"audio\":\"not-started\","
    << "\"deviceCount\":" << runtime.devices().size() << "}"
    << std::endl;

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.size() > kMaxMessageBytes) {
      writeResponse("", false, "error", "MESSAGE_TOO_LARGE");
      continue;
    }

    if (line.empty() || line.front() != '{' || line.back() != '}') {
      writeResponse("", false, "error", "MALFORMED_JSON");
      continue;
    }

    const auto id = readJsonStringField(line, "id");
    const auto type = readJsonStringField(line, "type");
    if (id.empty() || type.empty()) {
      writeResponse(id, false, "error", "INVALID_COMMAND");
      continue;
    }

    if (type == "ping") {
      writeResponse(id, true, "pong");
    } else if (type == "engine-status") {
      writeRawResponse(id, true, "engine-status", "\"status\":" + statusJson(runtime.status()) +
        ",\"micPermission\":\"" + microphonePermissionState() + "\"");
    } else if (type == "request-mic-permission") {
      writeRawResponse(id, true, "request-mic-permission",
        "\"micPermission\":\"" + requestMicrophonePermission() + "\"");
    } else if (type == "list-devices") {
      runtime.refreshDevices(loadNativeDevices());
      writeRawResponse(id, true, "devices", "\"devices\":" + devicesJson(runtime.devices()) +
        ",\"status\":" + statusJson(runtime.status()) +
        ",\"micPermission\":\"" + microphonePermissionState() + "\"");
    } else if (type == "fx-program-bank") {
      writeRawResponse(id, true, "fx-program-bank", localmixer::engine::protocol::fxProgramBankJsonFields());
    } else if (type == "fx-unit-select-program") {
      const auto unit = readFxUnitId(line);
      if (!unit.has_value()) {
        writeRawResponse(id, false, "fx-unit-select-program",
          "\"accepted\":false,\"applied\":false,\"error\":\"INVALID_UNIT\"");
        continue;
      }
      const auto programId = static_cast<std::uint32_t>(readJsonNumberField(line, "programId").value_or(0.0));
      const auto expectedRevision = static_cast<std::uint64_t>(readJsonNumberField(line, "expectedRevision").value_or(0.0));
      auto ack = fxProgramController.requestProgram(*unit, programId, expectedRevision);
      if (ack.accepted) {
        ack = fxProgramController.processPending(*unit, [](const auto&) {
          return true;
        });
      }
      writeRawResponse(id, ack.accepted, "fx-unit-select-program", fxProgramAckJson(ack));
    } else if (type == "fx-unit-set-macro") {
      const auto unit = readFxUnitId(line);
      if (!unit.has_value()) {
        writeRawResponse(id, false, "fx-unit-set-macro",
          "\"accepted\":false,\"applied\":false,\"error\":\"INVALID_UNIT\"");
        continue;
      }
      const auto expectedRevision = static_cast<std::uint64_t>(readJsonNumberField(line, "expectedRevision").value_or(0.0));
      const auto ack = fxProgramController.setMacroOverride(*unit, expectedRevision);
      writeRawResponse(id, ack.accepted, "fx-unit-set-macro", fxProgramAckJson(ack));
    } else if (type == "fx-unit-reset-macros") {
      const auto unit = readFxUnitId(line);
      if (!unit.has_value()) {
        writeRawResponse(id, false, "fx-unit-reset-macros",
          "\"accepted\":false,\"applied\":false,\"error\":\"INVALID_UNIT\"");
        continue;
      }
      const auto expectedRevision = static_cast<std::uint64_t>(readJsonNumberField(line, "expectedRevision").value_or(0.0));
      const auto ack = fxProgramController.resetMacros(*unit, expectedRevision);
      writeRawResponse(id, ack.accepted, "fx-unit-reset-macros", fxProgramAckJson(ack));
    } else if (type == "fx-unit-snapshot") {
      const auto unit = readFxUnitId(line);
      if (!unit.has_value()) {
        writeRawResponse(id, false, "fx-unit-snapshot", "\"error\":\"INVALID_UNIT\"");
        continue;
      }
      writeRawResponse(id, true, "fx-unit-snapshot",
        "\"unitId\":\"" + std::string(localmixer::engine::fxUnitName(*unit)) + "\"," +
        fxProgramSnapshotJson(fxProgramController.snapshot(*unit)));
    } else if (type == "channel-harmony-set-enabled") {
      const auto channelId = readJsonStringField(line, "channelId");
      const auto enabled = readJsonBoolField(line, "enabled").value_or(false);
      const auto expectedRevision = static_cast<std::uint64_t>(readJsonNumberField(line, "expectedRevision").value_or(0.0));
      const auto ack = harmonyController.setEnabled(channelId, enabled, expectedRevision);
      writeRawResponse(id, ack.accepted, "channel-harmony-set-enabled", harmonyAckJson(ack));
    } else if (type == "channel-harmony-configure") {
      const auto channelId = readJsonStringField(line, "channelId");
      localmixer::engine::HarmonyQuickParams params{
        .key = readJsonStringField(line, "key"),
        .scale = readJsonStringField(line, "scale"),
        .mode = readJsonStringField(line, "mode"),
        .voice1 = readJsonStringField(line, "voice1"),
        .voice2 = readJsonStringField(line, "voice2"),
        .levelDb = static_cast<float>(readJsonNumberField(line, "levelDb").value_or(0.0)),
      };
      if (params.key.empty()) params.key = "C";
      if (params.scale.empty()) params.scale = "Major";
      if (params.mode.empty()) params.mode = "Diatonic";
      if (params.voice1.empty()) params.voice1 = "+3rd";
      if (params.voice2.empty()) params.voice2 = "+5th";
      const auto expectedRevision = static_cast<std::uint64_t>(readJsonNumberField(line, "expectedRevision").value_or(0.0));
      const auto ack = harmonyController.configure(channelId, params, expectedRevision);
      writeRawResponse(id, ack.accepted, "channel-harmony-configure", harmonyAckJson(ack));
    } else if (type == "channel-harmony-snapshot") {
      const auto snapshot = harmonyController.snapshot(readJsonStringField(line, "channelId"));
      if (!snapshot.has_value()) {
        writeRawResponse(id, false, "channel-harmony-snapshot", "\"error\":\"CHANNEL_NOT_FOUND\"");
        continue;
      }
      writeRawResponse(id, true, "channel-harmony-snapshot", harmonyStateJson(*snapshot));
    } else if (type == "prepare-passthrough") {
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto blockSize = static_cast<std::uint32_t>(readJsonNumberField(line, "blockSize").value_or(256.0));
      const auto inputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "inputChannel").value_or(0.0));
      const auto outputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "outputChannel").value_or(0.0));
      const auto monitoringEnabled = readJsonBoolField(line, "monitoringEnabled").value_or(false);
      const auto result = runtime.preparePassthrough(localmixer::engine::PreparePassthroughRequest{
        .project = {.sampleRate = sampleRate, .blockSize = blockSize},
        .inputUid = readJsonStringField(line, "inputUid"),
        .outputUid = readJsonStringField(line, "outputUid"),
        .inputChannel = inputChannel,
        .outputChannel = outputChannel,
        .monitoringEnabled = monitoringEnabled,
      });
      const bool ok = result.error == localmixer::engine::DevicePrepareError::none;
      writeRawResponse(id, ok, "prepare-passthrough",
        "\"error\":\"" + std::string(localmixer::engine::prepareErrorName(result.error)) +
        "\",\"status\":" + statusJson(runtime.status()));
    } else if (type == "media-inspect") {
      const auto fields = mediaInfoJson(readJsonStringField(line, "path"));
      writeRawResponse(id, fields.find("\"imported\":true") != std::string::npos, "media-inspect", fields);
    } else if (type == "media-import-start") {
      const auto framesPerPoint = static_cast<std::uint32_t>(readJsonNumberField(line, "framesPerPoint").value_or(512.0));
      const auto status = mediaImportJobs.start(readJsonStringField(line, "path"), framesPerPoint);
      writeRawResponse(id, status.state != localmixer::engine::MediaImportState::error,
        "media-import-status", mediaImportStatusJson(status));
    } else if (type == "media-import-status") {
      const auto status = mediaImportJobs.status(readJsonStringField(line, "jobId"));
      if (!status.has_value()) {
        writeRawResponse(id, false, "media-import-status", "\"error\":\"UNKNOWN_JOB\"");
      } else {
        writeRawResponse(id, true, "media-import-status", mediaImportStatusJson(*status));
      }
    } else if (type == "media-import-cancel") {
      const auto status = mediaImportJobs.cancel(readJsonStringField(line, "jobId"));
      if (!status.has_value()) {
        writeRawResponse(id, false, "media-import-status", "\"error\":\"UNKNOWN_JOB\"");
      } else {
        writeRawResponse(id, true, "media-import-status", mediaImportStatusJson(*status));
      }
    } else if (type == "transport-play") {
      transportClock.play();
      writeRawResponse(id, true, "transport-status", transportJson(transportClock.snapshot()));
    } else if (type == "transport-pause") {
      transportClock.pause();
      writeRawResponse(id, true, "transport-status", transportJson(transportClock.snapshot()));
    } else if (type == "transport-stop") {
      transportClock.stop();
      writeRawResponse(id, true, "transport-status", transportJson(transportClock.snapshot()));
    } else if (type == "transport-seek") {
      const auto frame = static_cast<std::uint64_t>(readJsonNumberField(line, "positionFrame").value_or(0.0));
      transportClock.seek(frame);
      writeRawResponse(id, true, "transport-status", transportJson(transportClock.snapshot()));
    } else if (type == "transport-status") {
      writeRawResponse(id, true, "transport-status", transportJson(transportClock.snapshot()));
    } else if (type == "export-plan") {
      writeRawResponse(id, true, "export-plan", localmixer::engine::exportPlanJsonFields(line));
    } else if (type == "export-render") {
      writeRawResponse(id, true, "export-render", localmixer::engine::exportRenderJsonFields(line));
    } else if (type == "recording-plan") {
      writeRawResponse(id, true, "recording-plan", localmixer::engine::recordingPlanJsonFields(line));
    } else if (type == "recording-start") {
      writeRawResponse(id, true, "recording-start",
        localmixer::engine::recordingStartJsonFields(line, recordingSession));
    } else if (type == "recording-stop") {
      writeRawResponse(id, true, "recording-stop",
        localmixer::engine::recordingStopJsonFields(recordingSession));
    } else if (type == "routing-system-diagnostics") {
      runtime.refreshDevices(loadNativeDevices());
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto inputStart = static_cast<std::uint32_t>(readJsonNumberField(line, "blackHoleInputStartChannel").value_or(0.0));
      const auto outputStart = static_cast<std::uint32_t>(readJsonNumberField(line, "physicalOutputStartChannel").value_or(0.0));
      const auto fields = systemRouteDiagnosticsJson(
        runtime.devices(),
        readJsonStringField(line, "blackHoleUid"),
        readJsonStringField(line, "physicalOutputUid"),
        sampleRate,
        inputStart,
        outputStart
      );
      writeRawResponse(id, true, "routing-system-diagnostics", fields);
    } else if (type == "routing-system-enable") {
      runtime.refreshDevices(loadNativeDevices());
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto inputStart = static_cast<std::uint32_t>(readJsonNumberField(line, "blackHoleInputStartChannel").value_or(0.0));
      const auto outputStart = static_cast<std::uint32_t>(readJsonNumberField(line, "physicalOutputStartChannel").value_or(0.0));
      const auto diagnostics = localmixer::engine::validateSystemRoute(
        runtime.devices(),
        localmixer::engine::ProjectAudioConfig{.sampleRate = sampleRate, .blockSize = 256},
        localmixer::engine::SystemRouteSelection{
          .blackHoleUid = readJsonStringField(line, "blackHoleUid"),
          .physicalOutputUid = readJsonStringField(line, "physicalOutputUid"),
          .blackHoleInputStartChannel = inputStart,
          .physicalOutputStartChannel = outputStart,
        }
      );
      const bool engineReady = runtime.status().state != localmixer::engine::RuntimeState::error;
      const bool allowOsRouteChange = readJsonBoolField(line, "allowOsRouteChange").value_or(false);
      bool routeApplied = false;
      std::string originalOutputUid = readJsonStringField(line, "originalOutputUid");
#if defined(__APPLE__)
      if (allowOsRouteChange && diagnostics.routeValid && engineReady) {
        if (originalOutputUid.empty()) originalOutputUid = localmixer::platform::macos::currentDefaultOutputUid();
        const auto apply = localmixer::platform::macos::setDefaultOutputUid(diagnostics.selection.blackHoleUid);
        routeApplied = apply.ok;
      }
#endif
      const auto transaction = routeTransactionManager.requestEnable(
        diagnostics,
        originalOutputUid,
        engineReady,
        routeApplied
      );
      if (transaction.state == localmixer::engine::SystemRouteTransactionState::active) {
        routeRecoveryStore.save(transaction);
      }
      writeRawResponse(id, transaction.state == localmixer::engine::SystemRouteTransactionState::active,
        "routing-system-enable", systemRouteTransactionJson(transaction));
    } else if (type == "routing-system-disable") {
      const auto currentTransaction = routeTransactionManager.transaction();
      bool restored = false;
#if defined(__APPLE__)
      if (currentTransaction.ownsSystemRoute && !currentTransaction.originalOutputUid.empty()) {
        const auto restore = localmixer::platform::macos::setDefaultOutputUid(currentTransaction.originalOutputUid);
        restored = restore.ok;
      }
#endif
      const auto transaction = routeTransactionManager.disable(restored);
      if (transaction.error == localmixer::engine::SystemRouteError::none) routeRecoveryStore.clear();
      writeRawResponse(id, transaction.error == localmixer::engine::SystemRouteError::none,
        "routing-system-disable", systemRouteTransactionJson(transaction));
    } else if (type == "routing-system-status") {
      const auto marker = routeRecoveryStore.load();
      writeRawResponse(id, true, "routing-system-status",
        systemRouteTransactionJson(routeTransactionManager.transaction()) +
        ",\"recoveryMarkerPresent\":" + std::string(marker.has_value() ? "true" : "false") +
        ",\"recoveryOriginalOutputUid\":\"" + escapeJson(marker.has_value() ? marker->originalOutputUid : "") + "\"");
    } else if (type == "per-app-capture-capability") {
      writeRawResponse(id, true, "per-app-capture-capability", perAppCaptureCapabilityResultJson());
    } else if (type == "routing-system-recover") {
      const auto marker = routeRecoveryStore.load();
      if (!marker.has_value()) {
        writeRawResponse(id, false, "routing-system-recover",
          "\"recovered\":false,\"error\":\"NO_RECOVERY_MARKER\"");
        continue;
      }
      bool recovered = false;
#if defined(__APPLE__)
      const auto restore = localmixer::platform::macos::setDefaultOutputUid(marker->originalOutputUid);
      recovered = restore.ok;
#endif
      if (recovered) routeRecoveryStore.clear();
      writeRawResponse(id, recovered, "routing-system-recover",
        "\"recovered\":" + std::string(recovered ? "true" : "false") +
        ",\"error\":\"" + std::string(recovered ? "" : "RECOVERY_APPLY_FAILED") +
        "\",\"recoveryOriginalOutputUid\":\"" + escapeJson(marker->originalOutputUid) + "\"");
    } else if (type == "sync-mixer-graph") {
      const auto fields = syncMixerGraphResultJson(line, graphController, monitorSelection);
      writeRawResponse(id, fields.find("\"synced\":true") != std::string::npos, "sync-mixer-graph", fields);
    } else if (type == "start-mixer-monitor") {
      if (monitorSelection.activeMonitorCount == 0 || monitorSelection.inputUid.empty()) {
        writeRawResponse(id, false, "start-mixer-monitor",
          persistentMonitorStatusJson(false, "NO_MONITOR_SOURCE", 0, 0, 0.0, 0.0, 0.0f));
      } else {
#if defined(__APPLE__)
        const auto effectiveMonitorGainDb =
          monitorSelection.masterEnabled && !monitorSelection.masterMuted
            ? monitorSelection.channelTrimDb + monitorSelection.channelFaderDb +
                monitorSelection.masterTrimDb + monitorSelection.masterFaderDb +
                monitorSelection.monitorGainDb
            : -120.0f;
        const auto status = persistentMonitor.start({
          .inputUid = monitorSelection.inputUid,
          .outputUid = monitorSelection.outputUid,
          .projectSampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0),
          .inputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "inputChannel").value_or(0.0)),
          .outputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "outputChannel").value_or(0.0)),
          .mirrorToAllOutputChannels = readJsonBoolField(line, "mirrorToAllOutputChannels").value_or(true),
          .stereoInput = monitorSelection.channelStereo,
          .durationMs = 0,
          .monitorGainDb = effectiveMonitorGainDb,
          .monitorPan = monitorSelection.channelPan,
          .processors = monitorSelection.processors,
          .fxAProgramId = monitorSelection.fxAProgramId,
          .fxBProgramId = monitorSelection.fxBProgramId,
          .fxA = monitorSelection.fxA,
          .fxB = monitorSelection.fxB,
          .sendA = monitorSelection.sendA,
          .sendB = monitorSelection.sendB,
          .insertFxEnabled = monitorSelection.insertFxEnabled,
          .vocalFxSlots = monitorSelection.vocalFxSlots,
        });
        writeRawResponse(id, status.running, "start-mixer-monitor", persistentMonitorStatusJson(status));
#else
        writeRawResponse(id, false, "start-mixer-monitor",
          persistentMonitorStatusJson(false, "UNSUPPORTED_PLATFORM", 0, 0, 0.0, 0.0, 0.0f));
#endif
      }
    } else if (type == "stop-mixer-monitor") {
#if defined(__APPLE__)
      const auto status = persistentMonitor.stop();
      writeRawResponse(id, true, "stop-mixer-monitor", persistentMonitorStatusJson(status));
#else
      writeRawResponse(id, true, "stop-mixer-monitor",
        persistentMonitorStatusJson(false, "UNSUPPORTED_PLATFORM", 0, 0, 0.0, 0.0, 0.0f));
#endif
    } else if (type == "mixer-monitor-status") {
#if defined(__APPLE__)
      const auto status = persistentMonitor.status();
      writeRawResponse(id, true, "mixer-monitor-status", persistentMonitorStatusJson(status));
#else
      writeRawResponse(id, true, "mixer-monitor-status",
        persistentMonitorStatusJson(false, "UNSUPPORTED_PLATFORM", 0, 0, 0.0, 0.0, 0.0f));
#endif
    } else if (type == "play-test-tone") {
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto durationMs = static_cast<std::uint32_t>(readJsonNumberField(line, "durationMs").value_or(0.0));
      const auto monitorGainDb = static_cast<float>(readJsonNumberField(line, "monitorGainDb").value_or(0.0));
      writeRawResponse(id, true, "play-test-tone",
        testToneResultJson(readJsonStringField(line, "outputUid"), sampleRate, durationMs, monitorGainDb));
    } else if (type == "meter-input") {
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto durationMs = static_cast<std::uint32_t>(readJsonNumberField(line, "durationMs").value_or(500.0));
      writeRawResponse(id, true, "meter-input",
        inputMeterResultJson(readJsonStringField(line, "inputUid"), sampleRate, durationMs));
    } else if (type == "monitor-passthrough") {
      const auto sampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0);
      const auto inputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "inputChannel").value_or(0.0));
      const auto outputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "outputChannel").value_or(0.0));
      const auto mirrorToAll = readJsonBoolField(line, "mirrorToAllOutputChannels").value_or(true);
      const auto durationMs = static_cast<std::uint32_t>(readJsonNumberField(line, "durationMs").value_or(750.0));
      const auto monitorGainDb = static_cast<float>(readJsonNumberField(line, "monitorGainDb").value_or(-24.0));
      writeRawResponse(id, true, "monitor-passthrough",
        monitorPassthroughResultJson(
          readJsonStringField(line, "inputUid"),
          readJsonStringField(line, "outputUid"),
          sampleRate,
          inputChannel,
          outputChannel,
          mirrorToAll,
          durationMs,
          monitorGainDb
        ));
    } else if (type == "shutdown") {
      writeResponse(id, true, "bye");
      return 0;
    } else if (type == "crash-for-test") {
      std::cerr << "engine crash requested for test\n";
      return 70;
    } else {
      writeResponse(id, false, "error", "UNKNOWN_COMMAND");
    }
  }

  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  const std::string command = argc > 1 ? argv[1] : "--version";

  if (command == "--version") {
    printVersion();
    return 0;
  }

  if (command == "--stdio") return runStdioProtocol();

  if (command == "--list-devices") {
    printDevices();
    return 0;
  }

  if (command == "--request-mic-permission") {
    std::cout << "{\"micPermission\":\"" << requestMicrophonePermission() << "\"}" << std::endl;
    return 0;
  }

  if (command == "--test-tone") {
    std::cout << "{" << testToneResultJson(
      argumentValue(argc, argv, "--output-uid"),
      argumentDouble(argc, argv, "--sample-rate", 48000.0),
      argumentUint(argc, argv, "--duration-ms", 250),
      static_cast<float>(argumentDouble(argc, argv, "--monitor-gain-db", 0.0))
    ) << "}" << std::endl;
    return 0;
  }

  if (command == "--meter-input") {
    std::cout << "{" << inputMeterResultJson(
      argumentValue(argc, argv, "--input-uid"),
      argumentDouble(argc, argv, "--sample-rate", 48000.0),
      argumentUint(argc, argv, "--duration-ms", 750)
    ) << "}" << std::endl;
    return 0;
  }

  if (command == "--monitor-passthrough") {
    std::cout << "{" << monitorPassthroughResultJson(
      argumentValue(argc, argv, "--input-uid"),
      argumentValue(argc, argv, "--output-uid"),
      argumentDouble(argc, argv, "--sample-rate", 48000.0),
      argumentUint(argc, argv, "--input-channel", 0),
      argumentUint(argc, argv, "--output-channel", 0),
      !hasArgument(argc, argv, "--single-output-channel"),
      argumentUint(argc, argv, "--duration-ms", 750),
      static_cast<float>(argumentDouble(argc, argv, "--monitor-gain-db", -24.0))
    ) << "}" << std::endl;
    return 0;
  }

  if (command == "--per-app-capture-capability") {
    std::cout << "{" << perAppCaptureCapabilityResultJson() << "}" << std::endl;
    return 0;
  }

  if (command == "--self-test") {
    if (!runSelfTest()) {
      std::cerr << "engine self-test failed\n";
      return 1;
    }
    std::cout << "engine self-test ok\n";
    return 0;
  }

  std::cerr << "usage: local-mixer-engine [--version|--self-test|--stdio|--list-devices|--request-mic-permission|--test-tone|--meter-input|--monitor-passthrough|--per-app-capture-capability]\n";
  return 64;
}
