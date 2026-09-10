import { useState } from "react";
import type { FxProgram, FxUnitState } from "../../../adapters/MixerControlPort";
import { LevelMeter } from "../../../components/audio/LevelMeter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { Button } from "../../../components/ui/Button";
import { Badge } from "../../../components/ui/Badge";
import { ProgramPicker } from "./ProgramPicker";

interface CompactFxRowProps {
  unit: FxUnitState;
  program: FxProgram;
  programs: FxProgram[];
  onProgramChange(programId: number): void;
  onToggle(enabled: boolean): void;
  onReturn(valueDb: number): void;
  onReset(): void;
}

export function CompactFxRow({ unit, program, programs, onProgramChange, onToggle, onReturn, onReset }: CompactFxRowProps) {
  const [editing, setEditing] = useState(false);

  return (
    <section className={`compact-fx-row accent-${unit.accent}`}>
      <div className="fx-power">
        <span className="power-icon">⏻</span>
        <strong>{unit.label}</strong>
      </div>
      <Button tone={unit.accent} active={unit.enabled} onClick={() => onToggle(!unit.enabled)}>
        {unit.enabled ? "ON" : "OFF"}
      </Button>
      <ProgramPicker programs={programs} selectedId={unit.programId} onChange={onProgramChange} />
      {unit.modified ? <button className="modified-badge" onClick={onReset}>Modified</button> : <span className="preset-count">99 presets</span>}
      <RotaryKnob label={program.macro1.label} value={program.macro1.value} tone={unit.accent} size="sm" />
      <RotaryKnob label={program.macro2.label} value={program.macro2.value} tone={unit.accent} size="sm" />
      <div className="fx-return">
        <span>Return</span>
        <input type="range" min="-60" max="10" value={unit.returnDb} onChange={(event) => onReturn(Number(event.target.value))} aria-label={`${unit.label} return`} />
        <strong>{unit.returnDb.toFixed(1)} dB</strong>
      </div>
      <LevelMeter level={unit.meter} showChannelLabels />
      <div className="fx-edit-cell">
        <Button active={editing} onClick={() => setEditing(!editing)}>Edit</Button>
        {editing ? (
          <div className="fx-edit-popover" role="dialog" aria-label={`${unit.label} editor`}>
            <strong>{program.name}</strong>
            <label>
              Return
              <input type="range" min="-60" max="10" value={unit.returnDb} onChange={(event) => onReturn(Number(event.target.value))} />
              <span>{unit.returnDb.toFixed(1)} dB</span>
            </label>
            <p>{program.macro1.label}: {program.macro1.value}</p>
            <p>{program.macro2.label}: {program.macro2.value}</p>
          </div>
        ) : null}
      </div>
    </section>
  );
}
