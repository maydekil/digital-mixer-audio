interface VerticalFaderProps {
  valueDb: number;
  onChange?(value: number): void;
  label: string;
}

export function VerticalFader({ valueDb, onChange, label }: VerticalFaderProps) {
  const displayDb = clampFaderDb(valueDb);
  const position = dbToPosition(displayDb);
  const ticks = ["+10", "+5", "0", "-5", "-10"];

  return (
    <div className="vertical-fader">
      <div className="fader-scale" aria-hidden="true">
        {ticks.map((tick) => <span key={tick} style={{ bottom: `${dbToPosition(Number(tick))}%` }}>{tick}</span>)}
      </div>
      <label className="fader-track" aria-label={label}>
        <input
          type="range"
          min="0"
          max="100"
          step="1"
          value={position}
          onChange={(event) => onChange?.(positionToDb(Number(event.target.value)))}
        />
        <span className="fader-rail" />
        <span className="fader-cap" style={{ bottom: `${position}%` }} />
      </label>
      <div className="fader-readout">{displayDb.toFixed(1)} dB</div>
    </div>
  );
}

function dbToPosition(valueDb: number) {
  return ((clampFaderDb(valueDb) + 10) / 20) * 100;
}

function positionToDb(position: number) {
  const normalized = Math.max(0, Math.min(100, position));
  return (normalized / 100) * 20 - 10;
}

function clampFaderDb(valueDb: number) {
  return Math.max(-10, Math.min(10, valueDb));
}
