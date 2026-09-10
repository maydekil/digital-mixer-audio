import { describe, expect, it } from "vitest";
import { mkdtemp, rm } from "node:fs/promises";
import { join } from "node:path";
import { tmpdir } from "node:os";
import {
  normalizeProjectSavePath,
  readProjectFile,
  validateProjectJson,
  validateProjectOpenPath,
  writeProjectFile
} from "../../apps/desktop/electron/ProjectDialogs";

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

  it("validates project JSON documents before writing", () => {
    expect(validateProjectJson("{\"schemaVersion\":1,\"projectId\":\"project-a\"}")).toEqual({ ok: true });
    expect(validateProjectJson("{\"schemaVersion\":1,\"projectId\":\"\"}")).toMatchObject({ ok: false });
    expect(validateProjectJson("{")).toMatchObject({ ok: false });
  });

  it("writes and reads project files atomically", async () => {
    const directory = await mkdtemp(join(tmpdir(), "local-mixer-project-"));
    try {
      const target = join(directory, "session.json");
      const content = "{\"schemaVersion\":1,\"projectId\":\"project-a\",\"media\":[]}";
      const written = await writeProjectFile(target, content);
      expect(written).toEqual({ ok: true, path: join(directory, "session.lam.json") });

      const loaded = await readProjectFile(written.path ?? "");
      expect(loaded).toMatchObject({ ok: true, path: written.path, content });

      await expect(readProjectFile(join(directory, "session.json"))).resolves.toMatchObject({ ok: false });
      await expect(writeProjectFile(join(directory, "bad"), "{")).resolves.toMatchObject({ ok: false });
    } finally {
      await rm(directory, { recursive: true, force: true });
    }
  });
});
