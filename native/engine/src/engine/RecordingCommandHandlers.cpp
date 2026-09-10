#include "engine/RecordingCommandHandlers.hpp"

#include "engine/JsonProtocol.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

namespace localmixer::engine {
namespace {

using protocol::escapeJson;
using protocol::readJsonNumberField;
using protocol::readJsonStringField;

RecordingTap readRecordingTap(const std::string& line) {
  const auto tap = readJsonStringField(line, "tap");
  if (tap == "dry") return RecordingTap::dry;
  if (tap == "processed") return RecordingTap::processed;
  return RecordingTap::master;
}

std::string recordingMetadataFields(const RecordedTakeMetadata& metadata) {
  return std::string("\"takeId\":\"") + escapeJson(metadata.path.stem().string()) +
    "\",\"path\":\"" + escapeJson(metadata.path.string()) +
    "\",\"tap\":\"" + std::string(recordingTapName(metadata.tap)) +
    "\",\"sampleRate\":" + std::to_string(metadata.sampleRate) +
    ",\"channels\":" + std::to_string(metadata.channels) +
    ",\"frames\":" + std::to_string(metadata.frames) +
    ",\"replayWithNeutralInserts\":" + std::string(metadata.replayWithNeutralInserts ? "true" : "false") +
    ",\"partial\":" + std::string(metadata.partial ? "true" : "false");
}

std::string recordingInactiveFields(const std::string& error) {
  return "\"saved\":false,\"error\":\"" + escapeJson(error) +
    "\",\"takeId\":\"\",\"path\":\"\",\"tap\":\"master\",\"sampleRate\":48000,\"channels\":2,\"frames\":0,"
    "\"replayWithNeutralInserts\":true,\"partial\":true";
}

}  // namespace

std::string recordingPlanJsonFields(const std::string& line) {
  const auto directory = readJsonStringField(line, "directory");
  const auto baseName = readJsonStringField(line, "baseName");
  const auto sampleRate = static_cast<std::uint32_t>(readJsonNumberField(line, "sampleRate").value_or(48000.0));
  const auto channels = static_cast<std::uint16_t>(readJsonNumberField(line, "channels").value_or(2.0));
  const auto armedChannelCount = static_cast<std::uint32_t>(readJsonNumberField(line, "armedChannelCount").value_or(0.0));
  const auto tap = readRecordingTap(line);
  const bool valid = !directory.empty() && sampleRate > 0 && channels > 0 && armedChannelCount > 0;
  const auto path = valid
    ? makeCollisionSafeTakePath(directory, baseName.empty() ? "take" : baseName, ".wav")
    : std::filesystem::path();
  const auto tapName = std::string(recordingTapName(tap));
  const std::string takeId = path.empty() ? "" : path.stem().string();
  const std::string error = directory.empty()
    ? "INVALID_RECORDING_DIRECTORY"
    : (armedChannelCount == 0 ? "NO_ARMED_CHANNELS" : "");

  return "\"planned\":" + std::string(valid ? "true" : "false") +
    ",\"error\":\"" + escapeJson(error) + "\"" +
    ",\"takeId\":\"" + escapeJson(takeId) + "\"" +
    ",\"path\":\"" + escapeJson(path.string()) + "\"" +
    ",\"tap\":\"" + tapName + "\"" +
    ",\"sampleRate\":" + std::to_string(sampleRate) +
    ",\"channels\":" + std::to_string(channels) +
    ",\"frames\":0" +
    ",\"replayWithNeutralInserts\":" + std::string(tap == RecordingTap::dry ? "false" : "true") +
    ",\"partial\":false" +
    ",\"armedChannelCount\":" + std::to_string(armedChannelCount);
}

std::string recordingStartJsonFields(const std::string& line, RecordingSession& session) {
  const auto directory = readJsonStringField(line, "directory");
  const auto baseName = readJsonStringField(line, "baseName");
  const auto sampleRate = static_cast<std::uint32_t>(readJsonNumberField(line, "sampleRate").value_or(48000.0));
  const auto channels = static_cast<std::uint16_t>(readJsonNumberField(line, "channels").value_or(2.0));
  const auto armedChannelCount = static_cast<std::uint32_t>(readJsonNumberField(line, "armedChannelCount").value_or(0.0));
  const auto tap = readRecordingTap(line);
  const bool valid = !directory.empty() && sampleRate > 0 && channels > 0 && armedChannelCount > 0;

  if (!valid || !session.arm(RecordingConfig{
        .directory = directory,
        .baseName = baseName.empty() ? "take" : baseName,
        .sampleRate = sampleRate,
        .channels = channels,
        .tap = tap,
      }) || !session.start()) {
    const std::string error = directory.empty()
      ? "INVALID_RECORDING_DIRECTORY"
      : (armedChannelCount == 0 ? "NO_ARMED_CHANNELS" : "RECORDING_START_FAILED");
    return "\"started\":false,\"error\":\"" + escapeJson(error) +
      "\",\"path\":\"\",\"takeId\":\"\",\"tap\":\"" + std::string(recordingTapName(tap)) +
      "\",\"sampleRate\":" + std::to_string(sampleRate) +
      ",\"channels\":" + std::to_string(channels) +
      ",\"frames\":0,\"replayWithNeutralInserts\":" +
      std::string(tap == RecordingTap::dry ? "false" : "true") +
      ",\"partial\":false";
  }

  return "\"started\":true,\"error\":\"\"," + recordingMetadataFields(session.metadata());
}

std::string recordingStopJsonFields(RecordingSession& session) {
  if (!session.stop()) return recordingInactiveFields("RECORDING_NOT_ACTIVE");
  return "\"saved\":true,\"error\":\"\"," + recordingMetadataFields(session.metadata());
}

}  // namespace localmixer::engine
