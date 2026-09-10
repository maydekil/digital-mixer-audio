interface HorizontalSliderProps {
  label: string;
  value: number;
  min?: number;
  max?: number;
  unit?: string;
}

export function HorizontalSlider({ label, value, min = -60, max = 10, unit = "dB" }: HorizontalSliderProps) {
  return (
    <label className="horizontal-slider">
      <span>{label}</span>
      <input type="range" min={min} max={max} value={value} readOnly aria-label={label} />
      <strong>{value.toFixed(1)} {unit}</strong>
    </label>
  );
}
