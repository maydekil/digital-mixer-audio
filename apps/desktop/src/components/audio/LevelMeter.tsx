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
        <span className="meter-bar" style={{ "--meter": `${left}%` } as CSSProperties} />
        {showChannelLabels ? <b>L</b> : null}
      </span>
      <span className="meter-channel">
        <span className="meter-bar" style={{ "--meter": `${right}%` } as CSSProperties} />
        {showChannelLabels ? <b>R</b> : null}
      </span>
    </div>
  );
}

function meterPercent(db: number) {
  return Math.max(2, Math.min(100, ((db + 60) / 60) * 100));
}
