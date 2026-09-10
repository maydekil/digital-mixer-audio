interface VerticalFaderProps {
  valueDb: number;
  onChange?(value: number): void;
  label: string;
}

export function VerticalFader({ valueDb, onChange, label }: VerticalFaderProps) {
  const normalized = Math.max(0, Math.min(100, ((valueDb + 60) / 70) * 100));

  return (
    <div className="vertical-fader">
      <div className="fader-scale" aria-hidden="true">
        {["+6", "0", "-6", "-12", "-24", "-36", "-60"].map((tick) => <span key={tick}>{tick}</span>)}
      </div>
      <label className="fader-track" aria-label={label}>
        <input
          type="range"
          min="-60"
          max="10"
          step="1"
          value={valueDb}
          onChange={(event) => onChange?.(Number(event.target.value))}
        />
        <span className="fader-rail" />
        <span className="fader-cap" style={{ bottom: `${normalized}%` }} />
      </label>
      <div className="fader-readout">{valueDb.toFixed(1)} dB</div>
    </div>
  );
}
