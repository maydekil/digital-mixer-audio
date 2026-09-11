import { describe, expect, it } from "vitest";
import { approvedMixerSession } from "../../apps/desktop/src/fixtures/approvedMixerSession";
import { snapshotToExportRequest } from "../../apps/desktop/src/features/export/exportDocument";

describe("export workflow request", () => {
  it("builds a native export preflight payload from the mixer snapshot", () => {
    const snapshot = structuredClone(approvedMixerSession);
    const music = snapshot.channels.find((channel) => channel.id === "music");
    const voice = snapshot.channels.find((channel) => channel.id === "voice");
    if (music) music.enabled = true;
    if (voice) voice.enabled = true;
    const request = snapshotToExportRequest(snapshot, "/tmp/mix.wav");

    expect(request).toMatchObject({
      schemaVersion: 1,
      projectId: "local-audio-mixer",
      outputPath: "/tmp/mix.wav",
      format: "wav",
      sampleRate: 48000,
      durationFrames: 480000,
      blockFrames: 512,
      tailFrames: 144000,
      mediaPath: "Backing.wav",
      master: true,
      fxAReturn: true,
      fxBReturn: true,
      includeMonitorVolume: false
    });
    expect(request.liveSourceCount).toBeGreaterThan(0);
  });

  it("does not count disabled live source channels as offline blockers", () => {
    const snapshot = structuredClone(approvedMixerSession);
    snapshot.channels = snapshot.channels.map((channel) => channel.role === "vocal" || channel.role === "system" || channel.role === "instrument"
      ? { ...channel, enabled: false }
      : channel);

    expect(snapshotToExportRequest(snapshot, "/tmp/mix.wav").liveSourceCount).toBe(0);
  });

  it("omits non-WAV music sources from native render input", () => {
    const snapshot = structuredClone(approvedMixerSession);
    const music = snapshot.channels.find((channel) => channel.id === "music");
    if (music) music.source = "playlist.mp3";

    expect(snapshotToExportRequest(snapshot, "/tmp/mix.wav").mediaPath).toBe("");
  });
});
