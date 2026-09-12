import type { ChannelKind, ChannelRole, ChannelState, FxUnitState, MixerSnapshot, RecordedTakeState, RecordingTap } from "../../adapters/MixerControlPort";

export interface ProjectSessionDocument {
  schemaVersion: 1;
  projectId: string;
  media: Array<{ id: string; path: string; missing: boolean }>;
  channels: ProjectSessionChannel[];
  fxUnits: ProjectSessionFxUnit[];
  fxSends: ProjectSessionFxSend[];
  channelHarmony: ProjectSessionHarmony[];
  plugins: unknown[];
  recordedTakes: ProjectSessionRecordedTake[];
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
  deEsserEnabled: boolean;
  insertFxEnabled: boolean;
  gainDb: number;
  faderDb: number;
  pan: number;
  noiseThresholdDb: number;
  noiseRangeDb: number;
  noiseHoldMs: number;
  noiseReleaseMs: number;
  compThresholdDb: number;
  compRatio: number;
  compAttackMs: number;
  compReleaseMs: number;
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

export interface ProjectSessionRecordedTake {
  id: string;
  path: string;
  tap: string;
  sampleRate: number;
  channelCount: number;
  frameCount: number;
  replayWithNeutralInserts: boolean;
  partial: boolean;
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
    recordedTakes: snapshot.recording.takes.map(takeToSession)
  };
}

export function serializeProjectSession(snapshot: MixerSnapshot): string {
  return `${JSON.stringify(snapshotToProjectSession(snapshot), null, 2)}\n`;
}

export function projectSessionToSnapshot(content: string, baseSnapshot: MixerSnapshot): MixerSnapshot {
  const document = JSON.parse(content) as Partial<ProjectSessionDocument>;
  if (document.schemaVersion !== 1 || typeof document.projectId !== "string" || !Array.isArray(document.channels)) {
    throw new Error("Invalid Local Audio Mixer project document");
  }

  const snapshot = structuredClone(baseSnapshot);
  snapshot.projectName = document.projectId || snapshot.projectName;
  const savedChannels = document.channels ?? [];
  snapshot.channels = snapshot.channels.map((channel) => {
    const saved = savedChannels.find((item) => item.id === channel.id);
    return saved ? applyChannelSession(channel, saved) : channel;
  });
  const addedChannels = savedChannels
    .filter((saved) => !snapshot.channels.some((channel) => channel.id === saved.id))
    .map((saved) => channelSessionToSnapshot(saved, baseSnapshot));
  if (addedChannels.length > 0) {
    const masterIndex = snapshot.channels.findIndex((channel) => channel.kind === "master");
    const insertAt = masterIndex < 0 ? snapshot.channels.length : masterIndex;
    snapshot.channels = [
      ...snapshot.channels.slice(0, insertAt),
      ...addedChannels,
      ...snapshot.channels.slice(insertAt)
    ];
  }
  snapshot.fxUnits = snapshot.fxUnits.map((unit) => {
    const saved = document.fxUnits?.find((item) => item.unitId === unit.id);
    return saved ? applyFxUnitSession(unit, saved) : unit;
  });
  snapshot.channels = snapshot.channels.map((channel) => ({
    ...channel,
    sends: Object.fromEntries(Object.entries(channel.sends).map(([unitId, send]) => {
      const saved = document.fxSends?.find((item) => item.channelId === channel.id && item.unitId === unitId);
      return [unitId, saved ? { enabled: Boolean(saved.enabled), gainDb: Number(saved.gainDb) } : send];
    })) as ChannelState["sends"]
  }));
  const savedHarmony = document.channelHarmony?.find((item) => item.channelId === "voice") ?? document.channelHarmony?.[0];
  if (savedHarmony) {
    snapshot.harmony = {
      ...snapshot.harmony,
      enabled: Boolean(savedHarmony.harmonyEnabled),
      effectiveEnabled: Boolean(savedHarmony.harmonyEnabled),
      primaryInstanceId: savedHarmony.primaryHarmonyInstanceId,
      key: savedHarmony.key,
      scale: savedHarmony.scale,
      mode: savedHarmony.mode,
      voice1: savedHarmony.voice1,
      voice2: savedHarmony.voice2,
      levelDb: Number(savedHarmony.harmonyLevelDb),
      pending: false,
      error: ""
    };
  }
  snapshot.recording = {
    ...snapshot.recording,
    status: "idle",
    error: "",
    armedChannelIds: snapshot.channels.filter((channel) => channel.kind === "source" && channel.recordArm).map((channel) => channel.id),
    takes: (document.recordedTakes ?? []).flatMap(takeSessionToSnapshot),
    takeDirectory: firstTakeDirectory(document.recordedTakes ?? []) || snapshot.recording.takeDirectory
  };
  snapshot.selectedChannelId = snapshot.channels.some((channel) => channel.id === snapshot.selectedChannelId)
    ? snapshot.selectedChannelId
    : snapshot.channels[0]?.id ?? "";
  snapshot.channels = snapshot.channels.map((channel) => ({
    ...channel,
    selected: channel.id === snapshot.selectedChannelId,
    harmonyEnabled: channel.role === "vocal" ? snapshot.harmony.enabled : channel.harmonyEnabled
  }));
  snapshot.eqBands = structuredClone(snapshot.channels.find((channel) => channel.id === snapshot.selectedChannelId)?.eqBands ?? snapshot.eqBands);
  return snapshot;
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
    deEsserEnabled: channel.processing.deEsser,
    insertFxEnabled: channel.processing.insertFx,
    gainDb: channel.trimDb,
    faderDb: channel.faderDb,
    pan: channel.pan / 100,
    noiseThresholdDb: channel.dynamics.noise.thresholdDb,
    noiseRangeDb: channel.dynamics.noise.rangeDb,
    noiseHoldMs: channel.dynamics.noise.holdMs,
    noiseReleaseMs: channel.dynamics.noise.releaseMs,
    compThresholdDb: channel.dynamics.compressor.thresholdDb,
    compRatio: channel.dynamics.compressor.ratio,
    compAttackMs: channel.dynamics.compressor.attackMs,
    compReleaseMs: channel.dynamics.compressor.releaseMs
  };
}

