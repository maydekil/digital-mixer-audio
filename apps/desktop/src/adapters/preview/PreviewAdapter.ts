import type { FxUnitId, HarmonyState, MixerControlPort, MixerSnapshot } from "../MixerControlPort";
import { approvedMixerSession } from "../../fixtures/approvedMixerSession";

export class PreviewAdapter implements MixerControlPort {
  private snapshot: MixerSnapshot = structuredClone(approvedMixerSession);

  getSnapshot(): MixerSnapshot {
    return this.snapshot;
  }

  selectChannel(channelId: string): void {
    this.snapshot.selectedChannelId = channelId;
    this.snapshot.channels = this.snapshot.channels.map((channel) => ({ ...channel, selected: channel.id === channelId }));
  }

  setChannelFader(channelId: string, valueDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? { ...channel, faderDb: valueDb } : channel);
  }

  setChannelSend(channelId: string, unitId: FxUnitId, gainDb: number): void {
    this.snapshot.channels = this.snapshot.channels.map((channel) => channel.id === channelId ? {
      ...channel,
      sends: { ...channel.sends, [unitId]: { enabled: true, gainDb } }
    } : channel);
  }

  setFxProgram(unitId: FxUnitId, programId: number): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, programId, modified: false } : unit);
  }

  setFxEnabled(unitId: FxUnitId, enabled: boolean): void {
    this.snapshot.fxUnits = this.snapshot.fxUnits.map((unit) => unit.id === unitId ? { ...unit, enabled } : unit);
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
}
