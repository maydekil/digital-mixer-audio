import { describe, expect, it } from "vitest";
import { approvedMixerSession } from "../../apps/desktop/src/fixtures/approvedMixerSession";
import { plannedTakeFromResponse, replayChannelFromTake, snapshotToRecordingPlan } from "../../apps/desktop/src/features/recording/recordingDocument";

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

  it("parses saved native take metadata", () => {
    const takes = plannedTakeFromResponse({
      saved: true,
      takeId: "take-002",
      path: "/tmp/takes/take-002.wav",
      tap: "master",
      sampleRate: 48000,
      channels: 2,
      frames: 0,
      replayWithNeutralInserts: true,
      partial: false
    });

    expect(takes[0]).toMatchObject({ id: "take-002", tap: "master", frames: 0, partial: false });
  });

  it("creates replay channel drafts for saved takes", () => {
    expect(replayChannelFromTake({
      id: "take-voice",
      path: "/tmp/takes/take-voice.wav",
      tap: "processed",
      sampleRate: 48000,
      channels: 2,
      frames: 1024,
      replayWithNeutralInserts: true,
      partial: false
    })).toEqual({
      role: "vocal",
      name: "Take take-voice",
      source: "/tmp/takes/take-voice.wav"
    });

    expect(replayChannelFromTake({
      id: "take-partial",
      path: "/tmp/takes/take-partial.wav",
      tap: "master",
      sampleRate: 48000,
      channels: 2,
      frames: 512,
      replayWithNeutralInserts: true,
      partial: true
    })).toBeNull();
  });
});
