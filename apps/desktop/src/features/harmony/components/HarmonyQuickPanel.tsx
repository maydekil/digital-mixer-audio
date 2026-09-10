import type { HarmonyState } from "../../../adapters/MixerControlPort";
import { Button } from "../../../components/ui/Button";
import { SelectField } from "../../../components/ui/SelectField";

interface HarmonyQuickPanelProps {
  harmony: HarmonyState;
  onToggle(enabled: boolean): void;
  onChange(field: keyof HarmonyState, value: string | number | boolean): void;
}

export function HarmonyQuickPanel({ harmony, onToggle, onChange }: HarmonyQuickPanelProps) {
  const keyOptions = ["C", "D", "E", "F", "G", "A", "B"].map((key) => ({ value: key, label: key }));
  const scaleOptions = ["Major", "Natural Minor", "Chromatic"].map((scale) => ({ value: scale, label: scale }));
  const intervalOptions = ["+3rd", "+5th", "-3rd", "-5th", "+Oct"].map((interval) => ({ value: interval, label: interval }));

  return (
    <section className="harmony-panel">
      <div className="harmony-row harmony-main-row">
        <div className="harmony-title"><span>⏻</span><strong>VOICE · HARMONY</strong></div>
        <Button tone="violet" active={harmony.enabled} onClick={() => onToggle(!harmony.enabled)}>{harmony.enabled ? "ON" : "OFF"}</Button>
        <SelectField label="Key" value={harmony.key} options={keyOptions} onChange={(value) => onChange("key", value)} />
        <SelectField label="Scale" value={harmony.scale} options={scaleOptions} onChange={(value) => onChange("scale", value)} />
        <SelectField label="Voice 1" value={harmony.voice1} options={intervalOptions} onChange={(value) => onChange("voice1", value)} />
        <SelectField label="Voice 2" value={harmony.voice2} options={intervalOptions} onChange={(value) => onChange("voice2", value)} />
      </div>
      <div className="harmony-row harmony-secondary-row">
        <label className="harmony-level">
          <span>Harmony Level</span>
          <input type="range" min="-30" max="6" value={harmony.levelDb} onChange={(event) => onChange("levelDb", Number(event.target.value))} />
          <strong>{harmony.levelDb.toFixed(1)} dB</strong>
        </label>
        <Button>⚙ Advanced</Button>
        <p>Set key and scale to match your song.</p>
      </div>
    </section>
  );
}
