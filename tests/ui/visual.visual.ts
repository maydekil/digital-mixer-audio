import { expect, test } from "@playwright/test";

const viewports = [
  { name: "desktop-1680x945", width: 1680, height: 945 },
  { name: "minimum-1280x800", width: 1280, height: 800 }
];

for (const viewport of viewports) {
  test(`mixer layout ${viewport.name}`, async ({ page }) => {
    await page.setViewportSize({ width: viewport.width, height: viewport.height });
    await page.goto("/");
    await expect(page.getByText("UI PREVIEW · Audio engine not connected")).toBeVisible();
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

  await voiceStrip.getByLabel("VOICE fader").fill("-15");
  await expect(voiceStrip.getByText("-15.0 dB")).toBeVisible();

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

  await page.getByLabel("FX program").first().fill("Stereo");
  await page.keyboard.press("Enter");
  await expect(page.getByLabel("FX program").first()).toHaveValue(/Stereo/);

  await page.locator(".compact-fx-row").first().getByRole("button", { name: "Edit" }).click();
  await expect(page.getByRole("dialog", { name: "FX A editor" })).toBeVisible();
});