function applyChannelSession(channel: ChannelState, saved: ProjectSessionChannel): ChannelState {
  return {
    ...channel,
    name: saved.name || channel.name,
    source: saved.sourceUid || channel.source,
    enabled: Boolean(saved.enabled),
    mute: Boolean(saved.muted),
    solo: Boolean(saved.solo),
    monitor: Boolean(saved.monitor),
    recordArm: Boolean(saved.recordArm),
    trimDb: Number(saved.gainDb),
    faderDb: clamp(Number(saved.faderDb), -10, 10),
    pan: Number(saved.pan) * 100,
    processing: {
      ...channel.processing,
      eq: Boolean(saved.eqEnabled),
      noise: channel.role === "vocal" ? true : Boolean(saved.noiseEnabled),
      comp: Boolean(saved.compEnabled),
      deEsser: Boolean(saved.deEsserEnabled),
      insertFx: Boolean(saved.insertFxEnabled)
    },
    dynamics: {
      ...channel.dynamics,
      noise: {
        ...channel.dynamics.noise,
        thresholdDb: Number(saved.noiseThresholdDb),
        rangeDb: Number(saved.noiseRangeDb),
        holdMs: Number.isFinite(Number(saved.noiseHoldMs)) ? Number(saved.noiseHoldMs) : channel.dynamics.noise.holdMs,
        releaseMs: Number.isFinite(Number(saved.noiseReleaseMs)) ? Number(saved.noiseReleaseMs) : channel.dynamics.noise.releaseMs
      },
      compressor: {
        ...channel.dynamics.compressor,
        thresholdDb: Number(saved.compThresholdDb),
        ratio: Number(saved.compRatio),
        attackMs: Number.isFinite(Number(saved.compAttackMs)) ? Number(saved.compAttackMs) : channel.dynamics.compressor.attackMs,
        releaseMs: Number.isFinite(Number(saved.compReleaseMs)) ? Number(saved.compReleaseMs) : channel.dynamics.compressor.releaseMs
      }
    }
  };
}

