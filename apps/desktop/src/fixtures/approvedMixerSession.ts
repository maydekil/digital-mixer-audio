import type { MixerSnapshot } from "../adapters/MixerControlPort";
import { fxPrograms } from "./fxPrograms";

const baseEqBands = [
  { id: "low", label: "LOW", color: "#58F28A", freqHz: 100, gainDb: 3, freq: "100 Hz", gain: "+3.0 dB", type: "Shelf" },
  { id: "mid1", label: "MID 1", color: "#FFB843", freqHz: 350, gainDb: -2.5, qValue: 1.2, freq: "350 Hz", gain: "-2.5 dB", q: "1.20" },
  { id: "mid2", label: "MID 2", color: "#1FA8FF", freqHz: 2500, gainDb: 2, qValue: 1, freq: "2.5 kHz", gain: "+2.0 dB", q: "1.00" },
  { id: "high", label: "HIGH", color: "#B862F0", freqHz: 10000, gainDb: 4, freq: "10.0 kHz", gain: "+4.0 dB", type: "Shelf" }
] as const;

export const approvedMixerSession: MixerSnapshot = {
  modeLabel: "UI PREVIEW · Audio engine not connected",
  projectName: "Local Audio Mixer",
  transportTime: "00:01:24",
  sampleRateLabel: "48 kHz",
  engineStatus: "Preview Ready",
  selectedChannelId: "voice",
  programs: fxPrograms,
  fxUnits: [
    { id: "fx-a", label: "FX A", accent: "amber", enabled: true, programId: 12, modified: true, returnDb: -6, meter: { left: -9, right: -10, clip: false } },
    { id: "fx-b", label: "FX B", accent: "cyan", enabled: true, programId: 50, modified: false, returnDb: -12, meter: { left: -12, right: -13, clip: false } }
  ],
  channels: [
    channel("system", "SYSTEM", "BlackHole 1-2", "system", -6, -12, -12, false, false),
    channel("voice", "VOICE", "USB Mic", "vocal", -3, -18, -24, true, true),
    channel("guitar", "GUITAR", "Interface In 2", "instrument", -8, -18, -6, false, false),
    channel("music", "MUSIC", "Backing.wav", "music", -9, 0, 0, false, false),
    channel("group1", "GROUP 1", "Music Bus", "group", -4, -6, -6, false, false),
    {
      id: "master", name: "MASTER", source: "Output 1-2", kind: "master", role: "master",
      enabled: true, trimDb: 0, pan: 0, faderDb: -1, mute: false, solo: false,
      processing: { eq: true, comp: false, noise: false, insertFx: false },
      sends: { "fx-a": { enabled: false, gainDb: 0 }, "fx-b": { enabled: false, gainDb: 0 } },
      eqBands: cloneEqBands(),
      meter: { left: -4, right: -5, clip: true }
    }
  ],
  eqBands: cloneEqBands(),
  harmony: { enabled: true, key: "C", scale: "Major", voice1: "+3rd", voice2: "+5th", levelDb: 0 }
};

function channel(id: string, name: string, source: string, role: "system" | "vocal" | "instrument" | "music" | "group", faderDb: number, sendA: number, sendB: number, mon: boolean, rec: boolean) {
  return {
    id, name, source, kind: role === "group" ? "group" as const : "source" as const, role,
    selected: id === "voice", enabled: true, trimDb: 0, pan: 0, faderDb, mute: false, solo: false,
    monitor: mon, recordArm: rec, harmonyVisible: role === "vocal", harmonyEnabled: role === "vocal",
    processing: { eq: true, comp: role === "vocal", noise: role === "vocal", insertFx: role === "vocal" },
    sends: {
      "fx-a": { enabled: sendA !== 0, gainDb: sendA },
      "fx-b": { enabled: sendB !== 0, gainDb: sendB }
    },
    eqBands: cloneEqBands(),
    meter: { left: Math.max(-32, faderDb - 2), right: Math.max(-32, faderDb - 4), clip: id === "voice" || id === "guitar" || id === "music" }
  };
}

function cloneEqBands() {
  return baseEqBands.map((band) => ({ ...band }));
}
