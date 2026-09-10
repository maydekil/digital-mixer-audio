export interface ProjectMediaStatus {
  id: string;
  path: string;
  exists: boolean;
  missing: boolean;
}

export interface ProjectMediaRelinkBridge {
  inspectProjectMedia(content: string): Promise<{ ok: boolean; media?: ProjectMediaStatus[]; error?: string }>;
  relinkProjectMedia(content: string, mediaId: string, path: string): Promise<{ ok: boolean; content?: string; error?: string }>;
  chooseReplacement(media: ProjectMediaStatus): Promise<{ ok: boolean; path?: string; canceled?: boolean; error?: string }>;
}

export interface ProjectMediaRelinkResult {
  ok: boolean;
  content: string;
  relinked: number;
  missing: ProjectMediaStatus[];
  canceled?: boolean;
  error?: string;
}

export async function relinkMissingProjectMedia(
  content: string,
  bridge: ProjectMediaRelinkBridge
): Promise<ProjectMediaRelinkResult> {
  const inspected = await bridge.inspectProjectMedia(content);
  if (!inspected.ok) {
    return { ok: false, content, relinked: 0, missing: [], error: inspected.error ?? "PROJECT_MEDIA_INSPECT_FAILED" };
  }

  let nextContent = content;
  let relinked = 0;
  for (const media of (inspected.media ?? []).filter((item) => item.missing || !item.exists)) {
    const replacement = await bridge.chooseReplacement(media);
    if (replacement.canceled) continue;
    if (!replacement.ok || !replacement.path) {
      return {
        ok: false,
        content: nextContent,
        relinked,
        missing: [media],
        error: replacement.error ?? "PROJECT_MEDIA_RELINK_CANCELED"
      };
    }
    const result = await bridge.relinkProjectMedia(nextContent, media.id, replacement.path);
    if (!result.ok || !result.content) {
      return {
        ok: false,
        content: nextContent,
        relinked,
        missing: [media],
        error: result.error ?? "PROJECT_MEDIA_RELINK_FAILED"
      };
    }
    nextContent = result.content;
    relinked += 1;
  }

  const verified = await bridge.inspectProjectMedia(nextContent);
  const missing = (verified.media ?? []).filter((item) => item.missing || !item.exists);
  return { ok: true, content: nextContent, relinked, missing, canceled: relinked === 0 && missing.length > 0 };
}
