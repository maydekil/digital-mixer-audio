import { existsSync } from "node:fs";
import { join } from "node:path";
import { spawn, spawnSync } from "node:child_process";
import { createRequire } from "node:module";
import { setTimeout as delay } from "node:timers/promises";

const url = "http://127.0.0.1:5173";
const enginePath = join(process.cwd(), "native", "engine", "build", "native", "engine", "local-mixer-engine");
const require = createRequire(import.meta.url);
const electronPath = require("electron");

runRequiredBuild("build:native");
runRequiredBuild("build:desktop:main");

if (!existsSync(enginePath)) {
  throw new Error(`Native engine is required but missing: ${enginePath}`);
}

const vite = spawn("npm", ["run", "dev:ui"], { stdio: "inherit", shell: false });

process.on("exit", () => vite.kill());
process.on("SIGINT", () => {
  vite.kill();
  process.exit(130);
});

await waitForServer(url);

const electron = spawn(electronPath, ["dist/electron/main.js"], {
  stdio: "inherit",
  shell: false,
  env: electronEnv()
});

electron.on("exit", (code) => {
  vite.kill();
  process.exit(code ?? 0);
});

async function waitForServer(target) {
  for (let attempt = 0; attempt < 80; attempt += 1) {
    try {
      const response = await fetch(target);
      if (response.ok) return;
    } catch {
      await delay(150);
    }
  }
  vite.kill();
  throw new Error(`Timed out waiting for ${target}`);
}

function electronEnv() {
  const env = {
    ...process.env,
    LOCAL_MIXER_DEV_SERVER: "1",
    LOCAL_MIXER_DEV_URL: url,
    LOCAL_MIXER_REQUIRE_ENGINE: "1",
    LOCAL_MIXER_ENGINE_PATH: enginePath
  };
  delete env.ELECTRON_RUN_AS_NODE;
  return env;
}

function runRequiredBuild(script) {
  const result = spawnSync("npm", ["run", script], { stdio: "inherit", shell: false });
  if (result.status !== 0) process.exit(result.status ?? 1);
}
