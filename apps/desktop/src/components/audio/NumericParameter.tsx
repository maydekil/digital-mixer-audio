import type { CSSProperties } from "react";
import { useEffect, useState } from "react";

interface NumericParameterProps {
  label: string;
  value: string;
  color?: string;
  onCommit?(value: string): void;
}

export function NumericParameter({ label, value, color, onCommit }: NumericParameterProps) {
  const [draft, setDraft] = useState(value);

  useEffect(() => {
    setDraft(value);
  }, [value]);

  function commit() {
    onCommit?.(draft);
  }

  return (
    <label className="numeric-parameter" style={{ "--param-color": color ?? "var(--cyan)" } as CSSProperties}>
      <span>{label}</span>
      <input
        value={draft}
        readOnly={!onCommit}
        onChange={(event) => setDraft(event.target.value)}
        onBlur={commit}
        onKeyDown={(event) => {
          if (event.key === "Enter") event.currentTarget.blur();
        }}
        aria-label={label}
      />
    </label>
  );
}
