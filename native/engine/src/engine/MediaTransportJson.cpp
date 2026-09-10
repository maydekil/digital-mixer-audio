#include "engine/MediaTransportJson.hpp"

#include "engine/JsonProtocol.hpp"
#include "engine/MediaFile.hpp"

namespace localmixer::engine::protocol {

std::string mediaInfoJson(const std::string& path) {
  if (path.empty()) return "\"imported\":false,\"error\":\"MISSING_PATH\"";
  localmixer::engine::WavStreamReader reader;
  const auto error = reader.open(path);
  if (error != localmixer::engine::MediaFileError::none) {
    return "\"imported\":false,\"error\":\"" + std::string(localmixer::engine::mediaFileErrorName(error)) + "\"";
  }
  const auto& info = reader.info();
  const auto duration = info.sampleRate == 0 ? 0.0 : static_cast<double>(info.frameCount) / info.sampleRate;
  return "\"imported\":true,\"error\":\"\",\"container\":\"wav\",\"path\":\"" + escapeJson(path) +
    "\",\"channels\":" + std::to_string(info.channels) +
    ",\"sampleRate\":" + std::to_string(info.sampleRate) +
    ",\"bitsPerSample\":" + std::to_string(info.bitsPerSample) +
    ",\"frameCount\":" + std::to_string(info.frameCount) +
    ",\"durationSeconds\":" + std::to_string(duration);
}

std::string mediaImportStatusJson(const localmixer::engine::MediaImportStatus& status) {
  return "\"jobId\":\"" + escapeJson(status.jobId) +
    "\",\"state\":\"" + std::string(localmixer::engine::mediaImportStateName(status.state)) +
    "\",\"error\":\"" + std::string(localmixer::engine::mediaFileErrorName(status.error)) +
    "\",\"channels\":" + std::to_string(status.info.channels) +
    ",\"sampleRate\":" + std::to_string(status.info.sampleRate) +
    ",\"bitsPerSample\":" + std::to_string(status.info.bitsPerSample) +
    ",\"processedFrames\":" + std::to_string(status.processedFrames) +
    ",\"totalFrames\":" + std::to_string(status.totalFrames) +
    ",\"waveformPoints\":" + std::to_string(status.waveformPoints) +
    ",\"progress\":" + std::to_string(status.progress);
}

std::string transportJson(const localmixer::engine::TransportSnapshot& snapshot) {
  return "\"state\":\"" + std::string(localmixer::engine::transportStateName(snapshot.state)) +
    "\",\"positionFrame\":" + std::to_string(snapshot.positionFrame) +
    ",\"startFrame\":" + std::to_string(snapshot.startFrame) +
    ",\"bufferGeneration\":" + std::to_string(snapshot.bufferGeneration) +
    ",\"loopEnabled\":" + std::string(snapshot.loop.enabled ? "true" : "false") +
    ",\"loopStartFrame\":" + std::to_string(snapshot.loop.startFrame) +
    ",\"loopEndFrame\":" + std::to_string(snapshot.loop.endFrame);
}

}  // namespace localmixer::engine::protocol
