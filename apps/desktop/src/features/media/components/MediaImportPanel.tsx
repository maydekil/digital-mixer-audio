import { useState } from "react";
import { Button } from "../../../components/ui/Button";

interface MediaStatus {
  ok?: boolean;
  imported?: boolean;
  error?: string;
  path?: string;
  channels?: number;
  sampleRate?: number;
  frameCount?: number;
  totalFrames?: number;
  durationSeconds?: number;
  jobId?: string;
  state?: string;
  progress?: number;
  waveformPoints?: number;
}

interface MediaImportPanelProps {
  open: boolean;
  onClose(): void;
}

export function MediaImportPanel({ open, onClose }: MediaImportPanelProps) {
  const [path, setPath] = useState("");
  const [status, setStatus] = useState("No media selected");
  const [metadata, setMetadata] = useState<MediaStatus | null>(null);
  const [jobId, setJobId] = useState("");
  const [busy, setBusy] = useState(false);

  if (!open) return null;

  async function chooseFile() {
    if (!window.localMixer?.chooseMediaFile) {
      setStatus("Open desktop engine mode");
      return;
    }
    const result = await window.localMixer.chooseMediaFile();
    if (result.canceled) return;
    if (!result.ok || !result.path) {
      setStatus(result.error ?? "File selection failed");
      return;
    }
    setPath(result.path);
    setStatus("Media selected");
    setMetadata(null);
    setJobId("");
  }

  async function send(type: string, payload: Record<string, unknown> = {}) {
    if (!window.localMixer?.engineCommand) {
      setStatus("Native engine is not running");
      return null;
    }
    const result = await window.localMixer.engineCommand(type, payload) as MediaStatus;
    if (result.ok === false) setStatus(result.error ?? "Media command failed");
    return result;
  }

  async function inspect() {
    if (!path) return;
    setBusy(true);
    const result = await send("media-inspect", { path });
    setBusy(false);
    if (!result) return;
    setMetadata(result);
    setStatus(result.imported ? "Metadata ready" : result.error ?? "Unsupported media");
  }

  async function startImport() {
    if (!path) return;
    setBusy(true);
    const result = await send("media-import-start", { path, framesPerPoint: 512 });
    setBusy(false);
    if (!result) return;
    setJobId(String(result.jobId ?? ""));
    setMetadata(result);
    setStatus(result.state ?? result.error ?? "Import queued");
  }

  async function pollImport() {
    if (!jobId) return;
    const result = await send("media-import-status", { jobId });
    if (!result) return;
    setMetadata(result);
    setStatus(result.state ?? result.error ?? "Import status");
  }

  async function cancelImport() {
    if (!jobId) return;
    const result = await send("media-import-cancel", { jobId });
    if (!result) return;
    setMetadata(result);
    setStatus(result.state ?? result.error ?? "Import canceled");
  }

  return (
    <div className="modal-backdrop" role="presentation" onMouseDown={onClose}>
      <div className="media-import-modal" role="dialog" aria-modal="true" aria-label="Media import" onMouseDown={(event) => event.stopPropagation()}>
        <header>
          <div>
            <strong>MEDIA IMPORT</strong>
            <span>{status}</span>
          </div>
          <Button tone="danger" onClick={onClose}>Close</Button>
        </header>
        <div className="media-import-path">
          <span>File</span>
          <strong>{path || "No file selected"}</strong>
          <Button onClick={() => void chooseFile()} disabled={busy}>Choose</Button>
        </div>
        <div className="media-import-actions">
          <Button tone="cyan" onClick={() => void inspect()} disabled={busy || !path}>Inspect</Button>
          <Button tone="green" onClick={() => void startImport()} disabled={busy || !path}>Import</Button>
          <Button onClick={() => void pollImport()} disabled={!jobId}>Poll</Button>
          <Button tone="danger" onClick={() => void cancelImport()} disabled={!jobId}>Cancel</Button>
        </div>
        <div className="media-import-status">
          <span>Rate</span><strong>{metadata?.sampleRate ? `${metadata.sampleRate} Hz` : "-"}</strong>
          <span>Channels</span><strong>{metadata?.channels ?? "-"}</strong>
          <span>Frames</span><strong>{metadata?.frameCount ?? metadata?.totalFrames ?? "-"}</strong>
          <span>Progress</span><strong>{metadata?.progress !== undefined ? `${Math.round(metadata.progress * 100)}%` : "-"}</strong>
          <span>Waveform</span><strong>{metadata?.waveformPoints ?? "-"}</strong>
        </div>
      </div>
    </div>
  );
}
