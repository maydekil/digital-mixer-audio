#include "dsp/Gain.hpp"
#include "dsp/OutputProtection.hpp"
#include "engine/EngineRuntime.hpp"
#include "engine/MixerGraph.hpp"
#include "engine/MixerGraphController.hpp"
#include "engine/SystemRouting.hpp"
#if defined(__APPLE__)
#include "platform/macos/CoreAudioDevices.hpp"
#include "platform/macos/CoreAudioInputMeter.hpp"
#include "platform/macos/CoreAudioOutputStream.hpp"
#include "platform/macos/CoreAudioPassthrough.hpp"
#endif

#include <array>
#include <cmath>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace {

constexpr int kProtocolVersion = 1;
constexpr std::size_t kMaxMessageBytes = 8192;

struct SyncedMonitorSelection {
  std::string inputUid;
  std::string outputUid;
  std::uint32_t activeMonitorCount = 0;
  float monitorGainDb = -18.0f;
  float channelTrimDb = 0.0f;
  float channelFaderDb = 0.0f;
  float channelPan = 0.0f;
};

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

std::string escapeJson(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    if (c == '"' || c == '\\') out.push_back('\\');
    if (c >= 0 && c < 0x20) continue;
    out.push_back(c);
  }
  return out;
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

std::string systemRouteDiagnosticsJson(
  std::span<const localmixer::engine::DeviceDescriptor> devices,
  const std::string& blackHoleUid,
  const std::string& outputUid,
  double sampleRate,
  std::uint32_t inputStartChannel,
  std::uint32_t outputStartChannel
) {
  const auto diagnostics = localmixer::engine::validateSystemRoute(
    devices,
    localmixer::engine::ProjectAudioConfig{.sampleRate = sampleRate, .blockSize = 256},
    localmixer::engine::SystemRouteSelection{
      .blackHoleUid = blackHoleUid,
      .physicalOutputUid = outputUid,
      .blackHoleInputStartChannel = inputStartChannel,
      .physicalOutputStartChannel = outputStartChannel,
    }
  );
  std::string json = "\"routeValid\":" + std::string(diagnostics.routeValid ? "true" : "false");
  json += ",\"blackHoleAvailable\":" + std::string(diagnostics.blackHoleAvailable ? "true" : "false");
  json += ",\"error\":\"" + std::string(localmixer::engine::systemRouteErrorName(diagnostics.error)) + "\"";
  json += ",\"blackHoleUid\":\"" + escapeJson(diagnostics.selection.blackHoleUid) + "\"";
  json += ",\"physicalOutputUid\":\"" + escapeJson(diagnostics.selection.physicalOutputUid) + "\"";
  json += ",\"selectedInputStart\":" + std::to_string(diagnostics.selectedInputStart);
  json += ",\"selectedInputEnd\":" + std::to_string(diagnostics.selectedInputEnd);
  json += ",\"selectedOutputStart\":" + std::to_string(diagnostics.selectedOutputStart);
  json += ",\"selectedOutputEnd\":" + std::to_string(diagnostics.selectedOutputEnd);
  json += ",\"blackHoleSampleRate\":" + std::to_string(diagnostics.blackHoleSampleRate);
  json += ",\"outputSampleRate\":" + std::to_string(diagnostics.outputSampleRate);
  return json;
}

std::string devicesJson(std::span<const localmixer::engine::DeviceDescriptor> devices) {
  std::string json = "[";
  for (std::size_t index = 0; index < devices.size(); index += 1) {
    const auto& device = devices[index];
    if (index > 0) json += ",";
    json += "{\"uid\":\"" + escapeJson(device.uid) + "\"";
    json += ",\"name\":\"" + escapeJson(device.name) + "\"";
    json += ",\"defaultInput\":" + std::string(device.isDefaultInput ? "true" : "false");
    json += ",\"defaultOutput\":" + std::string(device.isDefaultOutput ? "true" : "false");
    json += ",\"inputChannels\":" + std::to_string(device.inputChannels);
    json += ",\"outputChannels\":" + std::to_string(device.outputChannels);
    json += ",\"sampleRate\":" + std::to_string(device.sampleRate);
    json += ",\"maxBlockSize\":" + std::to_string(device.maxBlockSize);
    json += "}";
  }
  json += "]";
  return json;
}

