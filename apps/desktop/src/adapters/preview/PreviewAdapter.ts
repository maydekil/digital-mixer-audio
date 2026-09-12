import type { ChannelDynamicsState, ChannelRole, ChannelState, EqBandState, FxProgram, FxUnitId, HarmonyState, MixerControlPort, MixerSnapshot, ProcessorId, RecordedTakeState, RecordingWorkflowStatus, VocalFxParameter } from "../MixerControlPort";
import { approvedMixerSession } from "../../fixtures/approvedMixerSession";
import { vocalFxPresetEffectTypes } from "../../fixtures/vocalFxPresets";

export class PreviewAdapter implements MixerControlPort {
  private snapshot: MixerSnapshot = structuredClone(approvedMixerSession);

  getSnapshot(): MixerSnapshot {
    return this.snapshot;
  }

  replaceSnapshot(snapshot: MixerSnapshot): void {
    this.snapshot = structuredClone(snapshot);
    this.syncSelectedEqBands();
  }

  setPrograms(programs: FxProgram[]): void {
    if (programs.length === 0) return;
    const programIds = new Set(programs.map((program) => program.id));
    this.snapshot.programs = structuredClone(programs);
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => (
      programIds.has(unit.programId) ? unit : { ...unit, programId: programs[0].id, modified: false }
    ));
  }

  addSourceChannel(role: Exclude<ChannelRole, "group" | "master">, name: string, source: string): string {
    const id = uniqueChannelId(this.snapshot.channels, slugChannelId(name || role));
    const channel = newSourceChannel(id, name.trim() || defaultChannelName(role), role, source.trim());
    const insertAt = this.snapshot.channels.findIndex((item) => item.kind !== "source");
    const nextChannels = [...this.snapshot.channels];
    nextChannels.splice(insertAt < 0 ? nextChannels.length : insertAt, 0, channel);
    this.snapshot.channels = nextChannels;
    this.selectChannel(id);
    return id;
  }

  renameChannel(channelId: string, name: string): void {
    const nextName = name.trim();
    if (!nextName) return;
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, name: nextName } : channel);
  }

  removeChannel(channelId: string): void {
    const target = this.snapshot.channels.find((channel) => channel.id === channelId);
    if (!target || target.kind === "master") return;
    const channels = this.snapshot.channels.filter((channel) => channel.id !== channelId);
    this.snapshot.channels = channels;
    if (this.snapshot.selectedChannelId === channelId) {
      const fallback = channels.find((channel) => channel.kind === "source") ?? channels[0];
      if (fallback) this.selectChannel(fallback.id);
    }
  }

  selectChannel(channelId: string): void {
    if (!this.snapshot.channels.some((channel) => channel.id === channelId)) return;
    this.snapshot.selectedChannelId = channelId;
    this.snapshot.channels = this.snapshot.channels.map((channel) => ({ ...channel, selected: channel.id === channelId }));
    this.syncSelectedEqBands();
  }

  setChannelEnabled(channelId: string, enabled: boolean): void {
    const target = this.snapshot.channels.find((channel) => channel.id === channelId);
    this.snapshot.channels = this.snapshot.channels.map((channel) => {
      if (channel.id === channelId) {
        return {
          ...channel,
          enabled,
          monitor: channel.kind === "source" ? enabled : channel.monitor,
          meter: enabled ? channel.meter : { left: -60, right: -60, clip: false }
        };
      }
      if (enabled && target?.kind === "source" && channel.kind === "source") return { ...channel, monitor: false };
      return channel;
    });
  }

  setChannelSource(channelId: string, source: string): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, source } : channel);
  }

  setChannelTrim(channelId: string, valueDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, trimDb: clamp(valueDb, -24, 24) } : channel);
  }

  setChannelPan(channelId: string, value: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, pan: clamp(value, -100, 100) } : channel);
  }

  setChannelFader(channelId: string, valueDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, faderDb: clamp(valueDb, -10, 10) } : channel);
  }

  setChannelSend(channelId: string, unitId: FxUnitId, gainDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      sends: {
        ...channel.sends,
        [unitId]: channel.role === "system" || !channel.processing.insertFx
          ? { enabled: false, gainDb: -60 }
          : { enabled: gainDb > -60, gainDb: clamp(gainDb, -60, 10) }
      }
    } : channel);
  }

  setChannelMute(channelId: string, muted: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, mute: muted } : channel);
  }

  setChannelSolo(channelId: string, solo: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, solo } : channel);
  }

  setChannelMonitor(channelId: string, monitor: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => {
      if (channel.id === channelId) return { ...channel, monitor };
      if (monitor && channel.kind === "source") return { ...channel, monitor: false };
      return channel;
    });
  }

  setChannelRecordArm(channelId: string, armed: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, recordArm: armed } : channel);
    this.syncArmedRecordingChannels();
  }

  setRecordingStatus(status: RecordingWorkflowStatus, error = ""): void {
    this.snapshot.recording = { ...this.snapshot.recording, status, error };
  }

  addRecordedTakes(takes: RecordedTakeState[], armedChannelIds = this.snapshot.recording.armedChannelIds, takeDirectory = this.snapshot.recording.takeDirectory): void {
    const byId = new Map(this.snapshot.recording.takes.map((take) => [take.id, take]));
    for (const take of takes) byId.set(take.id, structuredClone(take));
    this.snapshot.recording = {
      ...this.snapshot.recording,
      status: "planned",
      error: "",
      armedChannelIds: [...armedChannelIds],
      takeDirectory,
      takes: [...byId.values()]
    };
  }

  setChannelProcessor(channelId: string, processorId: ProcessorId, enabled: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      processing: {
        ...channel.processing,
        [processorId]: processorId === "noise" && channel.role === "vocal"
          ? true
          : isProcessorAvailable(channel, processorId) ? enabled : false
      }
    } : channel);
  }

  setChannelNoiseAmount(channelId: string, amount: number): void {
    const nextAmount = clamp(amount, 0, 100);
    const noise = noiseSettingsFromAmount(nextAmount);
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      processing: {
        ...channel.processing,
        noise: isProcessorAvailable(channel, "noise")
      },
      dynamics: { ...channel.dynamics, noise }
    } : channel);
  }

  setChannelNoiseParam(channelId: string, field: keyof ChannelDynamicsState["noise"], value: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      dynamics: {
        ...channel.dynamics,
        noise: {
          ...channel.dynamics.noise,
          [field]: clampNoise(field, value)
        }
      }
    } : channel);
  }

  setChannelCompressorParam(channelId: string, field: keyof ChannelDynamicsState["compressor"], value: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      dynamics: {
        ...channel.dynamics,
        compressor: {
          ...channel.dynamics.compressor,
          [field]: clampCompressor(field, value)
        }
      }
    } : channel);
  }

  setChannelDeEsserParam(channelId: string, field: keyof ChannelDynamicsState["deEsser"], value: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      dynamics: {
        ...channel.dynamics,
        deEsser: {
          ...channel.dynamics.deEsser,
          [field]: clampDeEsser(field, value)
        }
      }
    } : channel);
  }

  setFxProgram(unitId: FxUnitId, programId: number): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? {
      ...unit,
      programId,
      revision: unit.revision + 1,
      pending: false,
      error: "",
      modified: false
    } : unit);
  }

  setFxProgramPending(unitId: FxUnitId, pending: boolean): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, pending, error: "" } : unit);
  }

  ackFxProgram(unitId: FxUnitId, programId: number, revision: number, modified = false): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? {
      ...unit,
      programId,
      revision,
      pending: false,
      error: "",
      modified
    } : unit);
  }

  setFxError(unitId: FxUnitId, error: string): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, pending: false, error } : unit);
  }

  setFxEnabled(unitId: FxUnitId, enabled: boolean): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, enabled } : unit);
  }

  setFxReturn(unitId: FxUnitId, valueDb: number): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, returnDb: clamp(valueDb, -60, 10), modified: true } : unit);
  }

  setFxProgramMacro(unitId: FxUnitId, macro: "macro1" | "macro2", value: string): void {
    const unit = this.snapshot.fxUnits.find((item) => item.id === unitId);
    if (!unit) return;
    this.snapshot.programs = this.snapshot.programs.map((program) => program.id === unit.programId ? {
      ...program,
      [macro]: { ...program[macro], value: value.trim() || program[macro].value }
    } : program);
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((item) => item.id === unitId ? { ...item, modified: true, revision: item.revision + 1, error: "" } : item);
  }

  updateEqBand(bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string): void {
    const updatedBands = updateEqBands(this.snapshot.eqBands, bandId, field, value);
    this.snapshot.eqBands = updatedBands;
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === this.snapshot.selectedChannelId ? {
      ...channel,
      processing: { ...channel.processing, eq: true },
      eqBands: structuredClone(updatedBands)
    } : channel);
  }

  setChannelEqBand(channelId: string, bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => {
      if (channel.id !== channelId) return channel;
      const eqBands = updateEqBands(channel.eqBands, bandId, field, value);
      if (channel.id === this.snapshot.selectedChannelId) this.snapshot.eqBands = structuredClone(eqBands);
      return {
        ...channel,
        processing: { ...channel.processing, eq: true },
        eqBands
      };
    });
  }

  resetChannelEqBands(channelId: string): void {
    const resetBands = defaultEqBands();
    this.snapshot.channels = this.snapshot.channels.map((channel) => {
      if (channel.id !== channelId) return channel;
      if (channel.id === this.snapshot.selectedChannelId) this.snapshot.eqBands = structuredClone(resetBands);
      return { ...channel, eqBands: structuredClone(resetBands) };
    });
  }

  resetEqBands(): void {
    const resetBands = defaultEqBands();
    this.snapshot.eqBands = resetBands;
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === this.snapshot.selectedChannelId ? {
      ...channel,
      eqBands: structuredClone(resetBands)
    } : channel);
  }

  resetFxProgram(unitId: FxUnitId): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, modified: false, revision: unit.revision + 1, error: "" } : unit);
  }

  setHarmonyEnabled(enabled: boolean): void {
    this.snapshot.harmony = {
      ...this.snapshot.harmony,
      enabled,
      effectiveEnabled: enabled,
      pending: false,
      error: "",
      revision: this.snapshot.harmony.revision + 1,
      primaryInstanceId: enabled && !this.snapshot.harmony.primaryInstanceId ? "harmony:voice:primary" : this.snapshot.harmony.primaryInstanceId
    };
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.role === "vocal" ? { ...channel, harmonyEnabled: enabled } : channel);
    this.syncHarmonyVocalFx(enabled);
  }

  setHarmonyPending(pending: boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, pending, error: "" };
  }

  ackHarmony(state: Partial<HarmonyState>): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, ...state, pending: false, error: "" };
    const enabled = Boolean(state.enabled ?? this.snapshot.harmony.enabled);
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.role === "vocal" ? {
      ...channel,
      harmonyEnabled: enabled
    } : channel);
    this.syncHarmonyVocalFx(enabled);
  }

  setHarmonyError(error: string): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, pending: false, error };
  }

  updateHarmony(field: keyof HarmonyState, value: string | number | boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, [field]: value, revision: this.snapshot.harmony.revision + 1, error: "" };
    if ((field === "levelDb" || field === "voice1" || field === "voice2") && this.snapshot.harmony.enabled) {
      this.syncHarmonyVocalFx(true);
    }
  }

  selectVocalFxSlot(slotId: string): void {
    if (this.snapshot.vocalFx.slots.some((slot) => slot.id === slotId)) {
      this.snapshot.vocalFx = { ...this.snapshot.vocalFx, selectedSlotId: slotId };
    }
  }

  setVocalFxSlotEnabled(slotId: string, enabled: boolean): void {
    this.snapshot.vocalFx = {
      ...this.snapshot.vocalFx,
      slots: this.snapshot.vocalFx.slots.map((slot) => slot.id === slotId ? { ...slot, enabled } : slot)
    };
  }

  setVocalFxSlotMix(slotId: string, mix: number): void {
    const bounded = clamp(mix, 0, 1);
    this.snapshot.vocalFx = {
      ...this.snapshot.vocalFx,
      slots: this.snapshot.vocalFx.slots.map((slot) => slot.id === slotId ? {
        ...slot,
        mix: bounded,
        parameters: updateSlotMixParameter(slot.parameters, bounded)
      } : slot)
    };
  }

  applyVocalFxPreset(presetId: string): void {
    if (presetId === "default") {
      this.snapshot.vocalFx = {
        ...this.snapshot.vocalFx,
        activePresetId: "default",
        slots: this.snapshot.vocalFx.slots.map((slot) => ({ ...slot, enabled: slot.id === "harmony" && this.snapshot.harmony.enabled }))
      };
      return;
    }
    if (!this.snapshot.vocalFx.presets.some((preset) => preset.id === presetId)) return;
    this.snapshot.vocalFx = {
      ...this.snapshot.vocalFx,
      activePresetId: presetId,
      selectedSlotId: selectedSlotForPreset(presetId, this.snapshot.vocalFx.selectedSlotId),
      slots: this.snapshot.vocalFx.slots.map((slot) => ({
        ...slot,
        enabled: presetEnabled(presetId, slot.effectType) || (slot.id === "harmony" && this.snapshot.harmony.enabled)
      }))
    };
  }

  resetClip(channelId: string): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      meter: { ...channel.meter, clip: false }
    } : channel);
  }

  private syncSelectedEqBands(): void {
    const selected = this.snapshot.channels.find((channel) => channel.id === this.snapshot.selectedChannelId);
    if (selected) this.snapshot.eqBands = structuredClone(selected.eqBands);
  }

  private syncArmedRecordingChannels(): void {
    this.snapshot.recording = {
      ...this.snapshot.recording,
      armedChannelIds: this.snapshot.channels
        .filter((channel) => channel.kind === "source" && channel.recordArm)
        .map((channel) => channel.id)
    };
  }

  private syncHarmonyVocalFx(enabled: boolean): void {
    const harmonyMix = harmonyMixFromLevelDb(this.snapshot.harmony.levelDb);
    this.snapshot.vocalFx = {
      ...this.snapshot.vocalFx,
      selectedSlotId: enabled ? "harmony" : this.snapshot.vocalFx.selectedSlotId,
      slots: this.snapshot.vocalFx.slots.map((slot) => slot.id === "harmony" ? {
        ...slot,
        enabled,
        mix: harmonyMix,
        parameters: updateHarmonySlotParameters(slot.parameters, this.snapshot.harmony, harmonyMix)
      } : slot)
    };
    const hasActiveSlot = this.snapshot.vocalFx.slots.some((slot) => slot.enabled);
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.role === "vocal" ? {
      ...channel,
      processing: { ...channel.processing, insertFx: hasActiveSlot }
    } : channel);
  }
}

