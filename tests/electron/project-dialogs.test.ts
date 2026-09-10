import { describe, expect, it } from "vitest";
import { normalizeProjectSavePath, validateProjectOpenPath } from "../../apps/desktop/electron/ProjectDialogs";

describe("Project dialog path helpers", () => {
  it("normalizes save paths to the Local Audio Mixer project extension", () => {
    expect(normalizeProjectSavePath("/tmp/session")).toBe("/tmp/session.lam.json");
    expect(normalizeProjectSavePath("/tmp/session.json")).toBe("/tmp/session.lam.json");
    expect(normalizeProjectSavePath("/tmp/session.lam.json")).toBe("/tmp/session.lam.json");
  });

  it("accepts only project files for open paths", () => {
    expect(validateProjectOpenPath("/tmp/session.lam.json")).toEqual({ ok: true, path: "/tmp/session.lam.json" });
    expect(validateProjectOpenPath("/tmp/session.json")).toMatchObject({ ok: false });
    expect(validateProjectOpenPath(" ")).toMatchObject({ ok: false });
  });
});
