import { expect, test } from "@playwright/test";

const viewports = [
  { name: "desktop-1680x945", width: 1680, height: 945 },
  { name: "minimum-1280x800", width: 1280, height: 800 }
];

for (const viewport of viewports) {
  test(`mixer layout ${viewport.name}`, async ({ page }) => {
    await page.setViewportSize({ width: viewport.width, height: viewport.height });
    await page.goto("/");
    await expect(page.getByRole("heading", { name: "Local Audio Mixer" })).toBeVisible();
    await expect(page.getByText("FX A")).toBeVisible();
    await expect(page.getByText("FX B")).toBeVisible();
    await expect(page.getByText("VOICE · HARMONY")).toBeVisible();
    await expect(page.getByText("PARAMETRIC EQ")).toBeVisible();
    await expect(page.getByText("SEND A · Vocal Plate")).toBeVisible();
    await expect(page.getByText("SOUND PADS")).toBeVisible();
    await expect(page.getByRole("button", { name: /APPLAUSE/i })).toBeEnabled();
    await page.screenshot({ path: `docs/reports/ui/${viewport.name}.png`, fullPage: true });
  });
}

test("preview controls mutate visible mixer state", async ({ page }) => {
  await page.setViewportSize({ width: 1680, height: 945 });
  await page.goto("/");

  const voiceStrip = page.locator(".channel-strip.is-selected");
  await voiceStrip.locator('input[aria-label="Gain"]').fill("5");
  await expect(voiceStrip.getByText("5.0 dB")).toBeVisible();

  await voiceStrip.locator('input[aria-label="Pan"]').fill("-40");
  await expect(voiceStrip.getByText("L 40")).toBeVisible();

  await voiceStrip.locator('label[aria-label="VOICE fader"] input').fill("25");
  await expect(voiceStrip.getByText("-5.0 dB")).toBeVisible();

  await voiceStrip.locator('input[aria-label="SEND A"]').fill("-9");
  await expect(voiceStrip.getByText("-9 dB")).toBeVisible();
  await expect(page.locator(".processing-panel").getByText("-9 dB")).toBeVisible();

  await page.getByLabel("FX A return").fill("-18");
  await expect(page.locator(".compact-fx-row").first().getByText("-18.0 dB")).toBeVisible();

  await voiceStrip.getByRole("button", { name: "MON", exact: true }).click();
  await expect(page.getByText("MONITOR ON")).toHaveCount(0);
  await voiceStrip.getByRole("button", { name: "REC", exact: true }).click();
  await expect(voiceStrip.getByRole("button", { name: "REC", exact: true })).not.toHaveClass(/is-active/);
  await expect(voiceStrip.getByRole("button", { name: "EQ", exact: true })).toHaveClass(/is-active/);
  await voiceStrip.getByRole("button", { name: "NOISE", exact: true }).click();
  await expect(voiceStrip.getByRole("button", { name: "NOISE", exact: true })).not.toHaveClass(/is-active/);

  await page.getByLabel("FX program").first().selectOption("50");
  await expect(page.getByLabel("FX program").first()).toHaveValue("50");

  await page.locator(".compact-fx-row").first().getByRole("button", { name: "Edit" }).click();
  await expect(page.getByRole("dialog", { name: "FX A editor" })).toBeVisible();
});

test("hardware modal exposes guarded system routing controls", async ({ page }) => {
  await page.setViewportSize({ width: 1680, height: 945 });
  await page.goto("/");

  await page.getByRole("button", { name: "HW" }).click();
  await expect(page.getByRole("dialog", { name: "Hardware monitor" })).toBeVisible();
  await expect(page.getByText("Loopback")).toBeVisible();
  await expect(page.getByRole("button", { name: "Route Check" })).toBeVisible();
  await expect(page.getByRole("button", { name: "Enable Route" })).toBeVisible();
  await expect(page.getByText("Recovery")).toBeVisible();
});

test("timeline tab opens native media import surface", async ({ page }) => {
  await page.setViewportSize({ width: 1680, height: 945 });
  await page.goto("/");

  await page.getByRole("button", { name: "Timeline" }).click();
  await expect(page.getByRole("dialog", { name: "Media import" })).toBeVisible();
  await expect(page.getByRole("button", { name: "Choose" })).toBeVisible();
  await expect(page.getByRole("button", { name: "Import" })).toBeDisabled();
});

test("vocal fx tab opens rack editor surface", async ({ page }) => {
  await page.setViewportSize({ width: 1680, height: 945 });
  await page.goto("/");

  await page.getByRole("button", { name: "Vocal FX" }).click();
  await expect(page.getByRole("dialog", { name: "Vocal FX rack" })).toBeVisible();
  await expect(page.getByLabel("Voice preset selector")).toContainText("03 · Studio Pop");
  await page.getByLabel("Vocal FX preset").selectOption("voice-80-robot-modern");
  await expect(page.getByLabel("Voice preset selector")).toContainText("80 · Robot Modern");
  await page.getByRole("button", { name: "Advanced", exact: true }).click();
  await expect(page.getByLabel("Effect Library")).toContainText("Pitch Correction");
  await expect(page.getByLabel("Rack slots")).toContainText("Robot Voice");
  await expect(page.getByLabel("Effect editor")).toContainText("Robot Voice");
});
