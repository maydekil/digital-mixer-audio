import { describe, expect, it } from "vitest";
import { approvedMixerSession } from "../../apps/desktop/src/fixtures/approvedMixerSession";
import { plannedTakeFromResponse, snapshotToRecordingPlan } from "../../apps/desktop/src/features/recording/recordingDocument";

describe("recording workflow document", () => {
  it("builds a native recording preflight payload from armed channels", () => {
    const request = snapshotToRecordingPlan(approvedMixerSession, "/tmp/takes");

    expect(request).toMatchObject({
      schemaVersion: 1,
      projectId: "local-audio-mixer",
      directory: "/tmp/takes",
      baseName: "local-audio-mixer-master",
      tap: "master",
      sampleRate: 48000,
      channels: 2,
      armedChannelIds: ["voice"],
      armedChannelCount: 1
    });
  });

  it("parses planned native take metadata", () => {
    const takes = plannedTakeFromResponse({
      planned: true,
      takeId: "take-001",
      path: "/tmp/takes/take-001.wav",
      tap: "processed",
      sampleRate: 48000,
      channels: 2,
      frames: 0,
      replayWithNeutralInserts: true,
      partial: false
    });

    expect(takes).toEqual([{
      id: "take-001",
      path: "/tmp/takes/take-001.wav",
      tap: "processed",
      sampleRate: 48000,
      channels: 2,
      frames: 0,
      replayWithNeutralInserts: true,
      partial: false
    }]);
  });
});
