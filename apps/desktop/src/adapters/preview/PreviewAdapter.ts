import type { EqBandState, FxProgram, FxUnitId, HarmonyState, MixerControlPort, MixerSnapshot, ProcessorId } from "../MixerControlPort";
import { approvedMixerSession } from "../../fixtures/approvedMixerSession";

export class PreviewAdapter implements MixerControlPort {
  private snapshot: MixerSnapshot = structuredClone(approvedMixerSession);

  getSnapshot(): MixerSnapshot {
    return this.snapshot;
  }

  setPrograms(programs: FxProgram[]): void {
    if (programs.length === 0) return;
    const programIds = new Set(programs.map((program) => program.id));
    this.snapshot.programs = structuredClone(programs);
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => (
      programIds.has(unit.programId) ? unit : { ...unit, programId: programs[0].id, modified: false }
    ));
  }

  selectChannel(channelId: string): void {
    this.snapshot.selectedChannelId = channelId;
    this.snapshot.channels = this.snapshot.channels.map((channel) => ({ ...channel, selected: channel.id === channelId }));
    this.syncSelectedEqBands();
  }

  setChannelEnabled(channelId: string, enabled: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      enabled,
      meter: enabled ? channel.meter : { left: -60, right: -60, clip: false }
    } : channel);
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
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, faderDb: clamp(valueDb, -60, 10) } : channel);
  }

  setChannelSend(channelId: string, unitId: FxUnitId, gainDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      sends: { ...channel.sends, [unitId]: { enabled: gainDb > -60, gainDb: clamp(gainDb, -60, 10) } }
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
  }

  setChannelProcessor(channelId: string, processorId: ProcessorId, enabled: boolean): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      processing: { ...channel.processing, [processorId]: enabled }
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
    const updatedBands = this.snapshot.eqBands.map((band) => {
      if (band.id !== bandId) return band;
      if (field === "type") return { ...band, type: String(value) };

      const next = { ...band, [field]: Number(value) };
      next.freqHz = clamp(next.freqHz, 20, 20_000);
      next.gainDb = clamp(next.gainDb, -12, 12);
      if (next.qValue !== undefined) next.qValue = clamp(next.qValue, 0.1, 12);
      return formatEqBand(next);
    });
    this.snapshot.eqBands = updatedBands;
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === this.snapshot.selectedChannelId ? {
      ...channel,
      eqBands: structuredClone(updatedBands)
    } : channel);
  }

  resetFxProgram(unitId: FxUnitId): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, modified: false, revision: unit.revision + 1, error: "" } : unit);
  }

  setHarmonyEnabled(enabled: boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, enabled };
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.role === "vocal" ? { ...channel, harmonyEnabled: enabled } : channel);
  }

  updateHarmony(field: keyof HarmonyState, value: string | number | boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, [field]: value };
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

  applyVocalFxPreset(presetId: string): void {
    if (!this.snapshot.vocalFx.presets.some((preset) => preset.id === presetId)) return;
    this.snapshot.vocalFx = {
      ...this.snapshot.vocalFx,
      activePresetId: presetId,
      selectedSlotId: presetId === "robot" ? "robot" : presetId === "harmony-duo" ? "harmony" : this.snapshot.vocalFx.selectedSlotId,
      slots: this.snapshot.vocalFx.slots.map((slot) => ({
        ...slot,
        enabled: presetEnabled(presetId, slot.effectType, slot.enabled)
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
}

function presetEnabled(presetId: string, effectType: string, fallback: boolean) {
  const enabledByPreset: Record<string, string[]> = {
    "clean-voice": [],
    "warm-broadcast": ["saturation", "reverb"],
    "studio-pop": ["pitch_correct", "doubler", "reverb"],
    "karaoke-hall": ["reverb", "delay"],
    "slapback": ["delay"],
    "wide-double": ["doubler", "chorus"],
    "low-character": ["pitch_shift", "formant_shift"],
    "bright-character": ["pitch_shift", "formant_shift"],
    "hard-tune": ["pitch_correct"],
    "harmony-duo": ["harmony", "reverb"],
    "telephone": ["saturation"],
    "robot": ["vocoder"]
  };
  return enabledByPreset[presetId]?.includes(effectType) ?? fallback;
}

function clamp(value: number, min: number, max: number) {
  return Math.max(min, Math.min(max, Number.isFinite(value) ? value : min));
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