function channelSessionToSnapshot(saved: ProjectSessionChannel, baseSnapshot: MixerSnapshot): ChannelState {
  const template = baseSnapshot.channels.find((channel) => channel.role === saved.role) ??
    baseSnapshot.channels.find((channel) => channel.kind === "source") ??
    baseSnapshot.channels[0];
  return applyChannelSession({
    ...(template ? structuredClone(template) : fallbackChannel()),
    id: saved.id,
    name: saved.name || saved.id,
    kind: parseKind(saved.kind),
    role: parseRole(saved.role),
    selected: false,
    meter: { left: -60, right: -60, clip: false }
  }, saved);
}

function fallbackChannel(): ChannelState {
  return {
    id: "channel",
    name: "CHANNEL",
    source: "",
    kind: "source",
    role: "music",
    selected: false,
    enabled: true,
    trimDb: 0,
    pan: 0,
    faderDb: 0,
    mute: false,
    solo: false,
    monitor: false,
    recordArm: false,
    processing: { eq: true, comp: false, noise: false, deEsser: false, insertFx: false },
    dynamics: {
      noise: { thresholdDb: -38, rangeDb: -50, holdMs: 30, releaseMs: 160 },
      compressor: { thresholdDb: -18, ratio: 3, attackMs: 10, releaseMs: 120 },
      deEsser: { frequencyHz: 6000, thresholdDb: -24, maxReductionDb: 6 }
    },
    sends: { "fx-a": { enabled: false, gainDb: -60 }, "fx-b": { enabled: false, gainDb: -60 } },
    eqBands: [],
    meter: { left: -60, right: -60, clip: false }
  };
}

function parseKind(kind: string): ChannelKind {
  return kind === "group" || kind === "master" ? kind : "source";
}

function parseRole(role: string): ChannelRole {
  if (role === "system" || role === "vocal" || role === "instrument" || role === "music" || role === "group" || role === "master") return role;
  return "music";
}

function applyFxUnitSession(unit: FxUnitState, saved: ProjectSessionFxUnit): FxUnitState {
  return {
    ...unit,
    programId: Number(saved.programId),
    revision: Number(saved.revision),
    enabled: Boolean(saved.enabled),
    modified: Boolean(saved.modified),
    returnDb: Number(saved.returnDb),
    pending: false,
    error: ""
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

function takeToSession(take: RecordedTakeState): ProjectSessionRecordedTake {
  return {
    id: take.id,
    path: take.path,
    tap: take.tap,
    sampleRate: take.sampleRate,
    channelCount: take.channels,
    frameCount: take.frames,
    replayWithNeutralInserts: take.replayWithNeutralInserts,
    partial: take.partial
  };
}

function takeSessionToSnapshot(take: ProjectSessionRecordedTake): RecordedTakeState[] {
  if (!take || typeof take.id !== "string" || typeof take.path !== "string") return [];
  return [{
    id: take.id,
    path: take.path,
    tap: parseTap(take.tap),
    sampleRate: Number(take.sampleRate) || 48000,
    channels: Number(take.channelCount) || 2,
    frames: Number(take.frameCount) || 0,
    replayWithNeutralInserts: Boolean(take.replayWithNeutralInserts),
    partial: Boolean(take.partial)
  }];
}

function parseTap(tap: string): RecordingTap {
  if (tap === "dry" || tap === "processed" || tap === "master") return tap;
  return "master";
}

function firstTakeDirectory(takes: ProjectSessionRecordedTake[]) {
  const path = takes.find((take) => typeof take.path === "string" && take.path.includes("/"))?.path;
  return path ? path.slice(0, path.lastIndexOf("/")) : "";
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

function clamp(value: number, min: number, max: number) {
  if (!Number.isFinite(value)) return min;
  return Math.max(min, Math.min(max, value));
}
