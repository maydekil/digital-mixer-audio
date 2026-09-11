import type { CSSProperties } from "react";
import type { MeterLevel } from "./types";

interface LevelMeterProps {
  level: MeterLevel;
  vertical?: boolean;
  showChannelLabels?: boolean;
}

export function LevelMeter({ level, vertical = false, showChannelLabels = false }: LevelMeterProps) {
  const left = meterPercent(level.left);
  const right = meterPercent(level.right ?? level.left);
  return (
    <div className={`level-meter ${vertical ? "vertical" : "horizontal"}${showChannelLabels ? " has-channel-labels" : ""}`} aria-label="level meter">
      <span className="meter-channel">
        <span className="meter-bar" style={meterStyle(left)} />
        {showChannelLabels ? <b>L</b> : null}
      </span>
      <span className="meter-channel">
        <span className="meter-bar" style={meterStyle(right)} />
        {showChannelLabels ? <b>R</b> : null}
      </span>
    </div>
  );
}

function meterStyle(value: number) {
  return {
    "--meter": `${value}%`,
    "--meter-ratio": `${value / 100}`
  } as CSSProperties;
}

function meterPercent(db: number) {
  return Math.max(2, Math.min(100, ((db + 60) / 60) * 100));
}
