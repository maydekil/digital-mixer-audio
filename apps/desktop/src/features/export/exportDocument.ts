import type { MixerSnapshot } from "../../adapters/MixerControlPort";

export interface ExportWorkflowRequest {
  schemaVersion: 1;
  projectId: string;
  outputPath: string;
  format: "wav";
  sampleRate: number;
  durationFrames: number;
  blockFrames: number;
  mediaPath: string;
  master: boolean;
  fxAReturn: boolean;
  fxBReturn: boolean;
  includeMonitorVolume: false;
  liveSourceCount: number;
}

export function snapshotToExportRequest(snapshot: MixerSnapshot, outputPath: string): ExportWorkflowRequest {
  return {
    schemaVersion: 1,
    projectId: slugProjectId(snapshot.projectName),
    outputPath,
    format: "wav",
    sampleRate: 48000,
    durationFrames: 48000 * 10,
    blockFrames: 512,
    mediaPath: firstOfflineMediaPath(snapshot),
    master: true,
    fxAReturn: snapshot.fxUnits.some((unit) => unit.id === "fx-a" && unit.enabled),
    fxBReturn: snapshot.fxUnits.some((unit) => unit.id === "fx-b" && unit.enabled),
    includeMonitorVolume: false,
    liveSourceCount: countLiveSources(snapshot)
  };
}

function firstOfflineMediaPath(snapshot: MixerSnapshot) {
  return snapshot.channels.find((channel) => (
    channel.kind === "source" &&
    channel.enabled &&
    channel.role === "music" &&
    /\.(wav|wave)$/i.test(channel.source.trim())
  ))?.source.trim() ?? "";
}

function countLiveSources(snapshot: MixerSnapshot) {
  return snapshot.channels.filter((channel) => {
    if (channel.kind !== "source" || !channel.enabled) return false;
    return channel.role === "system" || channel.role === "vocal" || channel.role === "instrument";
  }).length;
}

function slugProjectId(projectName: string) {
  const slug = projectName.trim().toLowerCase().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, "");
  return slug || "local-audio-mixer";
}
