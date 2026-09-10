import { useEffect, useId, useState } from "react";
import type { FxProgram } from "../../../adapters/MixerControlPort";

interface ProgramPickerProps {
  programs: FxProgram[];
  selectedId: number;
  onChange(programId: number): void;
}

export function ProgramPicker({ programs, selectedId, onChange }: ProgramPickerProps) {
  const listId = useId();
  const selected = programs.find((program) => program.id === selectedId) ?? programs[0];
  const [text, setText] = useState(formatProgram(selected));

  useEffect(() => {
    setText(formatProgram(selected));
  }, [selected]);

  function commit(value: string) {
    const normalized = value.trim().toLowerCase();
    const numeric = Number.parseInt(normalized, 10);
    const match = programs.find((program) => program.id === numeric) ??
      programs.find((program) => formatProgram(program).toLowerCase() === normalized) ??
      programs.find((program) => program.name.toLowerCase().includes(normalized));

    if (match) onChange(match.id);
    setText(formatProgram(match ?? selected));
  }

  return (
    <>
      <input
        className="program-picker"
        list={listId}
        value={text}
        onChange={(event) => setText(event.target.value)}
        onBlur={(event) => commit(event.target.value)}
        onKeyDown={(event) => {
          if (event.key === "Enter") {
            event.currentTarget.blur();
            commit(event.currentTarget.value);
          }
        }}
        aria-label="FX program"
      />
      <datalist id={listId}>
      {programs.map((program) => (
        <option key={program.id} value={formatProgram(program)} />
      ))}
      </datalist>
    </>
  );
}

function formatProgram(program: FxProgram) {
  return `${program.id.toString().padStart(2, "0")} · ${program.name}`;
}