function newSourceChannel(id: string, name: string, role: Exclude<ChannelRole, "group" | "master">, source: string): ChannelState {
  return {
    id,
    name,
    source,
    kind: "source",
    role,
    selected: false,
    enabled: true,
    trimDb: 0,
    pan: 0,
    faderDb: 0,
    mute: false,
    solo: false,
    monitor: false,
    recordArm: false,
    harmonyVisible: role === "vocal",
    harmonyEnabled: false,
    processing: { eq: true, comp: role === "vocal", noise: role === "vocal", deEsser: false, insertFx: false },
    dynamics: defaultDynamics(),
    sends: { "fx-a": { enabled: false, gainDb: -60 }, "fx-b": { enabled: false, gainDb: -60 } },
    eqBands: defaultEqBands(),
    meter: { left: -60, right: -60, clip: false }
  };
}

function defaultChannelName(role: Exclude<ChannelRole, "group" | "master">) {
  if (role === "vocal") return "VOICE";
  if (role === "instrument") return "INSTRUMENT";
  if (role === "music") return "MUSIC";
  return "SYSTEM";
}

function uniqueChannelId(channels: ChannelState[], baseId: string) {
  const used = new Set(channels.map((channel) => channel.id));
  let id = baseId || "channel";
  let suffix = 2;
  while (used.has(id)) {
    id = `${baseId}-${suffix}`;
    suffix += 1;
  }
  return id;
}

