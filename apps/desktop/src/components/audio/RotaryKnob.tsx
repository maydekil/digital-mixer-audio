interface RotaryKnobProps {
  label: string;
  value: string;
  tone?: "cyan" | "amber" | "violet";
  size?: "sm" | "md";
  numericValue?: number;
  min?: number;
  max?: number;
  step?: number;
  onChange?(value: number): void;
}

export function RotaryKnob({ label, value, tone = "cyan", size = "md", numericValue, min = 0, max = 100, step = 1, onChange }: RotaryKnobProps) {
  const current = numericValue ?? min;
  const normalized = Math.max(0, Math.min(1, (current - min) / (max - min)));
  const rotation = -135 + normalized * 270;

  return (
    <label className={`rotary ${size} tone-${tone}`}>
      <div className="rotary-label">{label}</div>
      <div className="rotary-face">
        <span className="rotary-tick" style={{ transform: `translateX(-50%) rotate(${rotation}deg)` }} />
        {onChange ? (
          <input
            type="range"
            min={min}
            max={max}
            step={step}
            value={current}
            onChange={(event) => onChange(Number(event.target.value))}
            aria-label={label}
            aria-valuetext={value}
          />
        ) : null}
      </div>
      <div className="rotary-value">{value}</div>
    </label>
  );
}
