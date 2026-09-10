import { existsSync } from "node:fs";
import { copyFile, mkdir, readFile, rename, writeFile } from "node:fs/promises";
import { basename, dirname, extname, join, parse } from "node:path";

export const projectFileExtension = ".lam.json";

export interface ProjectPathResult {
  ok: boolean;
  canceled?: boolean;
  path?: string;
  error?: string;
}

export interface ProjectMediaStatus {
  id: string;
  path: string;
  exists: boolean;
  missing: boolean;
}

export interface ProjectMediaCollectResult extends ProjectPathResult {
  content?: string;
  collected?: Array<{ id: string; from: string; to: string }>;
  missing?: ProjectMediaStatus[];
}

export function normalizeProjectSavePath(path: string): string {
  const trimmed = path.trim();
  if (trimmed.toLowerCase().endsWith(projectFileExtension)) return trimmed;
  if (extname(trimmed).toLowerCase() === ".json") return trimmed.replace(/\.json$/i, projectFileExtension);
  return `${trimmed}${projectFileExtension}`;
}

export function validateProjectOpenPath(path: string): ProjectPathResult {
  const trimmed = path.trim();
  if (!trimmed) return { ok: false, error: "Project path is empty" };
  if (!trimmed.toLowerCase().endsWith(projectFileExtension)) {
    return { ok: false, error: `Project file must use ${projectFileExtension}` };
  }
  return { ok: true, path: trimmed };
}

export function validateProjectJson(content: string): ProjectPathResult {
  try {
    const parsed = JSON.parse(content) as { schemaVersion?: unknown; projectId?: unknown };
    if (typeof parsed.schemaVersion !== "number" || typeof parsed.projectId !== "string" || parsed.projectId.trim() === "") {
      return { ok: false, error: "Invalid Local Audio Mixer project document" };
    }
    return { ok: true };
  } catch {
    return { ok: false, error: "Project file is not valid JSON" };
  }
}

export async function readProjectFile(path: string): Promise<ProjectPathResult & { content?: string }> {
  const validation = validateProjectOpenPath(path);
  if (!validation.ok || !validation.path) return validation;
  try {
    const content = await readFile(validation.path, "utf8");
    const documentValidation = validateProjectJson(content);
    if (!documentValidation.ok) return documentValidation;
    return { ok: true, path: validation.path, content };
  } catch (error) {
    return { ok: false, error: error instanceof Error ? error.message : String(error) };
  }
}

export async function writeProjectFile(path: string, content: string): Promise<ProjectPathResult> {
  const normalized = normalizeProjectSavePath(path);
  const validation = validateProjectJson(content);
  if (!validation.ok) return validation;
  try {
    await mkdir(dirname(normalized), { recursive: true });
    const tempPath = `${normalized}.tmp`;
    await writeFile(tempPath, content, "utf8");
    await rename(tempPath, normalized);
    return { ok: true, path: normalized };
  } catch (error) {
    return { ok: false, error: error instanceof Error ? error.message : String(error) };
  }
}

export function inspectProjectMedia(content: string): ProjectPathResult & { media?: ProjectMediaStatus[] } {
  const parsed = parseProjectDocument(content);
  if (!parsed.ok || !parsed.document) return parsed;
  return {
    ok: true,
    media: mediaEntries(parsed.document).map((item) => {
      const exists = existsSync(item.path);
      return { id: item.id, path: item.path, exists, missing: item.missing || !exists };
    })
  };
}

export function relinkProjectMedia(content: string, mediaId: string, newPath: string): ProjectPathResult & { content?: string } {
  const parsed = parseProjectDocument(content);
  const id = mediaId.trim();
  const path = newPath.trim();
  if (!parsed.ok || !parsed.document) return parsed;
  if (!id) return { ok: false, error: "Media id is empty" };
  if (!path || !existsSync(path)) return { ok: false, error: "Replacement media file does not exist" };

  const document = parsed.document;
  const media = mediaEntries(document);
  const index = media.findIndex((item) => item.id === id);
  if (index < 0) return { ok: false, error: "Media reference was not found" };
  media[index] = { ...media[index], path, missing: false };
  document.media = media;
  syncChannelSourceForMedia(document, id, path);
  return { ok: true, content: `${JSON.stringify(document, null, 2)}\n` };
}

export async function collectProjectMedia(content: string, projectPath: string): Promise<ProjectMediaCollectResult> {
  const openValidation = validateProjectOpenPath(projectPath);
  if (!openValidation.ok || !openValidation.path) return openValidation;
  const parsed = parseProjectDocument(content);
  if (!parsed.ok || !parsed.document) return parsed;

  const document = parsed.document;
  const media = mediaEntries(document);
  const mediaDir = join(dirname(openValidation.path), `${parse(openValidation.path).name} Media`);
  const collected: Array<{ id: string; from: string; to: string }> = [];
  const missing: ProjectMediaStatus[] = [];
  await mkdir(mediaDir, { recursive: true });

  const usedNames = new Set<string>();
  const nextMedia = await Promise.all(media.map(async (item) => {
    if (!existsSync(item.path)) {
      const status = { id: item.id, path: item.path, exists: false, missing: true };
      missing.push(status);
      return { ...item, missing: true };
    }
    const target = uniqueCollectPath(mediaDir, basename(item.path), usedNames);
    await copyFile(item.path, target);
    collected.push({ id: item.id, from: item.path, to: target });
    syncChannelSourceForMedia(document, item.id, target);
    return { ...item, path: target, missing: false };
  }));

  document.media = nextMedia;
  return { ok: true, content: `${JSON.stringify(document, null, 2)}\n`, collected, missing };
}

function parseProjectDocument(content: string): ProjectPathResult & { document?: Record<string, unknown> } {
  const validation = validateProjectJson(content);
  if (!validation.ok) return validation;
  try {
    const document = JSON.parse(content) as Record<string, unknown>;
    return { ok: true, document };
  } catch {
    return { ok: false, error: "Project file is not valid JSON" };
  }
}

function mediaEntries(document: Record<string, unknown>) {
  const media = Array.isArray(document.media) ? document.media : [];
  return media.flatMap((item) => {
    if (!item || typeof item !== "object" || Array.isArray(item)) return [];
    const value = item as { id?: unknown; path?: unknown; missing?: unknown };
    if (typeof value.id !== "string" || typeof value.path !== "string") return [];
    return [{ id: value.id, path: value.path, missing: Boolean(value.missing) }];
  });
}

function syncChannelSourceForMedia(document: Record<string, unknown>, mediaId: string, path: string) {
  if (!Array.isArray(document.channels)) return;
  document.channels = document.channels.map((channel) => {
    if (!channel || typeof channel !== "object" || Array.isArray(channel)) return channel;
    const item = channel as { id?: unknown; sourceUid?: unknown };
    if (item.id !== mediaId || typeof item.sourceUid !== "string") return channel;
    return { ...item, sourceUid: path };
  });
}

function uniqueCollectPath(directory: string, fileName: string, usedNames: Set<string>) {
  const parsed = parse(fileName);
  let candidate = fileName;
  let counter = 1;
  while (usedNames.has(candidate) || existsSync(join(directory, candidate))) {
    candidate = `${parsed.name}-${counter}${parsed.ext}`;
    counter += 1;
  }
  usedNames.add(candidate);
  return join(directory, candidate);
}