function slugChannelId(name: string) {
  return name.trim().toLowerCase().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, "") || "channel";
}

function defaultEqBands(): EqBandState[] {
  return [
    { id: "low", label: "LOW", color: "#58F28A", freqHz: 100, gainDb: 0, freq: "100 Hz", gain: "+0.0 dB", type: "Shelf" },
    { id: "mid1", label: "MID 1", color: "#FFB843", freqHz: 350, gainDb: 0, qValue: 1, freq: "350 Hz", gain: "+0.0 dB", q: "1.00" },
    { id: "mid2", label: "MID 2", color: "#1FA8FF", freqHz: 2500, gainDb: 0, qValue: 1, freq: "2.5 kHz", gain: "+0.0 dB", q: "1.00" },
    { id: "high", label: "HIGH", color: "#B862F0", freqHz: 10000, gainDb: 0, freq: "10.0 kHz", gain: "+0.0 dB", type: "Shelf" }
  ];
}

function defaultDynamics(): ChannelDynamicsState {
  return {
    noise: { thresholdDb: -38, rangeDb: -50, holdMs: 30, releaseMs: 160 },
    compressor: { thresholdDb: -18, ratio: 3, attackMs: 10, releaseMs: 120 },
    deEsser: { frequencyHz: 6000, thresholdDb: -24, maxReductionDb: 6 }
  };
}

