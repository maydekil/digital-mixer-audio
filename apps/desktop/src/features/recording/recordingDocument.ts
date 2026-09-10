import type { MixerSnapshot, RecordedTakeState, RecordingTap } from "../../adapters/MixerControlPort";

export interface RecordingPlanRequest {
  schemaVersion: 1;
  projectId: string;
  directory: string;
  baseName: string;
  tap: RecordingTap;
  sampleRate: number;
  channels: number;
  armedChannelIds: string[];
  armedChannelCount: number;
}

export function snapshotToRecordingPlan(snapshot: MixerSnapshot, directory: string): RecordingPlanRequest {
  const armedChannelIds = snapshot.channels
    .filter((channel) => channel.kind === "source" && channel.recordArm)
    .map((channel) => channel.id);
  return {
    schemaVersion: 1,
    projectId: slugProjectId(snapshot.projectName),
    directory,
    baseName: `${slugProjectId(snapshot.projectName)}-${snapshot.recording.activeTap}`,
    tap: snapshot.recording.activeTap,
    sampleRate: 48000,
    channels: snapshot.recording.activeTap === "dry" ? 1 : 2,
    armedChannelIds,
    armedChannelCount: armedChannelIds.length
  };
}

export function plannedTakeFromResponse(message: Record<string, unknown>): RecordedTakeState[] {
  if (!isTakeResponse(message) || typeof message.path !== "string") return [];
  return [{
    id: typeof message.takeId === "string" ? message.takeId : "take",
    path: message.path,
    tap: parseTap(message.tap),
    sampleRate: numberField(message.sampleRate, 48000),
    channels: numberField(message.channels, 2),
    frames: numberField(message.frames, 0),
    replayWithNeutralInserts: message.replayWithNeutralInserts === true,
    partial: message.partial === true
  }];
}

function isTakeResponse(message: Record<string, unknown>) {
  return message.planned === true || message.started === true || message.saved === true;
}

function parseTap(value: unknown): RecordingTap {
  return value === "dry" || value === "processed" || value === "master" ? value : "master";
}

function numberField(value: unknown, fallback: number) {
  return typeof value === "number" && Number.isFinite(value) ? value : fallback;
}

function slugProjectId(projectName: string) {
  const slug = projectName.trim().toLowerCase().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, "");
  return slug || "local-audio-mixer";
}
