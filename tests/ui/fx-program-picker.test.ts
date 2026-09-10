import { describe, expect, it } from "vitest";
import { fxPrograms } from "../../apps/desktop/src/fixtures/fxPrograms";
import { formatProgram } from "../../apps/desktop/src/features/fx/components/ProgramPicker";

describe("FX program picker", () => {
  it("formats the full 99-program catalog for the combo box", () => {
    expect(fxPrograms).toHaveLength(99);
    expect(formatProgram(fxPrograms[11])).toBe("12 · Vocal Plate");
    expect(formatProgram(fxPrograms[49])).toBe("50 · Stereo 320");
    expect(formatProgram(fxPrograms[98])).toBe("99 · Infinite Mood");
  });
});