std::string statusJson(const localmixer::engine::RuntimeStatus& status) {
  std::string json = "{\"state\":\"";
  json += localmixer::engine::runtimeStateName(status.state);
  json += "\",\"deviceCount\":" + std::to_string(status.deviceCount);
  json += ",\"monitoringEnabled\":" + std::string(status.monitoringEnabled ? "true" : "false");
  json += ",\"inputUid\":\"" + escapeJson(status.inputUid) + "\"";
  json += ",\"outputUid\":\"" + escapeJson(status.outputUid) + "\"";
  if (!status.error.empty()) json += ",\"error\":\"" + escapeJson(status.error) + "\"";
  json += "}";
  return json;
}

void printDevices() {
  const auto devices = loadNativeDevices();
  std::cout << "{\"devices\":[";
  for (std::size_t index = 0; index < devices.size(); index += 1) {
    const auto& device = devices[index];
    if (index > 0) std::cout << ",";
    std::cout
      << "{\"uid\":\"" << escapeJson(device.uid) << "\""
      << ",\"name\":\"" << escapeJson(device.name) << "\""
      << ",\"defaultInput\":" << (device.isDefaultInput ? "true" : "false")
      << ",\"defaultOutput\":" << (device.isDefaultOutput ? "true" : "false")
      << ",\"inputChannels\":" << device.inputChannels
      << ",\"outputChannels\":" << device.outputChannels
      << ",\"sampleRate\":" << device.sampleRate
      << ",\"maxBlockSize\":" << device.maxBlockSize
      << "}";
  }
  std::cout << "],\"micPermission\":\"" << microphonePermissionState() << "\"}" << std::endl;
}

std::string readJsonStringField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return "";
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return "";
  const auto start = line.find('"', colon + 1);
  if (start == std::string::npos) return "";
  const auto end = line.find('"', start + 1);
  if (end == std::string::npos) return "";
  return line.substr(start + 1, end - start - 1);
}

std::optional<double> readJsonNumberField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return std::nullopt;
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return std::nullopt;
  const auto start = line.find_first_of("-0123456789", colon + 1);
  if (start == std::string::npos) return std::nullopt;
  const auto end = line.find_first_not_of("-0123456789.", start);
  try {
    return std::stod(line.substr(start, end - start));
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<bool> readJsonBoolField(const std::string& line, const std::string& field) {
  const std::string key = "\"" + field + "\"";
  const auto keyPos = line.find(key);
  if (keyPos == std::string::npos) return std::nullopt;
  const auto colon = line.find(':', keyPos + key.size());
  if (colon == std::string::npos) return std::nullopt;
  if (line.find("true", colon + 1) == colon + 1) return true;
  if (line.find("false", colon + 1) == colon + 1) return false;
  return std::nullopt;
}

std::string indexedField(std::uint32_t index, const std::string& suffix) {
  return "channel" + std::to_string(index) + suffix;
}

std::string persistentMonitorStatusJson(
  bool running,
  const std::string& error,
  std::uint32_t inputChannels,
  std::uint32_t outputChannels,
  double inputSampleRate,
  double outputSampleRate,
  float inputPeak
) {
  std::string json = "\"monitoring\":" + std::string(running ? "true" : "false");
  json += ",\"error\":\"" + escapeJson(error) + "\"";
  json += ",\"inputChannels\":" + std::to_string(inputChannels);
  json += ",\"outputChannels\":" + std::to_string(outputChannels);
  json += ",\"inputSampleRate\":" + std::to_string(inputSampleRate);
  json += ",\"outputSampleRate\":" + std::to_string(outputSampleRate);
  json += ",\"inputPeak\":" + std::to_string(inputPeak);
  return json;
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
    status.inputPeak
  );
}
#endif

