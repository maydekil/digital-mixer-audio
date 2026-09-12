import { describe, expect, it } from "vitest";
import { approvedMixerSession } from "../../apps/desktop/src/fixtures/approvedMixerSession";
import {
  projectSessionToSnapshot,
  serializeProjectSession,
  snapshotToProjectSession
} from "../../apps/desktop/src/features/project/sessionDocument";

describe("project session serialization", () => {
  it("serializes mixer snapshot into a native session-shaped document", () => {
    const document = snapshotToProjectSession(approvedMixerSession);
    const voice = document.channels.find((channel) => channel.id === "voice");
    const fxA = document.fxUnits.find((unit) => unit.unitId === "fx-a");
    const voiceSendA = document.fxSends.find((send) => send.channelId === "voice" && send.unitId === "fx-a");

    expect(document.schemaVersion).toBe(1);
    expect(document.projectId).toBe("local-audio-mixer");
    expect(document.channels).toHaveLength(6);
    expect(voice).toMatchObject({
      name: "VOICE",
      role: "vocal",
      sourceUid: "USB Mic",
      recordArm: true,
      noiseEnabled: true,
      deEsserEnabled: false,
      insertFxEnabled: true,
      noiseThresholdDb: -38,
      noiseRangeDb: -50,
      noiseHoldMs: 30,
      noiseReleaseMs: 160,
      compRatio: 3,
      compAttackMs: 10,
      compReleaseMs: 120
    });
    expect(fxA).toMatchObject({ programId: 12, enabled: true, modified: true, returnDb: -6 });
    expect(voiceSendA).toMatchObject({ enabled: true, gainDb: -18 });
    expect(document.channelHarmony[0]).toMatchObject({ channelId: "voice", key: "C", scale: "Major" });
  });

  it("emits project JSON accepted by desktop project validation", () => {
    const parsed = JSON.parse(serializeProjectSession(approvedMixerSession)) as { schemaVersion: number; projectId: string };
    expect(parsed.schemaVersion).toBe(1);
    expect(parsed.projectId).toBe("local-audio-mixer");
  });

  it("applies saved project session state back onto a mixer snapshot", () => {
    const source = structuredClone(approvedMixerSession);
    source.channels[1].source = "Headset Mic";
    source.channels[1].faderDb = -12;
      source.channels[1].processing.noise = false;
    source.channels[1].processing.deEsser = true;
    source.channels[1].dynamics.noise.holdMs = 55;
    source.channels[1].dynamics.noise.releaseMs = 220;
    source.channels[1].dynamics.compressor.ratio = 5;
    source.channels[1].dynamics.compressor.attackMs = 24;
    source.channels[1].dynamics.compressor.releaseMs = 240;
    source.fxUnits[0].enabled = false;
    source.fxUnits[0].returnDb = -18;
    source.harmony.enabled = true;
    source.harmony.primaryInstanceId = "harmony:voice:primary";
    source.channels[1].harmonyEnabled = true;

    const loaded = projectSessionToSnapshot(serializeProjectSession(source), approvedMixerSession);
    const voice = loaded.channels.find((channel) => channel.id === "voice");

    expect(loaded.projectName).toBe("local-audio-mixer");
    expect(voice?.source).toBe("Headset Mic");
    expect(voice?.faderDb).toBe(-10);
    expect(voice?.processing.noise).toBe(true);
    expect(voice?.processing.deEsser).toBe(true);
    expect(voice?.dynamics.noise.holdMs).toBe(55);
    expect(voice?.dynamics.noise.releaseMs).toBe(220);
    expect(voice?.dynamics.compressor.ratio).toBe(5);
    expect(voice?.dynamics.compressor.attackMs).toBe(24);
    expect(voice?.dynamics.compressor.releaseMs).toBe(240);
    expect(loaded.fxUnits[0]).toMatchObject({ enabled: false, returnDb: -18 });
    expect(loaded.harmony.enabled).toBe(true);
    expect(loaded.channels.find((channel) => channel.id === "voice")?.harmonyEnabled).toBe(true);
  });

  it("restores channels that are not present in the base snapshot", () => {
    const document = snapshotToProjectSession(approvedMixerSession);
    document.channels.push({
      id: "break-music",
      name: "BREAK MUSIC",
      kind: "source",
      role: "music",
      sourceUid: "/tmp/break.wav",
      enabled: true,
      muted: false,
      solo: false,
      monitor: false,
      recordArm: false,
      eqEnabled: true,
      noiseEnabled: false,
      compEnabled: false,
      deEsserEnabled: false,
      insertFxEnabled: false,
      gainDb: 0,
      faderDb: -9,
      pan: 0,
      noiseThresholdDb: -38,
      noiseRangeDb: -50,
      noiseHoldMs: 30,
      noiseReleaseMs: 160,
      compThresholdDb: -18,
      compRatio: 3,
      compAttackMs: 10,
      compReleaseMs: 120
    });
    document.fxSends.push({ channelId: "break-music", unitId: "fx-a", enabled: true, gainDb: -18 });

    const loaded = projectSessionToSnapshot(`${JSON.stringify(document)}\n`, approvedMixerSession);
    const restored = loaded.channels.find((channel) => channel.id === "break-music");

    expect(restored).toMatchObject({ name: "BREAK MUSIC", source: "/tmp/break.wav", role: "music", faderDb: -9 });
    expect(restored?.sends["fx-a"]).toMatchObject({ enabled: true, gainDb: -18 });
  });

  it("roundtrips recorded take metadata through project sessions", () => {
    const source = structuredClone(approvedMixerSession);
    source.recording.takes = [{
      id: "take-001",
      path: "/tmp/takes/take-001.wav",
      tap: "processed",
      sampleRate: 48000,
      channels: 2,
      frames: 128,
      replayWithNeutralInserts: true,
      partial: false
    }];

    const document = snapshotToProjectSession(source);
    expect(document.recordedTakes[0]).toMatchObject({
      id: "take-001",
      tap: "processed",
      channelCount: 2,
      frameCount: 128,
      replayWithNeutralInserts: true
    });

    const loaded = projectSessionToSnapshot(`${JSON.stringify(document)}\n`, approvedMixerSession);
    expect(loaded.recording.takes[0]).toMatchObject({
      id: "take-001",
      path: "/tmp/takes/take-001.wav",
      tap: "processed",
      frames: 128
    });
    expect(loaded.recording.takeDirectory).toBe("/tmp/takes");
  });
});
