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
  onMacro(macro: "macro1" | "macro2", value: string): void;
  onReset(): void;
}

export function CompactFxRow({ unit, program, programs, onProgramChange, onToggle, onReturn, onMacro, onReset }: CompactFxRowProps) {
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
      <EditableMacro label={program.macro1.label} value={program.macro1.value} tone={unit.accent} onCommit={(value) => onMacro("macro1", value)} />
      <EditableMacro label={program.macro2.label} value={program.macro2.value} tone={unit.accent} onCommit={(value) => onMacro("macro2", value)} />
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
            <label>
              {program.macro1.label}
              <input value={program.macro1.value} onChange={(event) => onMacro("macro1", event.target.value)} />
            </label>
            <label>
              {program.macro2.label}
              <input value={program.macro2.value} onChange={(event) => onMacro("macro2", event.target.value)} />
            </label>
          </div>
        ) : null}
      </div>
    </section>
  );
}

function EditableMacro({ label, value, tone, onCommit }: { label: string; value: string; tone: "amber" | "cyan"; onCommit(value: string): void }) {
  const [editing, setEditing] = useState(false);
  const [draft, setDraft] = useState(value);

  function commit(nextValue = draft) {
    setEditing(false);
    onCommit(nextValue);
  }

  if (editing) {
    return (
      <label className="fx-macro-edit">
        <span>{label}</span>
        <input
          value={draft}
          autoFocus
          onChange={(event) => setDraft(event.target.value)}
          onBlur={(event) => commit(event.target.value)}
          onKeyDown={(event) => {
            if (event.key === "Enter") event.currentTarget.blur();
            if (event.key === "Escape") {
              setDraft(value);
              setEditing(false);
            }
          }}
        />
      </label>
    );
  }

  return (
    <button className="fx-macro-button" type="button" onClick={() => {
      setDraft(value);
      setEditing(true);
    }}>
      <RotaryKnob label={label} value={value} tone={tone} size="sm" />
    </button>
  );
}
