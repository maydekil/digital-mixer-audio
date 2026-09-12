import type { MixerSnapshot } from "../adapters/MixerControlPort";
import { fxPrograms } from "./fxPrograms";
import { vocalFxPresets } from "./vocalFxPresets";

const baseEqBands = [
  { id: "low", label: "LOW", color: "#58F28A", freqHz: 100, gainDb: 0, freq: "100 Hz", gain: "+0.0 dB", type: "Shelf" },
  { id: "mid1", label: "MID 1", color: "#FFB843", freqHz: 350, gainDb: 0, qValue: 1, freq: "350 Hz", gain: "+0.0 dB", q: "1.00" },
  { id: "mid2", label: "MID 2", color: "#1FA8FF", freqHz: 2500, gainDb: 0, qValue: 1, freq: "2.5 kHz", gain: "+0.0 dB", q: "1.00" },
  { id: "high", label: "HIGH", color: "#B862F0", freqHz: 10000, gainDb: 0, freq: "10.0 kHz", gain: "+0.0 dB", type: "Shelf" }
] as const;

const defaultDynamics = {
  noise: { thresholdDb: -38, rangeDb: -50, holdMs: 30, releaseMs: 160 },
  compressor: { thresholdDb: -18, ratio: 3, attackMs: 10, releaseMs: 120 },
  deEsser: { frequencyHz: 6000, thresholdDb: -24, maxReductionDb: 6 }
};

export const approvedMixerSession: MixerSnapshot = {
  modeLabel: "UI PREVIEW · Audio engine not connected",
  projectName: "Local Audio Mixer",
  transportTime: "00:01:24",
  sampleRateLabel: "48 kHz",
  engineStatus: "Preview Ready",
  selectedChannelId: "voice",
  programs: fxPrograms,
  fxUnits: [
    { id: "fx-a", label: "FX A", accent: "amber", enabled: true, programId: 12, revision: 0, pending: false, error: "", modified: true, returnDb: -6, meter: { left: -9, right: -10, clip: false } },
    { id: "fx-b", label: "FX B", accent: "cyan", enabled: true, programId: 50, revision: 0, pending: false, error: "", modified: false, returnDb: -12, meter: { left: -12, right: -13, clip: false } }
  ],
  channels: [
    channel("system", "SYSTEM", "BlackHole 1-2", "system", 0, 0, 0, false, false),
    channel("voice", "VOICE", "USB Mic", "vocal", 0, -18, -24, true, true),
    channel("guitar", "GUITAR", "Interface In 2", "instrument", 0, -18, -6, false, false),
    channel("music", "MUSIC", "Backing.wav", "music", 0, 0, 0, false, false),
    channel("group1", "GROUP 1", "Music Bus", "group", 0, -6, -6, false, false),
    {
      id: "master", name: "MASTER", source: "Output 1-2", kind: "master", role: "master",
      enabled: true, trimDb: 0, pan: 0, faderDb: 0, mute: false, solo: false,
      processing: { eq: true, comp: false, noise: false, deEsser: false, insertFx: false },
      dynamics: cloneDynamics(),
      sends: { "fx-a": { enabled: false, gainDb: 0 }, "fx-b": { enabled: false, gainDb: 0 } },
      eqBands: cloneEqBands(),
      meter: { left: -4, right: -5, clip: true }
    }
  ],
  eqBands: cloneEqBands(),
  harmony: { enabled: false, effectiveEnabled: false, pending: false, error: "", revision: 0, primaryInstanceId: "", key: "C", scale: "Major", mode: "Diatonic", voice1: "+3rd", voice2: "+5th", levelDb: 0 },
  recording: { status: "idle", activeTap: "master", takeDirectory: "", error: "", armedChannelIds: ["voice"], takes: [] },
  vocalFx: {
    selectedSlotId: "pitch-correct",
    activePresetId: "voice-03-studio-pop",
    presets: vocalFxPresets,
    slots: [
      slot("pitch-correct", "pitch_correct", "Pitch Correction", "Pitch", false, 1, 92, [["Key", "C"], ["Scale", "Major"], ["Retune", "80 ms"], ["Amount", "70%"]]),
      slot("formant-shift", "formant_shift", "Formant Shift", "Pitch", false, 1, 58, [["Shift", "+0 st"], ["Mix", "100%"]]),
      slot("doubler", "doubler", "Vocal Doubler", "Modulation", false, 1, 0, [["Voice 1", "-6 cent"], ["Voice 2", "+6 cent"], ["Level", "-9 dB"]]),
      slot("plate", "reverb", "Plate Reverb", "Space", false, 0.15, 0, [["Decay", "1.4 s"], ["Pre-delay", "20 ms"], ["Mix", "15%"]]),
      slot("stereo-delay", "delay", "Stereo Delay", "Space", false, 0.12, 0, [["Time", "250 ms"], ["Feedback", "20%"], ["Mix", "12%"]]),
      slot("harmony", "harmony", "Harmony Duo", "Pitch", false, 1, 92, [["Voice 1", "+3rd"], ["Voice 2", "+5th"], ["Level", "-12 dB"]]),
      slot("robot", "vocoder", "Robot Voice", "Synth", false, 1, 0, [["Carrier", "C2"], ["Bands", "16"], ["Wet", "100%"]]),
      slot("saturation", "saturation", "Saturation", "Character", false, 1, 0, [["Drive", "3 dB"], ["Output", "-3 dB"]])
    ]
  }
};

function channel(id: string, name: string, source: string, role: "system" | "vocal" | "instrument" | "music" | "group", faderDb: number, sendA: number, sendB: number, mon: boolean, rec: boolean) {
  return {
    id, name, source, kind: role === "group" ? "group" as const : "source" as const, role,
    selected: id === "voice", enabled: false, trimDb: 0, pan: 0, faderDb, mute: false, solo: false,
    monitor: mon, recordArm: rec, harmonyVisible: role === "vocal", harmonyEnabled: false,
    processing: { eq: true, comp: role === "vocal", noise: role === "vocal", deEsser: false, insertFx: false },
    dynamics: cloneDynamics(),
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

function cloneDynamics() {
  return structuredClone(defaultDynamics);
}

function slot(id: string, effectType: string, label: string, category: "Pitch" | "Space" | "Modulation" | "Character" | "Synth", enabled: boolean, mix: number, latencyMs: number, params: Array<[string, string]>) {
  return {
    id,
    effectType,
    label,
    category,
    enabled,
    mix,
    latencyMs,
    availability: "implemented_unverified" as const,
    parameters: params.map(([paramLabel, value]) => ({ id: `${id}-${paramLabel.toLowerCase().replaceAll(" ", "-")}`, label: paramLabel, value }))
  };
}
