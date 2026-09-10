import { describe, expect, it } from "vitest";
import { mkdtemp, readFile, rm, writeFile } from "node:fs/promises";
import { join } from "node:path";
import { tmpdir } from "node:os";
import { normalizeExportOutputPath, validateExportOutputPath } from "../../apps/desktop/electron/ExportDialogs";
import {
  collectProjectMedia,
  inspectProjectMedia,
  normalizeProjectSavePath,
  readProjectFile,
  relinkProjectMedia,
  validateProjectJson,
  validateProjectOpenPath,
  writeProjectFile
} from "../../apps/desktop/electron/ProjectDialogs";

describe("Project dialog path helpers", () => {
  it("normalizes export output paths to WAV", () => {
    expect(normalizeExportOutputPath("/tmp/mix")).toBe("/tmp/mix.wav");
    expect(normalizeExportOutputPath("/tmp/mix.wave")).toBe("/tmp/mix.wav");
    expect(validateExportOutputPath(" ")).toMatchObject({ ok: false });
    expect(validateExportOutputPath("/tmp/mix.wav")).toEqual({ ok: true, path: "/tmp/mix.wav" });
  });

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

  it("inspects and relinks media references in project JSON", async () => {
    const directory = await mkdtemp(join(tmpdir(), "local-mixer-relink-"));
    try {
      const missing = join(directory, "missing.wav");
      const replacement = join(directory, "replacement.wav");
      await writeFile(replacement, "wav");
      const content = projectContent(missing);

      expect(inspectProjectMedia(content).media?.[0]).toMatchObject({ id: "music", exists: false, missing: true });
      expect(relinkProjectMedia(content, "music", join(directory, "none.wav"))).toMatchObject({ ok: false });

      const relinked = relinkProjectMedia(content, "music", replacement);
      expect(relinked.ok).toBe(true);
      const parsed = JSON.parse(relinked.content ?? "") as { media: Array<{ path: string; missing: boolean }>; channels: Array<{ sourceUid: string }> };
      expect(parsed.media[0]).toMatchObject({ path: replacement, missing: false });
      expect(parsed.channels[0].sourceUid).toBe(replacement);
    } finally {
      await rm(directory, { recursive: true, force: true });
    }
  });

  it("collects existing media next to the project and leaves missing media marked", async () => {
    const directory = await mkdtemp(join(tmpdir(), "local-mixer-collect-"));
    try {
      const source = join(directory, "backing.wav");
      const missing = join(directory, "gone.wav");
      const project = join(directory, "session.lam.json");
      await writeFile(source, "wav");

      const result = await collectProjectMedia(projectContent(source, missing), project);
      expect(result.ok).toBe(true);
      expect(result.collected).toHaveLength(1);
      expect(result.missing).toHaveLength(1);

      const copiedPath = result.collected?.[0].to ?? "";
      await expect(readFile(copiedPath, "utf8")).resolves.toBe("wav");
      const parsed = JSON.parse(result.content ?? "") as { media: Array<{ id: string; path: string; missing: boolean }>; channels: Array<{ id: string; sourceUid: string }> };
      expect(parsed.media.find((item) => item.id === "music")?.path).toBe(copiedPath);
      expect(parsed.channels.find((item) => item.id === "music")?.sourceUid).toBe(copiedPath);
      expect(parsed.media.find((item) => item.id === "missing")?.missing).toBe(true);
    } finally {
      await rm(directory, { recursive: true, force: true });
    }
  });
});

function projectContent(mediaPath: string, missingPath?: string) {
  const media = [
    { id: "music", path: mediaPath, missing: false },
    ...(missingPath ? [{ id: "missing", path: missingPath, missing: false }] : [])
  ];
  const channels = [
    { id: "music", sourceUid: mediaPath },
    ...(missingPath ? [{ id: "missing", sourceUid: missingPath }] : [])
  ];
  return JSON.stringify({ schemaVersion: 1, projectId: "project-a", media, channels });
}
