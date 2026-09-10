import { describe, expect, it } from "vitest";
import { relinkMissingProjectMedia, type ProjectMediaRelinkBridge } from "../../apps/desktop/src/features/project/projectMediaWorkflow";

describe("project media relink workflow", () => {
  it("relinks missing media references through the injected native bridge", async () => {
    const calls: string[] = [];
    const bridge: ProjectMediaRelinkBridge = {
      async inspectProjectMedia(content) {
        const missing = content.includes("missing.wav");
        return { ok: true, media: [{ id: "music", path: missing ? "/old/missing.wav" : "/new/backing.wav", exists: !missing, missing }] };
      },
      async relinkProjectMedia(content, mediaId, path) {
        calls.push(`${mediaId}:${path}`);
        return { ok: true, content: content.replace("/old/missing.wav", path) };
      },
      async chooseReplacement() {
        return { ok: true, path: "/new/backing.wav" };
      }
    };

    const result = await relinkMissingProjectMedia("{\"path\":\"/old/missing.wav\"}", bridge);

    expect(result).toMatchObject({ ok: true, relinked: 1, missing: [] });
    expect(result.content).toContain("/new/backing.wav");
    expect(calls).toEqual(["music:/new/backing.wav"]);
  });

  it("leaves unresolved media marked when the replacement picker is canceled", async () => {
    const bridge: ProjectMediaRelinkBridge = {
      async inspectProjectMedia() {
        return { ok: true, media: [{ id: "music", path: "/old/missing.wav", exists: false, missing: true }] };
      },
      async relinkProjectMedia() {
        throw new Error("should not relink after cancel");
      },
      async chooseReplacement() {
        return { ok: true, canceled: true };
      }
    };

    const result = await relinkMissingProjectMedia("content", bridge);

    expect(result).toMatchObject({ ok: true, relinked: 0, canceled: true });
    expect(result.missing).toHaveLength(1);
    expect(result.content).toBe("content");
  });

  it("returns a relink error without mutating later references", async () => {
    const bridge: ProjectMediaRelinkBridge = {
      async inspectProjectMedia() {
        return { ok: true, media: [{ id: "music", path: "/old/missing.wav", exists: false, missing: true }] };
      },
      async relinkProjectMedia() {
        return { ok: false, error: "Replacement media file does not exist" };
      },
      async chooseReplacement() {
        return { ok: true, path: "/new/backing.wav" };
      }
    };

    const result = await relinkMissingProjectMedia("content", bridge);

    expect(result).toMatchObject({ ok: false, relinked: 0, error: "Replacement media file does not exist" });
    expect(result.content).toBe("content");
  });
});
