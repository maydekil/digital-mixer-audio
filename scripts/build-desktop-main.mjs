import { copyFileSync, mkdirSync } from "node:fs";
import { join } from "node:path";
import { spawnSync } from "node:child_process";

const tsc = spawnSync("tsc", ["-p", "apps/desktop/electron/tsconfig.json"], { stdio: "inherit", shell: false });
if (tsc.status !== 0) process.exit(tsc.status ?? 1);

mkdirSync(join("dist", "electron"), { recursive: true });
copyFileSync(join("apps", "desktop", "electron", "preload.cjs"), join("dist", "electron", "preload.cjs"));
