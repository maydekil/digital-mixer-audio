interface SelectFieldProps {
  label?: string;
  value: string | number;
  options: Array<{ value: string | number; label: string }>;
  onChange(value: string): void;
  className?: string;
}

export function SelectField({ label, value, options, onChange, className = "" }: SelectFieldProps) {
  return (
    <label className={`select-field ${className}`}>
      {label ? <span>{label}</span> : null}
      <select value={value} onChange={(event) => onChange(event.target.value)}>
        {options.map((option) => <option key={option.value} value={option.value}>{option.label}</option>)}
      </select>
    </label>
  );
}