function presetEnabled(presetId: string, effectType: string) {
  return vocalFxPresetEffectTypes(presetId)?.includes(effectType) ?? false;
}

function updateSlotMixParameter(parameters: VocalFxParameter[], mix: number) {
  return parameters.map((parameter) => (
    parameter.label === "Mix" || parameter.label === "Wet"
      ? { ...parameter, value: `${Math.round(mix * 100)}%` }
      : parameter
  ));
}

function updateSlotLevelParameter(parameters: VocalFxParameter[], mix: number) {
  return parameters.map((parameter) => (
    parameter.label === "Level" ? { ...parameter, value: `${Math.round(mix * 100)}%` } : parameter
  ));
}

function updateHarmonySlotParameters(parameters: VocalFxParameter[], harmony: HarmonyState, mix: number) {
  return updateSlotLevelParameter(parameters, mix).map((parameter) => {
    if (parameter.label === "Voice 1") return { ...parameter, value: harmony.voice1 };
    if (parameter.label === "Voice 2") return { ...parameter, value: harmony.voice2 };
    return parameter;
  });
}

function harmonyMixFromLevelDb(levelDb: number) {
  const normalized = clamp((levelDb + 30) / 36, 0, 1);
  return roundTo(0.12 + normalized * 0.64, 2);
}

