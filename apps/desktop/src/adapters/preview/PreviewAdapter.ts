import type { EqBandState, FxUnitId, HarmonyState, MixerControlPort, MixerSnapshot, ProcessorId } from "../MixerControlPort";
import { approvedMixerSession } from "../../fixtures/approvedMixerSession";

export class PreviewAdapter implements MixerControlPort {
  private snapshot: MixerSnapshot = structuredClone(approvedMixerSession);

  getSnapshot(): MixerSnapshot {
    return this.snapshot;
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
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, monitor } : channel);
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
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, programId, modified: false } : unit);
  }

  setFxEnabled(unitId: FxUnitId, enabled: boolean): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, enabled } : unit);
  }

  setFxReturn(unitId: FxUnitId, valueDb: number): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, returnDb: clamp(valueDb, -60, 10), modified: true } : unit);
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
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, modified: false } : unit);
  }

  setHarmonyEnabled(enabled: boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, enabled };
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.role === "vocal" ? { ...channel, harmonyEnabled: enabled } : channel);
  }

  updateHarmony(field: keyof HarmonyState, value: string | number | boolean): void {
    this.snapshot.harmony = { ...this.snapshot.harmony, [field]: value };
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
