import { describe, expect, it } from "vitest";
import { PreviewAdapter } from "../../apps/desktop/src/adapters/preview/PreviewAdapter";

describe("PreviewAdapter", () => {
  it("starts every channel fader at unity gain", () => {
    const adapter = new PreviewAdapter();
    const snapshot = adapter.getSnapshot();
    expect(snapshot.channels.map((channel) => [channel.id, channel.faderDb])).toEqual([
      ["system", 0],
      ["voice", 0],
      ["guitar", 0],
      ["music", 0],
      ["group1", 0],
      ["master", 0]
    ]);
  });

  it("starts with only the master output channel enabled", () => {
    const adapter = new PreviewAdapter();
    const snapshot = adapter.getSnapshot();
    expect(snapshot.channels.map((channel) => [channel.id, channel.enabled])).toEqual([
      ["system", false],
      ["voice", false],
      ["guitar", false],
      ["music", false],
      ["group1", false],
      ["master", true]
    ]);
  });

  it("keeps selected channel send A linked to the same state", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelSend("voice", "fx-a", -12);
    const voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");
    expect(voice?.sends["fx-a"].gainDb).toBe(-12);
  });

  it("does not reset sends, return, or enabled state when selecting an FX program", () => {
    const adapter = new PreviewAdapter();
    adapter.setFxProgram("fx-a", 22);
    const snapshot = adapter.getSnapshot();
    const fxA = snapshot.fxUnits.find((unit) => unit.id === "fx-a");
    const voice = snapshot.channels.find((channel) => channel.id === "voice");
    expect(fxA?.programId).toBe(22);
    expect(fxA?.enabled).toBe(true);
    expect(fxA?.returnDb).toBe(-6);
    expect(voice?.sends["fx-a"].gainDb).toBe(-18);
  });

  it("toggles only the vocal harmony shortcut state", () => {
    const adapter = new PreviewAdapter();
    adapter.setHarmonyEnabled(false);
    const snapshot = adapter.getSnapshot();
    expect(snapshot.harmony.enabled).toBe(false);
    expect(snapshot.channels.find((channel) => channel.id === "voice")?.harmonyEnabled).toBe(false);
    expect(snapshot.channels.find((channel) => channel.id === "music")?.harmonyEnabled).toBe(false);
  });

  it("updates channel knobs, faders, and toggle buttons", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelTrim("voice", 4.5);
    adapter.setChannelPan("voice", -35);
    adapter.setChannelFader("voice", -11);
    adapter.setChannelEnabled("voice", false);
    adapter.setChannelSource("voice", "BuiltInHeadphoneInputDevice");
    adapter.setChannelMute("voice", true);
    adapter.setChannelSolo("voice", true);
    adapter.setChannelMonitor("voice", false);
    adapter.setChannelRecordArm("voice", false);

    const voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");
    expect(voice?.trimDb).toBe(4.5);
    expect(voice?.pan).toBe(-35);
    expect(voice?.faderDb).toBe(-10);
    expect(voice?.enabled).toBe(false);
    expect(voice?.source).toBe("BuiltInHeadphoneInputDevice");
    expect(voice?.meter.left).toBe(-60);
    expect(voice?.mute).toBe(true);
    expect(voice?.solo).toBe(true);
    expect(voice?.monitor).toBe(false);
    expect(voice?.recordArm).toBe(false);
  });

  it("updates FX return and EQ parameter display values", () => {
    const adapter = new PreviewAdapter();
    adapter.setFxReturn("fx-a", -18);
    adapter.setFxProgramMacro("fx-a", "macro1", "2.2 s");
    adapter.updateEqBand("mid1", "freqHz", 1200);
    adapter.updateEqBand("mid1", "gainDb", 4.5);
    adapter.updateEqBand("mid1", "qValue", 2);

    const snapshot = adapter.getSnapshot();
    expect(snapshot.fxUnits.find((unit) => unit.id === "fx-a")?.returnDb).toBe(-18);
    expect(snapshot.fxUnits.find((unit) => unit.id === "fx-a")?.modified).toBe(true);
    expect(snapshot.programs.find((program) => program.id === 12)?.macro1.value).toBe("2.2 s");
    const mid1 = snapshot.eqBands.find((band) => band.id === "mid1");
    expect(mid1?.freq).toBe("1.2 kHz");
    expect(mid1?.gain).toBe("+4.5 dB");
    expect(mid1?.q).toBe("2.00");
  });

  it("updates selected channel dynamics parameters", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelNoiseParam("voice", "thresholdDb", -46);
    adapter.setChannelNoiseParam("voice", "rangeDb", -70);
    adapter.setChannelNoiseParam("voice", "holdMs", 45);
    adapter.setChannelNoiseParam("voice", "releaseMs", 180);
    adapter.setChannelCompressorParam("voice", "thresholdDb", -22);
    adapter.setChannelCompressorParam("voice", "ratio", 4);
    adapter.setChannelCompressorParam("voice", "attackMs", 25);
    adapter.setChannelCompressorParam("voice", "releaseMs", 250);
    adapter.setChannelProcessor("voice", "deEsser", true);
    adapter.setChannelDeEsserParam("voice", "frequencyHz", 7200);

    const voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");
    expect(voice?.dynamics.noise.thresholdDb).toBe(-46);
    expect(voice?.dynamics.noise.rangeDb).toBe(-70);
    expect(voice?.dynamics.noise.holdMs).toBe(45);
    expect(voice?.dynamics.noise.releaseMs).toBe(180);
    expect(voice?.dynamics.compressor.thresholdDb).toBe(-22);
    expect(voice?.dynamics.compressor.ratio).toBe(4);
    expect(voice?.dynamics.compressor.attackMs).toBe(25);
    expect(voice?.dynamics.compressor.releaseMs).toBe(250);
    expect(voice?.processing.deEsser).toBe(true);
    expect(voice?.dynamics.deEsser.frequencyHz).toBe(7200);
  });

  it("maps one vocal noise knob onto safe gate parameters", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelNoiseAmount("voice", 70);
    let voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");

    expect(voice?.processing.noise).toBe(true);
    expect(voice?.dynamics.noise).toEqual({
      thresholdDb: -24.6,
      rangeDb: -84.6,
      holdMs: 32,
      releaseMs: 98
    });

    adapter.setChannelNoiseAmount("voice", 0);
    voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");
    expect(voice?.processing.noise).toBe(true);
    expect(voice?.dynamics.noise).toEqual({
      thresholdDb: -55,
      rangeDb: -32,
      holdMs: 90,
      releaseMs: 180
    });
  });

  it("loads a native FX bank without changing existing unit choices", () => {
    const adapter = new PreviewAdapter();
    adapter.setPrograms([
      { id: 12, name: "Vocal Plate", family: "Plate", macro1: { label: "Decay", value: "1.4 s" }, macro2: { label: "Pre-delay", value: "20 ms" } },
      { id: 50, name: "Stereo 320", family: "Stereo Delay", macro1: { label: "Time", value: "320 ms" }, macro2: { label: "Feedback", value: "25 %" } }
    ]);

    const snapshot = adapter.getSnapshot();
    expect(snapshot.programs).toHaveLength(2);
    expect(snapshot.fxUnits.find((unit) => unit.id === "fx-a")?.programId).toBe(12);
    expect(snapshot.fxUnits.find((unit) => unit.id === "fx-b")?.programId).toBe(50);
  });

  it("keeps EQ edits and processor toggles scoped to the selected channel", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelProcessor("voice", "noise", false);
    adapter.setChannelProcessor("voice", "deEsser", true);
    adapter.updateEqBand("low", "gainDb", 8);
    adapter.selectChannel("guitar");

    expect(adapter.getSnapshot().eqBands.find((band) => band.id === "low")?.gain).toBe("+0.0 dB");
    adapter.setChannelProcessor("guitar", "eq", false);
    adapter.setChannelEqBand("guitar", "mid2", "gainDb", -3.5);

    const snapshot = adapter.getSnapshot();
    const voice = snapshot.channels.find((channel) => channel.id === "voice");
    const guitar = snapshot.channels.find((channel) => channel.id === "guitar");
    expect(voice?.processing.noise).toBe(true);
    expect(voice?.processing.deEsser).toBe(true);
    expect(voice?.eqBands.find((band) => band.id === "low")?.gain).toBe("+8.0 dB");
    expect(guitar?.processing.eq).toBe(true);
    expect(guitar?.eqBands.find((band) => band.id === "low")?.gain).toBe("+0.0 dB");
    expect(guitar?.eqBands.find((band) => band.id === "mid2")?.gain).toBe("-3.5 dB");
  });

  it("resets selected channel EQ bands to the neutral music start point", () => {
    const adapter = new PreviewAdapter();
    adapter.updateEqBand("low", "gainDb", 8);
    adapter.updateEqBand("mid1", "freqHz", 1200);

    adapter.resetEqBands();
    const snapshot = adapter.getSnapshot();
    const selected = snapshot.channels.find((channel) => channel.id === snapshot.selectedChannelId);
    const low = selected?.eqBands.find((band) => band.id === "low");
    const mid1 = selected?.eqBands.find((band) => band.id === "mid1");

    expect(low).toMatchObject({ freqHz: 100, gainDb: 0, gain: "+0.0 dB" });
    expect(mid1).toMatchObject({ freqHz: 350, gainDb: 0, qValue: 1 });
    expect(snapshot.eqBands.find((band) => band.id === "mid1")?.freq).toBe("350 Hz");
  });

  it("can reset master output EQ without changing the selected strip", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelEqBand("master", "low", "gainDb", 6);
    adapter.resetChannelEqBands("master");

    const snapshot = adapter.getSnapshot();
    const master = snapshot.channels.find((channel) => channel.id === "master");
    expect(snapshot.selectedChannelId).toBe("voice");
    expect(master?.eqBands.find((band) => band.id === "low")).toMatchObject({ freqHz: 100, gainDb: 0, gain: "+0.0 dB" });
  });

  it("keeps system channel noise, insert FX, and sends disabled", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelProcessor("system", "noise", true);
    adapter.setChannelProcessor("system", "insertFx", true);
    adapter.setChannelSend("system", "fx-a", -6);
    adapter.setChannelSend("system", "fx-b", -6);

    const system = adapter.getSnapshot().channels.find((channel) => channel.id === "system");
    expect(system?.processing.noise).toBe(false);
    expect(system?.processing.insertFx).toBe(false);
    expect(system?.sends["fx-a"]).toEqual({ enabled: false, gainDb: -60 });
    expect(system?.sends["fx-b"]).toEqual({ enabled: false, gainDb: -60 });
  });

  it("bypasses channel FX sends while insert FX is disabled", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelProcessor("voice", "insertFx", false);
    adapter.setChannelSend("voice", "fx-a", -6);
    adapter.setChannelSend("voice", "fx-b", -6);

    const voice = adapter.getSnapshot().channels.find((channel) => channel.id === "voice");
    expect(voice?.sends["fx-a"]).toEqual({ enabled: false, gainDb: -60 });
    expect(voice?.sends["fx-b"]).toEqual({ enabled: false, gainDb: -60 });
  });

  it("keeps source monitoring exclusive until aggregate routing is available", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelMonitor("voice", true);
    adapter.setChannelMonitor("guitar", true);

    const snapshot = adapter.getSnapshot();
    expect(snapshot.channels.find((channel) => channel.id === "voice")?.monitor).toBe(false);
    expect(snapshot.channels.find((channel) => channel.id === "guitar")?.monitor).toBe(true);
    expect(snapshot.channels.find((channel) => channel.id === "music")?.monitor).toBe(false);
  });

  it("auto-arms source monitoring from the channel on/off state", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelEnabled("voice", true);
    adapter.setChannelEnabled("guitar", true);

    let snapshot = adapter.getSnapshot();
    expect(snapshot.channels.find((channel) => channel.id === "voice")?.monitor).toBe(false);
    expect(snapshot.channels.find((channel) => channel.id === "guitar")?.monitor).toBe(true);

    adapter.setChannelEnabled("guitar", false);
    snapshot = adapter.getSnapshot();
    expect(snapshot.channels.find((channel) => channel.id === "guitar")?.monitor).toBe(false);
  });

  it("adds, renames, and removes source channels while keeping selection valid", () => {
    const adapter = new PreviewAdapter();
    const channelId = adapter.addSourceChannel("music", "Break Music", "/tmp/break.wav");
    adapter.renameChannel(channelId, "BREAK");

    let snapshot = adapter.getSnapshot();
    expect(snapshot.selectedChannelId).toBe(channelId);
    expect(snapshot.channels.find((channel) => channel.id === channelId)).toMatchObject({
      name: "BREAK",
      role: "music",
      source: "/tmp/break.wav",
      kind: "source"
    });
    expect(snapshot.channels.findIndex((channel) => channel.id === channelId)).toBeLessThan(
      snapshot.channels.findIndex((channel) => channel.kind === "master")
    );

    adapter.removeChannel(channelId);
    snapshot = adapter.getSnapshot();
    expect(snapshot.channels.some((channel) => channel.id === channelId)).toBe(false);
    expect(snapshot.channels.some((channel) => channel.id === snapshot.selectedChannelId)).toBe(true);
  });

  it("tracks armed channels and planned recording take metadata", () => {
    const adapter = new PreviewAdapter();
    adapter.setChannelRecordArm("voice", false);
    adapter.setChannelRecordArm("guitar", true);
    adapter.addRecordedTakes([{
      id: "take-001",
      path: "/tmp/takes/take-001.wav",
      tap: "master",
      sampleRate: 48000,
      channels: 2,
      frames: 0,
      replayWithNeutralInserts: true,
      partial: false
    }], ["guitar"], "/tmp/takes");

    const recording = adapter.getSnapshot().recording;
    expect(recording.status).toBe("planned");
    expect(recording.armedChannelIds).toEqual(["guitar"]);
    expect(recording.takeDirectory).toBe("/tmp/takes");
    expect(recording.takes[0]).toMatchObject({ id: "take-001", tap: "master", replayWithNeutralInserts: true });
  });

  it("updates vocal FX rack selection, bypass, and preset state", () => {
    const adapter = new PreviewAdapter();
    adapter.selectVocalFxSlot("robot");
    adapter.setVocalFxSlotEnabled("robot", true);
    adapter.applyVocalFxPreset("voice-10-harmony-duo");

    const snapshot = adapter.getSnapshot();
    expect(snapshot.vocalFx.presets).toHaveLength(99);
    expect(snapshot.vocalFx.selectedSlotId).toBe("harmony");
    expect(snapshot.vocalFx.activePresetId).toBe("voice-10-harmony-duo");
    expect(snapshot.vocalFx.slots.find((slot) => slot.id === "harmony")?.enabled).toBe(true);
    expect(snapshot.vocalFx.slots.find((slot) => slot.id === "robot")?.enabled).toBe(false);
  });
});
