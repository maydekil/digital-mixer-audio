import type { VocalFxState } from "../../../adapters/MixerControlPort";
import { Button } from "../../../components/ui/Button";

interface VocalFxPanelProps {
  open: boolean;
  vocalFx: VocalFxState;
  onClose(): void;
  onSelect(slotId: string): void;
  onToggle(slotId: string, enabled: boolean): void;
  onPreset(presetId: string): void;
}

const library = [
  ["Pitch", "Pitch Correction", "Pitch Shift", "Formant Shift", "Harmony"],
  ["Space", "Reverb", "Delay"],
  ["Modulation", "Chorus", "Doubler", "Flanger", "Phaser"],
  ["Character", "Saturation"],
  ["Synth", "Vocoder / Robot"]
] as const;

export function VocalFxPanel({ open, vocalFx, onClose, onSelect, onToggle, onPreset }: VocalFxPanelProps) {
  if (!open) return null;
  const selected = vocalFx.slots.find((slot) => slot.id === vocalFx.selectedSlotId) ?? vocalFx.slots[0];
  const latencyMs = vocalFx.slots.filter((slot) => slot.enabled).reduce((sum, slot) => sum + slot.latencyMs, 0);

  return (
    <div className="modal-backdrop">
      <section className="vocal-fx-modal" role="dialog" aria-label="Vocal FX rack">
        <header>
          <div><strong>Vocal FX Rack</strong><span>Native insert chain</span></div>
          <select value={vocalFx.activePresetId} onChange={(event) => onPreset(event.target.value)} aria-label="Vocal FX preset">
            {vocalFx.presets.map((preset) => <option key={preset.id} value={preset.id}>{preset.name}</option>)}
          </select>
          <Button onClick={onClose}>Close</Button>
        </header>
        <aside className="vocal-fx-library" aria-label="Effect Library">
          {library.map(([category, ...items]) => (
            <div key={category}>
              <strong>{category}</strong>
              {items.map((item) => <button key={item}>{item}<span>+</span></button>)}
            </div>
          ))}
        </aside>
        <div className="vocal-fx-rack" aria-label="Rack slots">
          {vocalFx.slots.map((slot, index) => (
            <button
              key={slot.id}
              className={`vocal-fx-slot ${slot.id === selected.id ? "is-selected" : ""} ${slot.enabled ? "is-enabled" : ""}`}
              onClick={() => onSelect(slot.id)}
            >
              <span>{String(index + 1).padStart(2, "0")}</span>
              <strong>{slot.label}</strong>
              <em>{slot.category}</em>
            </button>
          ))}
        </div>
        <section className="vocal-fx-editor" aria-label="Effect editor">
          <div className="vocal-fx-editor-title">
            <div>
              <strong>{selected.label}</strong>
              <span>{selected.effectType} · {selected.availability}</span>
            </div>
            <Button active={selected.enabled} onClick={() => onToggle(selected.id, !selected.enabled)}>
              {selected.enabled ? "ON" : "BYPASS"}
            </Button>
          </div>
          <div className="vocal-fx-params">
            {selected.parameters.map((parameter) => (
              <label key={parameter.id}>
                <span>{parameter.label}</span>
                <b>{parameter.value}</b>
              </label>
            ))}
          </div>
          <footer>
            <span>Latency</span><strong>{latencyMs} ms</strong>
            <span>Profile</span><strong>Studio FX</strong>
          </footer>
        </section>
      </section>
    </div>
  );
}
