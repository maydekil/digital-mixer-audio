import { describe, expect, it } from "vitest";
import { PreviewAdapter } from "../../apps/desktop/src/adapters/preview/PreviewAdapter";

describe("PreviewAdapter", () => {
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
    expect(voice?.faderDb).toBe(-11);
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
    adapter.updateEqBand("low", "gainDb", 8);
    adapter.selectChannel("guitar");

    expect(adapter.getSnapshot().eqBands.find((band) => band.id === "low")?.gain).toBe("+3.0 dB");
    adapter.setChannelProcessor("guitar", "eq", false);

    const snapshot = adapter.getSnapshot();
    const voice = snapshot.channels.find((channel) => channel.id === "voice");
    const guitar = snapshot.channels.find((channel) => channel.id === "guitar");
    expect(voice?.processing.noise).toBe(false);
    expect(voice?.eqBands.find((band) => band.id === "low")?.gain).toBe("+8.0 dB");
    expect(guitar?.processing.eq).toBe(false);
    expect(guitar?.eqBands.find((band) => band.id === "low")?.gain).toBe("+3.0 dB");
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

  it("updates vocal FX rack selection, bypass, and preset state", () => {
    const adapter = new PreviewAdapter();
    adapter.selectVocalFxSlot("robot");
    adapter.setVocalFxSlotEnabled("robot", true);
    adapter.applyVocalFxPreset("harmony-duo");

    const snapshot = adapter.getSnapshot();
    expect(snapshot.vocalFx.selectedSlotId).toBe("harmony");
    expect(snapshot.vocalFx.activePresetId).toBe("harmony-duo");
    expect(snapshot.vocalFx.slots.find((slot) => slot.id === "harmony")?.enabled).toBe(true);
    expect(snapshot.vocalFx.slots.find((slot) => slot.id === "robot")?.enabled).toBe(false);
  });
});
