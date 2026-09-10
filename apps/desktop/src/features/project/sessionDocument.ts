import type { ChannelState, FxUnitState, MixerSnapshot } from "../../adapters/MixerControlPort";

export interface ProjectSessionDocument {
  schemaVersion: 1;
  projectId: string;
  media: Array<{ id: string; path: string; missing: boolean }>;
  channels: ProjectSessionChannel[];
  fxUnits: ProjectSessionFxUnit[];
  fxSends: ProjectSessionFxSend[];
  channelHarmony: ProjectSessionHarmony[];
  plugins: unknown[];
  recordedTakes: unknown[];
}

export interface ProjectSessionChannel {
  id: string;
  name: string;
  kind: string;
  role: string;
  sourceUid: string;
  enabled: boolean;
  muted: boolean;
  solo: boolean;
  monitor: boolean;
  recordArm: boolean;
  eqEnabled: boolean;
  noiseEnabled: boolean;
  compEnabled: boolean;
  insertFxEnabled: boolean;
  gainDb: number;
  faderDb: number;
  pan: number;
  noiseThresholdDb: number;
  noiseRangeDb: number;
  compThresholdDb: number;
  compRatio: number;
}

export interface ProjectSessionFxUnit {
  unitId: string;
  programId: number;
  processorType: string;
  revision: number;
  enabled: boolean;
  modified: boolean;
  returnDb: number;
  macro1Value: string;
  macro2Value: string;
}

export interface ProjectSessionFxSend {
  channelId: string;
  unitId: string;
  enabled: boolean;
  gainDb: number;
}

export interface ProjectSessionHarmony {
  channelId: string;
  contentRole: string;
  primaryHarmonyInstanceId: string;
  harmonyEnabled: boolean;
  key: string;
  scale: string;
  mode: string;
  voice1: string;
  voice2: string;
  harmonyLevelDb: number;
}

export function snapshotToProjectSession(snapshot: MixerSnapshot): ProjectSessionDocument {
  return {
    schemaVersion: 1,
    projectId: slugProjectId(snapshot.projectName),
    media: mediaRefs(snapshot),
    channels: snapshot.channels.map(channelToSession),
    fxUnits: snapshot.fxUnits.map((unit) => fxUnitToSession(unit, snapshot)),
    fxSends: snapshot.channels.flatMap(channelFxSends),
    channelHarmony: snapshot.channels.filter((channel) => channel.role === "vocal").map((channel) => ({
      channelId: channel.id,
      contentRole: channel.role,
      primaryHarmonyInstanceId: snapshot.harmony.primaryInstanceId,
      harmonyEnabled: Boolean(channel.harmonyEnabled),
      key: snapshot.harmony.key,
      scale: snapshot.harmony.scale,
      mode: snapshot.harmony.mode,
      voice1: snapshot.harmony.voice1,
      voice2: snapshot.harmony.voice2,
      harmonyLevelDb: snapshot.harmony.levelDb
    })),
    plugins: [],
    recordedTakes: []
  };
}

export function serializeProjectSession(snapshot: MixerSnapshot): string {
  return `${JSON.stringify(snapshotToProjectSession(snapshot), null, 2)}\n`;
}

function channelToSession(channel: ChannelState): ProjectSessionChannel {
  return {
    id: channel.id,
    name: channel.name,
    kind: channel.kind,
    role: channel.role,
    sourceUid: channel.kind === "source" ? channel.source : "",
    enabled: channel.enabled,
    muted: channel.mute,
    solo: channel.solo,
    monitor: Boolean(channel.monitor),
    recordArm: Boolean(channel.recordArm),
    eqEnabled: channel.processing.eq,
    noiseEnabled: channel.processing.noise,
    compEnabled: channel.processing.comp,
    insertFxEnabled: channel.processing.insertFx,
    gainDb: channel.trimDb,
    faderDb: channel.faderDb,
    pan: channel.pan / 100,
    noiseThresholdDb: channel.dynamics.noise.thresholdDb,
    noiseRangeDb: channel.dynamics.noise.rangeDb,
    compThresholdDb: channel.dynamics.compressor.thresholdDb,
    compRatio: channel.dynamics.compressor.ratio
  };
}

function fxUnitToSession(unit: FxUnitState, snapshot: MixerSnapshot): ProjectSessionFxUnit {
  const program = snapshot.programs.find((item) => item.id === unit.programId);
  return {
    unitId: unit.id,
    programId: unit.programId,
    processorType: program?.family ?? "",
    revision: unit.revision,
    enabled: unit.enabled,
    modified: unit.modified,
    returnDb: unit.returnDb,
    macro1Value: program?.macro1.value ?? "",
    macro2Value: program?.macro2.value ?? ""
  };
}

function channelFxSends(channel: ChannelState): ProjectSessionFxSend[] {
  return Object.entries(channel.sends).map(([unitId, send]) => ({
    channelId: channel.id,
    unitId,
    enabled: send.enabled,
    gainDb: send.gainDb
  }));
}

function mediaRefs(snapshot: MixerSnapshot) {
  return snapshot.channels
    .filter((channel) => channel.role === "music" && channel.source.trim() !== "")
    .map((channel) => ({ id: channel.id, path: channel.source, missing: false }));
}

function slugProjectId(projectName: string) {
  const slug = projectName.trim().toLowerCase().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, "");
  return slug || "local-audio-mixer";
}
