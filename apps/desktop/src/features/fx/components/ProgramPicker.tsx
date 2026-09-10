import type { FxProgram } from "../../../adapters/MixerControlPort";

interface ProgramPickerProps {
  programs: FxProgram[];
  selectedId: number;
  onChange(programId: number): void;
}

export function ProgramPicker({ programs, selectedId, onChange }: ProgramPickerProps) {
  return (
    <select className="program-picker" value={selectedId} onChange={(event) => onChange(Number(event.target.value))} aria-label="FX program">
      {programs.map((program) => (
        <option key={program.id} value={program.id}>
          {program.id.toString().padStart(2, "0")} · {program.name}
        </option>
      ))}
    </select>
  );
}
