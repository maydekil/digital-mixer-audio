import { extname } from "node:path";
import { mkdir, readFile, rename, writeFile } from "node:fs/promises";
import { dirname } from "node:path";

export const projectFileExtension = ".lam.json";

export interface ProjectPathResult {
  ok: boolean;
  canceled?: boolean;
  path?: string;
  error?: string;
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
