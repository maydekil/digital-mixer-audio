interface RotaryKnobProps {
  label: string;
  value: string;
  tone?: "cyan" | "amber" | "violet";
  size?: "sm" | "md";
}

export function RotaryKnob({ label, value, tone = "cyan", size = "md" }: RotaryKnobProps) {
  return (
    <div className={`rotary ${size} tone-${tone}`} role="slider" aria-label={label} aria-valuetext={value} tabIndex={0}>
      <div className="rotary-label">{label}</div>
      <div className="rotary-face">
        <span className="rotary-tick" />
      </div>
      <div className="rotary-value">{value}</div>
    </div>
  );
}
