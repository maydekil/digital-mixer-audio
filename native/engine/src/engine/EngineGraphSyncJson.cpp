#include "engine/EngineGraphSyncJson.hpp"

#include "engine/ChannelProcessorChain.hpp"
#include "engine/JsonProtocol.hpp"
#include "engine/MixerGraph.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace localmixer::engine::protocol {
namespace {

std::string indexedField(std::uint32_t index, const std::string& suffix) {
  return "channel" + std::to_string(index) + suffix;
}

std::string indexedEqField(std::uint32_t channelIndex, std::size_t bandIndex, const std::string& suffix) {
  return indexedField(channelIndex, "Eq" + std::to_string(bandIndex) + suffix);
}

std::string indexedVocalFxField(std::uint32_t index, const std::string& suffix) {
  return "vocalFxSlot" + std::to_string(index) + suffix;
}

std::vector<dsp::fx::RackSlotState> readVocalFxSlots(const std::string& line) {
  std::vector<dsp::fx::RackSlotState> slots;
  const auto count = static_cast<std::uint32_t>(readJsonNumberField(line, "vocalFxSlotCount").value_or(0.0));
  slots.reserve(std::min<std::uint32_t>(count, dsp::fx::kMaxNativeChannelFxSlots));
  for (std::uint32_t index = 0; index < count && slots.size() < dsp::fx::kMaxNativeChannelFxSlots; index += 1) {
    const auto id = readJsonStringField(line, indexedVocalFxField(index, "Id"));
    const auto type = readJsonStringField(line, indexedVocalFxField(index, "Type"));
    if (id.empty() || type.empty()) continue;
    std::vector<std::pair<dsp::fx::ParameterId, float>> parameters;
    for (std::uint32_t parameter = 1; parameter <= 8; parameter += 1) {
      const auto value = readJsonNumberField(line, indexedVocalFxField(index, "Param" + std::to_string(parameter)));
      if (value.has_value()) parameters.push_back({parameter, static_cast<float>(*value)});
    }
    slots.push_back(dsp::fx::RackSlotState{
      .instanceId = id,
      .effectType = type,
      .mix = static_cast<float>(std::clamp(readJsonNumberField(line, indexedVocalFxField(index, "Mix")).value_or(1.0), 0.0, 1.0)),
      .bypassed = !readJsonBoolField(line, indexedVocalFxField(index, "Enabled")).value_or(false),
      .parameters = std::move(parameters),
    });
  }
  return slots;
}

dsp::EqFilterType eqTypeFromUiName(const std::string& name, dsp::EqFilterType fallback) {
  if (name == "High Pass" || name == "HPF") return dsp::EqFilterType::highPass;
  if (name == "Low Pass" || name == "LPF") return dsp::EqFilterType::lowPass;
  if (name == "Peak" || name == "Bell") return dsp::EqFilterType::peaking;
  if (name == "Low Shelf") return dsp::EqFilterType::lowShelf;
  if (name == "High Shelf") return dsp::EqFilterType::highShelf;
  return fallback;
}

void readEqBandFields(const std::string& line, std::uint32_t channelIndex, ChannelProcessorConfig& processors) {
  for (std::size_t bandIndex = 0; bandIndex < processors.eqBands.size(); bandIndex += 1) {
    auto& band = processors.eqBands[bandIndex];
    band.frequencyHz = readJsonNumberField(line, indexedEqField(channelIndex, bandIndex, "FreqHz")).value_or(band.frequencyHz);
    band.gainDb = readJsonNumberField(line, indexedEqField(channelIndex, bandIndex, "GainDb")).value_or(band.gainDb);
    band.q = readJsonNumberField(line, indexedEqField(channelIndex, bandIndex, "Q")).value_or(band.q);
    band.type = eqTypeFromUiName(readJsonStringField(line, indexedEqField(channelIndex, bandIndex, "Type")), band.type);
  }
}

dsp::NoiseMode noiseModeFromUiName(const std::string& name) {
  if (name == "expander") return dsp::NoiseMode::expander;
  return dsp::NoiseMode::gate;
}

std::string syncGraphError(MixerError error, std::uint32_t stripCount, std::uint32_t monitorCount, std::size_t retiredCount) {
  return "\"synced\":false,\"error\":\"" + std::string(mixerErrorName(error)) +
    "\",\"stripCount\":" + std::to_string(stripCount) +
    ",\"activeMonitorCount\":" + std::to_string(monitorCount) +
    ",\"retiredGraphCount\":" + std::to_string(retiredCount);
}

void clearMonitorSelection(SyncedMonitorSelection& monitorSelection) {
  monitorSelection = SyncedMonitorSelection{};
}

}  // namespace

