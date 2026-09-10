import { extname } from "node:path";

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
