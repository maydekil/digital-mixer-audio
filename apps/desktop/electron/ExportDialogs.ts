import { extname } from "node:path";

export const exportFileExtension = ".wav";

export interface ExportPathResult {
  ok: boolean;
  canceled?: boolean;
  path?: string;
  error?: string;
}

export function normalizeExportOutputPath(path: string): string {
  const trimmed = path.trim();
  if (trimmed.toLowerCase().endsWith(exportFileExtension)) return trimmed;
  if (extname(trimmed).toLowerCase() === ".wave") return trimmed.replace(/\.wave$/i, exportFileExtension);
  return `${trimmed}${exportFileExtension}`;
}

export function validateExportOutputPath(path: string): ExportPathResult {
  const normalized = normalizeExportOutputPath(path);
  if (!normalized || normalized === exportFileExtension) return { ok: false, error: "Export path is empty" };
  if (!normalized.toLowerCase().endsWith(exportFileExtension)) {
    return { ok: false, error: `Export output must use ${exportFileExtension}` };
  }
  return { ok: true, path: normalized };
}
