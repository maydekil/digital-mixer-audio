import type { CSSProperties } from "react";

interface NumericParameterProps {
  label: string;
  value: string;
  color?: string;
}

export function NumericParameter({ label, value, color }: NumericParameterProps) {
  return (
    <label className="numeric-parameter" style={{ "--param-color": color ?? "var(--cyan)" } as CSSProperties}>
      <span>{label}</span>
      <input value={value} readOnly aria-label={label} />
    </label>
  );
}
