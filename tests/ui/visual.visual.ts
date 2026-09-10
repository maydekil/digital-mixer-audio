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