std::string syncMixerGraphResultJson(
  const std::string& line,
  MixerGraphController& controller,
  SyncedMonitorSelection& monitorSelection
) {
  const auto channelCount = static_cast<std::uint32_t>(readJsonNumberField(line, "channelCount").value_or(0.0));
  if (channelCount > kMaxMixerStrips) {
    clearMonitorSelection(monitorSelection);
    return "\"synced\":false,\"error\":\"GRAPH_FULL\",\"stripCount\":0,\"activeMonitorCount\":0,\"retiredGraphCount\":" +
      std::to_string(controller.retiredCount());
  }

  auto prepared = MixerGraph{};
  std::uint32_t stripCount = 0;
  std::uint32_t monitorCount = 0;
  std::string monitorInputUid;
  std::vector<SyncedMonitorSource> monitorSources;
  std::uint32_t fxAProgramId = static_cast<std::uint32_t>(readJsonNumberField(line, "fxAProgramId").value_or(12.0));
  std::uint32_t fxBProgramId = static_cast<std::uint32_t>(readJsonNumberField(line, "fxBProgramId").value_or(50.0));
  FxSendState monitorSendA;
  FxSendState monitorSendB;
  const auto vocalFxSlots = readVocalFxSlots(line);
  bool monitorInsertFxEnabled = false;
  float monitorTrimDb = 0.0f;
  float monitorFaderDb = 0.0f;
  float monitorPan = 0.0f;
  bool monitorStereo = false;
  bool masterEnabled = true;
  bool masterMuted = false;
  float masterTrimDb = 0.0f;
  float masterFaderDb = 0.0f;
  auto monitorProcessors = defaultChannelProcessorConfig();
  const auto outputUid = readJsonStringField(line, "outputUid");
  const auto monitorGainDb = static_cast<float>(readJsonNumberField(line, "monitorGainDb").value_or(-18.0));
  const auto fxA = FxUnitRuntime{
    .enabled = readJsonBoolField(line, "fxAEnabled").value_or(false),
    .returnDb = static_cast<float>(readJsonNumberField(line, "fxAReturnDb").value_or(-12.0)),
  };
  const auto fxB = FxUnitRuntime{
    .enabled = readJsonBoolField(line, "fxBEnabled").value_or(false),
    .returnDb = static_cast<float>(readJsonNumberField(line, "fxBReturnDb").value_or(-12.0)),
  };
  prepared.setFxUnit(FxBusId::a, fxA);
  prepared.setFxUnit(FxBusId::b, fxB);
  for (std::uint32_t index = 0; index < channelCount; index += 1) {
    const auto kind = readJsonStringField(line, indexedField(index, "Kind"));
    const auto name = readJsonStringField(line, indexedField(index, "Name"));
    const auto color = readJsonStringField(line, indexedField(index, "Color"));
    const auto created = prepared.createStrip(name.empty() ? "Channel" : name, color.empty() ? "#18d6e7" : color);
    if (created.error != MixerError::none) {
      clearMonitorSelection(monitorSelection);
      return syncGraphError(created.error, stripCount, monitorCount, controller.retiredCount());
    }

    const auto sourceUid = readJsonStringField(line, indexedField(index, "SourceUid"));
    const auto assignment = readJsonStringField(line, indexedField(index, "Assignment")) == "stereo" ? SourceAssignment::stereo
                                                                                                      : SourceAssignment::mono;
    const auto trimDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "TrimDb")).value_or(0.0));
    const auto faderDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "FaderDb")).value_or(0.0));
    const auto pan = static_cast<float>(readJsonNumberField(line, indexedField(index, "Pan")).value_or(0.0));
    const auto enabled = readJsonBoolField(line, indexedField(index, "Enabled")).value_or(true);
    const auto muted = readJsonBoolField(line, indexedField(index, "Mute")).value_or(false);
    const auto solo = readJsonBoolField(line, indexedField(index, "Solo")).value_or(false);
    const auto monitor = readJsonBoolField(line, indexedField(index, "Monitor")).value_or(false);
    auto processors = defaultChannelProcessorConfig();
    processors.eqEnabled = readJsonBoolField(line, indexedField(index, "ProcessorEq")).value_or(false);
    processors.compressorEnabled = readJsonBoolField(line, indexedField(index, "ProcessorComp")).value_or(false);
    processors.noiseEnabled = readJsonBoolField(line, indexedField(index, "ProcessorNoise")).value_or(false);
    processors.deEsserEnabled = readJsonBoolField(line, indexedField(index, "ProcessorDeEsser")).value_or(false);
    const auto insertFxEnabled = readJsonBoolField(line, indexedField(index, "ProcessorInsertFx")).value_or(false);
    processors.noise.thresholdDb = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "NoiseThresholdDb")).value_or(processors.noise.thresholdDb));
    processors.noise.rangeDb = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "NoiseRangeDb")).value_or(processors.noise.rangeDb));
    processors.noise.holdMs = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "NoiseHoldMs")).value_or(processors.noise.holdMs));
    processors.noise.releaseMs = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "NoiseReleaseMs")).value_or(processors.noise.releaseMs));
    processors.noise.mode = noiseModeFromUiName(readJsonStringField(line, indexedField(index, "NoiseMode")));
    processors.noise.ratio = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "NoiseRatio")).value_or(processors.noise.ratio));
    processors.compressor.thresholdDb = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "CompThresholdDb")).value_or(processors.compressor.thresholdDb));
    processors.compressor.ratio = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "CompRatio")).value_or(processors.compressor.ratio));
    processors.compressor.attackMs = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "CompAttackMs")).value_or(processors.compressor.attackMs));
    processors.compressor.releaseMs = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "CompReleaseMs")).value_or(processors.compressor.releaseMs));
    processors.deEsser.detectorFrequencyHz = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "DeEsserFrequencyHz")).value_or(processors.deEsser.detectorFrequencyHz));
    processors.deEsser.thresholdDb = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "DeEsserThresholdDb")).value_or(processors.deEsser.thresholdDb));
    processors.deEsser.maxReductionDb = static_cast<float>(
      readJsonNumberField(line, indexedField(index, "DeEsserMaxReductionDb")).value_or(processors.deEsser.maxReductionDb));
    readEqBandFields(line, index, processors);

    prepared.setSourceUid(created.id, sourceUid);
    prepared.setAssignment(created.id, assignment, 0, assignment == SourceAssignment::stereo);
    prepared.setLevel(created.id, trimDb, faderDb, pan);
    prepared.setEnabled(created.id, enabled);
    prepared.setMute(created.id, muted);
    prepared.setSolo(created.id, solo);
    prepared.setInputMonitoring(created.id, monitor);
    prepared.setProcessors(created.id, processors);
    const auto sendA = FxSendState{
      .enabled = readJsonBoolField(line, indexedField(index, "SendAEnabled")).value_or(false),
      .gainDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "SendAGainDb")).value_or(-90.0)),
    };
    const auto sendB = FxSendState{
      .enabled = readJsonBoolField(line, indexedField(index, "SendBEnabled")).value_or(false),
      .gainDb = static_cast<float>(readJsonNumberField(line, indexedField(index, "SendBGainDb")).value_or(-90.0)),
    };
    prepared.setFxSend(created.id, FxBusId::a, sendA);
    prepared.setFxSend(created.id, FxBusId::b, sendB);
    stripCount += 1;
    if (kind == "master") {
      masterEnabled = enabled;
      masterMuted = muted;
      masterTrimDb = trimDb;
      masterFaderDb = faderDb;
    }
    if (monitor && enabled && !muted && !sourceUid.empty() && kind == "source") {
      monitorCount += 1;
      if (monitorInputUid.empty()) monitorInputUid = sourceUid;
      monitorSources.push_back(SyncedMonitorSource{
        .inputUid = sourceUid,
        .sendA = sendA,
        .sendB = sendB,
        .insertFxEnabled = insertFxEnabled,
        .vocalFxSlots = insertFxEnabled ? vocalFxSlots : std::vector<dsp::fx::RackSlotState>{},
        .channelTrimDb = trimDb,
        .channelFaderDb = faderDb,
        .channelPan = pan,
        .channelStereo = assignment == SourceAssignment::stereo,
        .processors = processors,
      });
      monitorSendA = sendA;
      monitorSendB = sendB;
      monitorInsertFxEnabled = insertFxEnabled;
      monitorTrimDb = trimDb;
      monitorFaderDb = faderDb;
      monitorPan = pan;
      monitorStereo = assignment == SourceAssignment::stereo;
      monitorProcessors = processors;
    }
  }

  controller.publish(std::move(prepared));
  controller.reclaimRetired();
  monitorSelection.inputUid = monitorInputUid;
  monitorSelection.outputUid = outputUid;
  monitorSelection.activeMonitorCount = monitorCount;
  monitorSelection.fxAProgramId = fxAProgramId;
  monitorSelection.fxBProgramId = fxBProgramId;
  monitorSelection.fxA = fxA;
  monitorSelection.fxB = fxB;
  monitorSelection.sendA = monitorSendA;
  monitorSelection.sendB = monitorSendB;
  monitorSelection.insertFxEnabled = monitorInsertFxEnabled;
  monitorSelection.vocalFxSlots = monitorInsertFxEnabled ? vocalFxSlots : std::vector<dsp::fx::RackSlotState>{};
  monitorSelection.monitorGainDb = monitorGainDb;
  monitorSelection.channelTrimDb = monitorTrimDb;
  monitorSelection.channelFaderDb = monitorFaderDb;
  monitorSelection.channelPan = monitorPan;
  monitorSelection.channelStereo = monitorStereo;
  monitorSelection.sources = std::move(monitorSources);
  monitorSelection.masterEnabled = masterEnabled;
  monitorSelection.masterMuted = masterMuted;
  monitorSelection.masterTrimDb = masterTrimDb;
  monitorSelection.masterFaderDb = masterFaderDb;
  monitorSelection.processors = monitorProcessors;
  return "\"synced\":true,\"error\":\"\",\"stripCount\":" + std::to_string(stripCount) +
    ",\"activeMonitorCount\":" + std::to_string(monitorCount) +
    ",\"retiredGraphCount\":" + std::to_string(controller.retiredCount());
}

}  // namespace localmixer::engine::protocol