std::string syncMixerGraphResultJson(
  const std::string& line,
  localmixer::engine::MixerGraphController& controller,
  SyncedMonitorSelection& monitorSelection
) {
  const auto channelCount = static_cast<std::uint32_t>(readJsonNumberField(line, "channelCount").value_or(0.0));
  if (channelCount > localmixer::engine::kMaxMixerStrips) {
    return "\"synced\":false,\"error\":\"GRAPH_FULL\",\"stripCount\":0,\"activeMonitorCount\":0,\"retiredGraphCount\":" +
      std::to_string(controller.retiredCount());
  }

  auto prepared = localmixer::engine::MixerGraph{};
  std::uint32_t stripCount = 0;
  std::uint32_t monitorCount = 0;
  std::string monitorInputUid;
  float monitorTrimDb = 0.0f;
  float monitorFaderDb = 0.0f;
  float monitorPan = 0.0f;
  const auto outputUid = readJsonStringField(line, "outputUid");
  const auto monitorGainDb = static_cast<float>(readJsonNumberField(line, "monitorGainDb").value_or(-18.0));
  for (std::uint32_t index = 0; index < channelCount; index += 1) {
    const auto kind = readJsonStringField(line, indexedField(index, "Kind"));
    const auto name = readJsonStringField(line, indexedField(index, "Name"));
    const auto color = readJsonStringField(line, indexedField(index, "Color"));
    const auto created = prepared.createStrip(name.empty() ? "Channel" : name, color.empty() ? "#18d6e7" : color);
    if (created.error != localmixer::engine::MixerError::none) {
      return "\"synced\":false,\"error\":\"" + std::string(localmixer::engine::mixerErrorName(created.error)) +
        "\",\"stripCount\":" + std::to_string(stripCount) +
        ",\"activeMonitorCount\":" + std::to_string(monitorCount) +
        ",\"retiredGraphCount\":" + std::to_string(controller.retiredCount());
    }

    const auto sourceUid = readJsonStringField(line, indexedField(index, "SourceUid"));
    const auto assignment = readJsonStringField(line, indexedField(index, "Assignment")) == "stereo"
      ? localmixer::engine::SourceAssignment::stereo
      : localmixer::engine::SourceAssignment::mono;
    const auto trimDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "TrimDb")).value_or(0.0));
    const auto faderDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "FaderDb")).value_or(0.0));
    const auto pan = static_cast<float>(readJsonNumberField(line, indexedField(index, "Pan")).value_or(0.0));
    const auto enabled = readJsonBoolField(line, indexedField(index, "Enabled")).value_or(true);
    const auto muted = readJsonBoolField(line, indexedField(index, "Mute")).value_or(false);
    const auto solo = readJsonBoolField(line, indexedField(index, "Solo")).value_or(false);
    const auto monitor = readJsonBoolField(line, indexedField(index, "Monitor")).value_or(false);

    prepared.setSourceUid(created.id, sourceUid);
    prepared.setAssignment(created.id, assignment, 0, assignment == localmixer::engine::SourceAssignment::stereo);
    prepared.setLevel(created.id, trimDb, faderDb, pan);
    prepared.setEnabled(created.id, enabled);
    prepared.setMute(created.id, muted);
    prepared.setSolo(created.id, solo);
    prepared.setInputMonitoring(created.id, monitor);
    stripCount += 1;
    if (monitor && enabled && !sourceUid.empty() && kind == "source") {
      monitorCount += 1;
      if (monitorInputUid.empty()) monitorInputUid = sourceUid;
      monitorTrimDb = trimDb;
      monitorFaderDb = faderDb;
      monitorPan = pan;
    }
  }

  if (monitorCount > 1) {
    return "\"synced\":false,\"error\":\"MULTIPLE_MONITOR_SOURCES_UNSUPPORTED\",\"stripCount\":" +
      std::to_string(stripCount) +
      ",\"activeMonitorCount\":" + std::to_string(monitorCount) +
      ",\"retiredGraphCount\":" + std::to_string(controller.retiredCount());
  }

  controller.publish(std::move(prepared));
  controller.reclaimRetired();
  monitorSelection.inputUid = monitorInputUid;
  monitorSelection.outputUid = outputUid;
  monitorSelection.activeMonitorCount = monitorCount;
  monitorSelection.monitorGainDb = monitorGainDb;
  monitorSelection.channelTrimDb = monitorTrimDb;
  monitorSelection.channelFaderDb = monitorFaderDb;
  monitorSelection.channelPan = monitorPan;
  return "\"synced\":true,\"error\":\"\",\"stripCount\":" + std::to_string(stripCount) +
    ",\"activeMonitorCount\":" + std::to_string(monitorCount) +
    ",\"retiredGraphCount\":" + std::to_string(controller.retiredCount());
}

