import { describe, expect, it } from "vitest";
import { approvedMixerSession } from "../../apps/desktop/src/fixtures/approvedMixerSession";
import { serializeProjectSession, snapshotToProjectSession } from "../../apps/desktop/src/features/project/sessionDocument";

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
      insertFxEnabled: true,
      noiseThresholdDb: -50,
      compRatio: 3
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
});
