#include "engine/SessionDocument.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

using localmixer::engine::SessionDocument;
using localmixer::engine::SessionError;
using localmixer::engine::SessionChannelHarmonyState;
using localmixer::engine::SessionFxSendAssignment;
using localmixer::engine::SessionFxUnitState;
using localmixer::engine::SessionMediaRef;
using localmixer::engine::SessionPluginState;
using localmixer::engine::SessionRecordedTake;
using localmixer::engine::autosavePathFor;
using localmixer::engine::loadSession;
using localmixer::engine::parseSession;
using localmixer::engine::recoveryCandidateFor;
using localmixer::engine::saveSessionAtomic;

}  // namespace

int main() {
  const auto directory = std::filesystem::temp_directory_path() / "local-mixer-session-test";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory);
  const auto sessionPath = directory / "project.lam.json";

  SessionDocument document;
  document.projectId = "project-a";
  document.media.push_back(SessionMediaRef{.id = "music", .path = "Media/music.wav"});
  document.media.push_back(SessionMediaRef{.id = "missing", .path = "/missing/file.wav", .missing = true});
  document.fxUnits.push_back(SessionFxUnitState{
    .unitId = "fx-a",
    .programId = 12,
    .processorType = "reverb",
    .revision = 7,
    .enabled = true,
    .modified = true,
    .returnDb = -6.0f,
    .macro1Value = "2.2 s",
    .macro2Value = "20 ms",
  });
  document.fxSends.push_back(SessionFxSendAssignment{.channelId = "voice", .unitId = "fx-a", .enabled = true, .gainDb = -18.0f});
  document.channelHarmony.push_back(SessionChannelHarmonyState{
    .channelId = "voice",
    .contentRole = "vocal",
    .primaryHarmonyInstanceId = "harmony:voice:primary",
    .harmonyEnabled = true,
    .key = "C",
    .scale = "Major",
    .mode = "Diatonic",
    .voice1 = "+3rd",
    .voice2 = "+5th",
    .harmonyLevelDb = 0.0f,
  });
  document.plugins.push_back(SessionPluginState{
    .instanceId = "plugin-1",
    .identifier = "com.example.TestPlugin",
    .version = "1.0.0",
    .missing = false,
    .stateBase64 = "AQID",
  });
  document.recordedTakes.push_back(SessionRecordedTake{
    .id = "take-voice-001",
    .path = "Recordings/voice-001.wav",
    .tap = "processed",
    .sampleRate = 48000,
    .channels = 2,
    .frames = 96000,
    .replayWithNeutralInserts = true,
    .partial = false,
  });
  if (!saveSessionAtomic(sessionPath, document)) {
    std::cerr << "session save should succeed\n";
    return 1;
  }

  const auto loaded = loadSession(sessionPath);
  if (loaded.error != SessionError::none || loaded.document.projectId != "project-a" || loaded.document.media.size() != 2 ||
      !loaded.document.media[1].missing || loaded.document.fxUnits.size() != 1 || loaded.document.fxSends.size() != 1 ||
      loaded.document.fxUnits[0].macro1Value != "2.2 s" || loaded.document.fxSends[0].gainDb != -18.0f ||
      loaded.document.channelHarmony.size() != 1 ||
      loaded.document.channelHarmony[0].primaryHarmonyInstanceId != "harmony:voice:primary" ||
      loaded.document.plugins.size() != 1 || loaded.document.plugins[0].stateBase64 != "AQID" ||
      loaded.document.recordedTakes.size() != 1 ||
      loaded.document.recordedTakes[0].path != std::filesystem::path("Recordings/voice-001.wav") ||
      loaded.document.recordedTakes[0].tap != "processed" ||
      loaded.document.recordedTakes[0].frames != 96000 ||
      !loaded.document.recordedTakes[0].replayWithNeutralInserts ||
      loaded.document.recordedTakes[0].partial) {
    std::cerr << "session load roundtrip mismatch\n";
    return 1;
  }

  const auto legacy = parseSession("{\"schemaVersion\":1,\"projectId\":\"legacy\",\"media\":[]}");
  if (legacy.error != SessionError::none || !legacy.document.fxUnits.empty() || !legacy.document.fxSends.empty() ||
      !legacy.document.channelHarmony.empty() || !legacy.document.plugins.empty() ||
      !legacy.document.recordedTakes.empty()) {
    std::cerr << "legacy session without FX state should load with empty FX vectors\n";
    return 1;
  }

  document.projectId = "project-b";
  if (!saveSessionAtomic(sessionPath, document) || !std::filesystem::exists(sessionPath.string() + ".bak")) {
    std::cerr << "second save should create previous-good backup\n";
    return 1;
  }

  const auto newer = parseSession("{\"schemaVersion\":999,\"projectId\":\"future\",\"media\":[]}");
  if (newer.error != SessionError::newerSchema) {
    std::cerr << "newer schema should be rejected\n";
    return 1;
  }

  {
    std::ofstream corrupt(sessionPath, std::ios::trunc);
    corrupt << "{\"schemaVersion\":";
  }
  if (loadSession(sessionPath).error != SessionError::invalidDocument) {
    std::cerr << "corrupt session should not load as valid\n";
    return 1;
  }
  if (loadSession(sessionPath.string() + ".bak").error != SessionError::none) {
    std::cerr << "previous-good backup should remain loadable after corrupt current save\n";
    return 1;
  }

  const auto autosavePath = autosavePathFor(sessionPath);
  {
    std::ofstream autosave(autosavePath, std::ios::trunc);
    autosave << "{\"schemaVersion\":1,\"projectId\":\"autosave\",\"media\":[]}";
  }
  const auto recovery = recoveryCandidateFor(sessionPath);
  if (!recovery.has_value() || *recovery != autosavePath) {
    std::cerr << "autosave recovery candidate should be offered without overwriting original\n";
    return 1;
  }

  std::filesystem::remove_all(directory);
  std::cout << "local-mixer-session-document-tests ok\n";
  return 0;
}
