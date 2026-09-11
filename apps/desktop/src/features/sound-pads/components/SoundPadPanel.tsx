import { useRef, useState } from "react";
import { Button } from "../../../components/ui/Button";

const SOUND_PADS = [
  { id: "applause", label: "Applause" },
  { id: "laugh", label: "Laugh" },
  { id: "cheer", label: "Cheer" },
  { id: "drumroll", label: "Drum Roll" },
  { id: "ding", label: "Ding" },
  { id: "whoosh", label: "Whoosh" },
];

export function SoundPadPanel() {
  const [activePadId, setActivePadId] = useState<string | null>(null);
  const [status, setStatus] = useState("Ready");
  const requestId = useRef(0);

  async function playPad(padId: string) {
    if (!window.localMixer?.playSoundPad) {
      setStatus("Native bridge unavailable");
      return;
    }

    const currentRequest = requestId.current + 1;
    requestId.current = currentRequest;
    setActivePadId(padId);
    setStatus(`Playing ${padId}`);
    const result = await window.localMixer?.playSoundPad?.(padId);
    if (requestId.current !== currentRequest) return;
    if (result?.ok) setStatus("Ready");
    if (result && !result.ok) setStatus(result.error ?? "Playback failed");
    setActivePadId(null);
  }

  async function stopPads() {
    requestId.current += 1;
    setActivePadId(null);
    if (!window.localMixer?.stopSoundPads) {
      setStatus("Native bridge unavailable");
      return;
    }

    const result = await window.localMixer.stopSoundPads();
    setStatus(result?.ok ? "Ready" : result?.error ?? "Stop failed");
  }

  return (
    <section className="sound-pad-panel" aria-label="Sound effect pads">
      <header>
        <div>
          <strong>SOUND PADS</strong>
          <span>{status}</span>
        </div>
        <Button tone="danger" onClick={() => void stopPads()}>STOP</Button>
      </header>
      <div className="sound-pad-grid">
        {SOUND_PADS.map((pad) => (
          <button className={`sound-pad${activePadId === pad.id ? " is-playing" : ""}`} key={pad.id} type="button" onClick={() => void playPad(pad.id)}>
            <strong>{pad.label}</strong>
          </button>
        ))}
      </div>
    </section>
  );
}
