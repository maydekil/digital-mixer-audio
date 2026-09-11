interface VerticalFaderProps {
  valueDb: number;
  onChange?(value: number): void;
  label: string;
}

export function VerticalFader({ valueDb, onChange, label }: VerticalFaderProps) {
  const volume = dbToVolume(valueDb);
  const ticks = ["20", "15", "10", "5", "0"];

  return (
    <div className="vertical-fader">
      <div className="fader-scale" aria-hidden="true">
        {ticks.map((tick) => <span key={tick} style={{ bottom: `${volumeToPosition(Number(tick))}%` }}>{tick}</span>)}
      </div>
      <label className="fader-track" aria-label={label}>
        <input
          type="range"
          min="0"
          max="20"
          step="1"
          value={volume}
          onChange={(event) => onChange?.(volumeToDb(Number(event.target.value)))}
        />
        <span className="fader-rail" />
        <span className="fader-cap" style={{ bottom: `${volumeToPosition(volume)}%` }} />
      </label>
      <div className="fader-readout">{volume.toFixed(0)}</div>
    </div>
  );
}

function volumeToPosition(volume: number) {
  return (clampVolume(volume) / 20) * 100;
}

function dbToVolume(valueDb: number) {
  if (!Number.isFinite(valueDb) || valueDb <= -60) return 0;
  return Math.max(0, Math.min(20, Math.round(20 * 10 ** (valueDb / 20))));
}

function volumeToDb(volume: number) {
  const normalized = clampVolume(volume) / 20;
  if (normalized <= 0) return -60;
  return 20 * Math.log10(normalized);
}

function clampVolume(volume: number) {
  return Math.max(0, Math.min(20, volume));
}
