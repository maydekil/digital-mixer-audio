export function ClipIndicator({ active, onReset }: { active: boolean; onReset?(): void }) {
  return (
    <button className={`clip-indicator ${active ? "is-active" : ""}`} onClick={onReset} aria-label="Reset clip indicator">
      CLIP
    </button>
  );
}