void writeResponse(const std::string& id, bool ok, const std::string& type, const std::string& error = "") {
  std::cout << "{\"id\":\"" << id << "\",\"type\":\"" << type << "\",\"ok\":" << (ok ? "true" : "false");
  if (!error.empty()) std::cout << ",\"error\":\"" << error << "\"";
  std::cout << "}" << std::endl;
}

void writeRawResponse(const std::string& id, bool ok, const std::string& type, const std::string& fields) {
  std::cout << "{\"id\":\"" << id << "\",\"type\":\"" << type << "\",\"ok\":" << (ok ? "true" : "false");
  if (!fields.empty()) std::cout << "," << fields;
  std::cout << "}" << std::endl;
}

int runStdioProtocol() {
  localmixer::engine::EngineRuntime runtime;
  localmixer::engine::MixerGraphController graphController;
  SyncedMonitorSelection monitorSelection;
#if defined(__APPLE__)
  localmixer::platform::macos::PersistentPassthroughMonitor persistentMonitor;
#endif
  runtime.refreshDevices(loadNativeDevices());

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
    } else if (type == "sync-mixer-graph") {
      const auto fields = syncMixerGraphResultJson(line, graphController, monitorSelection);
      writeRawResponse(id, fields.find("\"synced\":true") != std::string::npos, "sync-mixer-graph", fields);
    } else if (type == "start-mixer-monitor") {
      if (monitorSelection.activeMonitorCount == 0 || monitorSelection.inputUid.empty()) {
        writeRawResponse(id, false, "start-mixer-monitor",
          persistentMonitorStatusJson(false, "NO_MONITOR_SOURCE", 0, 0, 0.0, 0.0, 0.0f));
      } else {
#if defined(__APPLE__)
        const auto status = persistentMonitor.start({
          .inputUid = monitorSelection.inputUid,
          .outputUid = monitorSelection.outputUid,
          .projectSampleRate = readJsonNumberField(line, "sampleRate").value_or(48000.0),
          .inputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "inputChannel").value_or(0.0)),
          .outputChannel = static_cast<std::uint32_t>(readJsonNumberField(line, "outputChannel").value_or(0.0)),
          .mirrorToAllOutputChannels = readJsonBoolField(line, "mirrorToAllOutputChannels").value_or(true),
          .durationMs = 0,
          .monitorGainDb = monitorSelection.channelTrimDb + monitorSelection.channelFaderDb + monitorSelection.monitorGainDb,
          .monitorPan = monitorSelection.channelPan,
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

  if (command == "--self-test") {
    if (!runSelfTest()) {
      std::cerr << "engine self-test failed\n";
      return 1;
    }
    std::cout << "engine self-test ok\n";
    return 0;
  }

  std::cerr << "usage: local-mixer-engine [--version|--self-test|--stdio|--list-devices|--request-mic-permission|--test-tone|--meter-input|--monitor-passthrough]\n";
  return 64;
}
