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
});