function selectedSlotForPreset(presetId: string, fallback: string) {
  const enabled = vocalFxPresetEffectTypes(presetId) ?? [];
  if (enabled.includes("vocoder")) return "robot";
  if (enabled.includes("harmony")) return "harmony";
  if (enabled.includes("pitch_correct")) return "pitch-correct";
  if (enabled.includes("reverb")) return "plate";
  if (enabled.includes("delay")) return "stereo-delay";
  if (enabled.includes("doubler")) return "doubler";
  if (enabled.includes("saturation")) return "saturation";
  if (enabled.includes("formant_shift")) return "formant-shift";
  return fallback;
}

function updateEqBands(bands: EqBandState[], bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string) {
  return bands.map((band) => {
    if (band.id !== bandId) return band;
    if (field === "type") return { ...band, type: String(value) };

    const next = { ...band, [field]: Number(value) };
    next.freqHz = clamp(next.freqHz, 20, 20_000);
    next.gainDb = clamp(next.gainDb, -12, 12);
    if (next.qValue !== undefined) next.qValue = clamp(next.qValue, 0.1, 12);
    return formatEqBand(next);
  });
}

function clamp(value: number, min: number, max: number) {
  return Math.max(min, Math.min(max, Number.isFinite(value) ? value : min));
}

function clampCompressor(field: keyof ChannelDynamicsState["compressor"], value: number) {
  if (field === "thresholdDb") return clamp(value, -80, 0);
  if (field === "ratio") return clamp(value, 1, 20);
  if (field === "attackMs") return clamp(value, 0.1, 200);
  return clamp(value, 10, 3000);
}

function noiseSettingsFromAmount(amount: number): ChannelDynamicsState["noise"] {
  const normalized = clamp(amount, 0, 100) / 100;
  const curve = normalized ** 0.55;
  const hardZone = clamp((normalized - 0.85) / 0.15, 0, 1) ** 2;
  return {
    thresholdDb: roundTo(-55 + curve * 45 + hardZone * 8, 1),
    rangeDb: roundTo(-32 - curve * 68 - hardZone * 20, 1),
    holdMs: Math.round(90 - curve * 80 - hardZone * 10),
    releaseMs: Math.round(180 - curve * 120 - hardZone * 35)
  };
}

function isProcessorAvailable(channel: ChannelState, processorId: ProcessorId) {
  if (processorId === "noise") return channel.role !== "system";
  if (processorId === "insertFx") return channel.role !== "system";
  if (processorId === "deEsser") return channel.role === "vocal";
  return true;
}

function clampNoise(field: keyof ChannelDynamicsState["noise"], value: number) {
  if (field === "thresholdDb") return clamp(value, -90, 0);
  if (field === "rangeDb") return clamp(value, -90, 0);
  if (field === "holdMs") return clamp(value, 0, 1000);
  return clamp(value, 5, 3000);
}

function roundTo(value: number, decimals: number) {
  const factor = 10 ** decimals;
  return Math.round(value * factor) / factor;
}

function clampDeEsser(field: keyof ChannelDynamicsState["deEsser"], value: number) {
  if (field === "frequencyHz") return clamp(value, 1000, 12000);
  if (field === "thresholdDb") return clamp(value, -80, 0);
  return clamp(value, 0, 24);
}

function formatEqBand<T extends EqBandState>(band: T): T {
  return {
    ...band,
    freq: formatFreq(band.freqHz),
    gain: `${band.gainDb >= 0 ? "+" : ""}${band.gainDb.toFixed(1)} dB`,
    q: band.qValue === undefined ? band.q : band.qValue.toFixed(2)
  };
}

function formatFreq(freqHz: number) {
  if (freqHz >= 1000) return `${(freqHz / 1000).toFixed(freqHz >= 10_000 ? 1 : 1)} kHz`;
  return `${Math.round(freqHz)} Hz`;
}
