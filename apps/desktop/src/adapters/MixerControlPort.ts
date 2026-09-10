import type { EqBandDisplay, MeterLevel } from "../components/audio/types";

export type ChannelKind = "source" | "group" | "master";
export type ChannelRole = "system" | "vocal" | "instrument" | "music" | "group" | "master";
export type FxUnitId = "fx-a" | "fx-b";
export type ProcessorId = "eq" | "comp" | "noise" | "insertFx";

export interface SendState {
  enabled: boolean;
  gainDb: number;
}

export interface ChannelState {
  id: string;
  name: string;
  source: string;
  kind: ChannelKind;
  role: ChannelRole;
  selected?: boolean;
  enabled: boolean;
  trimDb: number;
  pan: number;
  faderDb: number;
  mute: boolean;
  solo: boolean;
  monitor?: boolean;
  recordArm?: boolean;
  harmonyVisible?: boolean;
  harmonyEnabled?: boolean;
  processing: Record<ProcessorId, boolean>;
  sends: Record<FxUnitId, SendState>;
  eqBands: EqBandState[];
  meter: MeterLevel;
}

export interface FxProgram {
  id: number;
  name: string;
  family: string;
  macro1: { label: string; value: string };
  macro2: { label: string; value: string };
}

export interface FxUnitState {
  id: FxUnitId;
  label: "FX A" | "FX B";
  accent: "amber" | "cyan";
  enabled: boolean;
  programId: number;
  revision: number;
  pending: boolean;
  error: string;
  modified: boolean;
  returnDb: number;
  meter: MeterLevel;
}

export type EqBandState = EqBandDisplay;

export interface HarmonyState {
  enabled: boolean;
  effectiveEnabled: boolean;
  pending: boolean;
  error: string;
  revision: number;
  primaryInstanceId: string;
  key: string;
  scale: string;
  mode: string;
  voice1: string;
  voice2: string;
  levelDb: number;
}

export interface VocalFxParameter {
  id: string;
  label: string;
  value: string;
}

export interface VocalFxSlot {
  id: string;
  effectType: string;
  label: string;
  category: "Pitch" | "Space" | "Modulation" | "Character" | "Synth";
  enabled: boolean;
  availability: "implemented_unverified" | "verified";
  latencyMs: number;
  parameters: VocalFxParameter[];
}

export interface VocalFxPreset {
  id: string;
  name: string;
}

export interface VocalFxState {
  selectedSlotId: string;
  activePresetId: string;
  slots: VocalFxSlot[];
  presets: VocalFxPreset[];
}

export interface MixerSnapshot {
  modeLabel: string;
  projectName: string;
  transportTime: string;
  sampleRateLabel: string;
  engineStatus: string;
  selectedChannelId: string;
  channels: ChannelState[];
  fxUnits: FxUnitState[];
  programs: FxProgram[];
  eqBands: EqBandState[];
  harmony: HarmonyState;
  vocalFx: VocalFxState;
}

export interface MixerControlPort {
  getSnapshot(): MixerSnapshot;
  setPrograms(programs: FxProgram[]): void;
  selectChannel(channelId: string): void;
  setChannelEnabled(channelId: string, enabled: boolean): void;
  setChannelSource(channelId: string, source: string): void;
  setChannelTrim(channelId: string, valueDb: number): void;
  setChannelPan(channelId: string, value: number): void;
  setChannelFader(channelId: string, valueDb: number): void;
  setChannelSend(channelId: string, unitId: FxUnitId, gainDb: number): void;
  setChannelMute(channelId: string, muted: boolean): void;
  setChannelSolo(channelId: string, solo: boolean): void;
  setChannelMonitor(channelId: string, monitor: boolean): void;
  setChannelRecordArm(channelId: string, armed: boolean): void;
  setChannelProcessor(channelId: string, processorId: ProcessorId, enabled: boolean): void;
  setFxProgram(unitId: FxUnitId, programId: number): void;
  setFxProgramPending(unitId: FxUnitId, pending: boolean): void;
  ackFxProgram(unitId: FxUnitId, programId: number, revision: number, modified?: boolean): void;
  setFxError(unitId: FxUnitId, error: string): void;
  setFxEnabled(unitId: FxUnitId, enabled: boolean): void;
  setFxReturn(unitId: FxUnitId, valueDb: number): void;
  setFxProgramMacro(unitId: FxUnitId, macro: "macro1" | "macro2", value: string): void;
  updateEqBand(bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string): void;
  resetFxProgram(unitId: FxUnitId): void;
  setHarmonyEnabled(enabled: boolean): void;
  setHarmonyPending(pending: boolean): void;
  ackHarmony(state: Partial<HarmonyState>): void;
  setHarmonyError(error: string): void;
  updateHarmony(field: keyof HarmonyState, value: string | number | boolean): void;
  selectVocalFxSlot(slotId: string): void;
  setVocalFxSlotEnabled(slotId: string, enabled: boolean): void;
  applyVocalFxPreset(presetId: string): void;
  resetClip(channelId: string): void;
}
