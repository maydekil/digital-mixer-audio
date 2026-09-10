import type { FxProgram } from "../../../adapters/MixerControlPort";

interface ProgramPickerProps {
  programs: FxProgram[];
  selectedId: number;
  onChange(programId: number): void;
}

export function ProgramPicker({ programs, selectedId, onChange }: ProgramPickerProps) {
  const selected = programs.some((program) => program.id === selectedId) ? selectedId : programs[0]?.id ?? 0;

  return (
    <select
      className="program-picker"
      value={selected}
      onChange={(event) => onChange(Number(event.target.value))}
      aria-label="FX program"
    >
      {programs.map((program) => (
        <option key={program.id} value={program.id}>
          {formatProgram(program)}
        </option>
      ))}
    </select>
  );
}

export function formatProgram(program: FxProgram) {
  return `${program.id.toString().padStart(2, "0")} · ${program.name}`;
}
