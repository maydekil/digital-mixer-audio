import { dirname, join } from "node:path";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import { existsSync } from "node:fs";
import { spawn } from "node:child_process";
import { EngineSupervisor } from "./EngineSupervisor.js";
import { normalizeProjectSavePath, readProjectFile, validateProjectOpenPath, writeProjectFile } from "./ProjectDialogs.js";

const require = createRequire(import.meta.url);
const { app, BrowserWindow, ipcMain, shell, dialog } = require("electron") as typeof import("electron");

const __dirname = dirname(fileURLToPath(import.meta.url));
const isDev = process.env.LOCAL_MIXER_DEV_SERVER === "1";
const devUrl = process.env.LOCAL_MIXER_DEV_URL ?? "http://127.0.0.1:5173";
const requireEngine = process.env.LOCAL_MIXER_REQUIRE_ENGINE === "1";
const supportedSoundPads = new Set(["applause", "laugh", "cheer", "drumroll", "ding", "whoosh"]);
const supportedEngineCommands = new Set([
  "engine-status",
  "channel-harmony-configure",
  "channel-harmony-set-enabled",
  "channel-harmony-snapshot",
  "fx-program-bank",
  "fx-unit-reset-macros",
  "fx-unit-select-program",
  "fx-unit-set-macro",
  "fx-unit-snapshot",
  "list-devices",
  "meter-input",
  "play-test-tone",
  "monitor-passthrough",
  "prepare-passthrough",
  "media-inspect",
  "media-import-start",
  "media-import-status",
  "media-import-cancel",
  "transport-play",
  "transport-pause",
  "transport-stop",
  "transport-seek",
  "transport-status",
  "routing-system-diagnostics",
  "routing-system-enable",
  "routing-system-disable",
  "routing-system-status",
  "routing-system-recover",
  "per-app-capture-capability",
  "sync-mixer-graph",
  "start-mixer-monitor",
  "stop-mixer-monitor",
  "mixer-monitor-status"
]);
const soundPadMaxMs = 6000;
let activeSoundPad: { child: ReturnType<typeof spawn>; timeout: ReturnType<typeof setTimeout> } | null = null;
let engineSupervisor: EngineSupervisor | null = null;

function soundPadHelperPath() {
  if (process.env.LOCAL_MIXER_SOUND_PAD_HELPER) return process.env.LOCAL_MIXER_SOUND_PAD_HELPER;
  if (app.isPackaged) return join(process.resourcesPath, "native", "sound-pad", "sound-pad-helper");
  return join(process.cwd(), "native", "sound-pad", "build", "sound-pad-helper");
}

function soundPadAssetDir() {
  if (process.env.LOCAL_MIXER_SOUND_PAD_ASSETS) return process.env.LOCAL_MIXER_SOUND_PAD_ASSETS;
  return resourcePath("assets", "sound-pads");
}

function nativeEnginePath() {
  if (process.env.LOCAL_MIXER_ENGINE_PATH) return process.env.LOCAL_MIXER_ENGINE_PATH;
  if (app.isPackaged) return join(process.resourcesPath, "native", "engine", "local-mixer-engine");
  return join(process.cwd(), "native", "engine", "build", "native", "engine", "local-mixer-engine");
}

function resourcePath(...parts: string[]) {
  if (app.isPackaged) return join(process.resourcesPath, ...parts);
  return join(process.cwd(), ...parts);
}

function validateRequiredEngine() {
  if (!requireEngine) return;
  const engine = nativeEnginePath();
  if (!existsSync(engine)) throw new Error(`Native engine required but missing: ${engine}`);
}

async function startEngineIfRequired() {
  if (!requireEngine) return;
  engineSupervisor = new EngineSupervisor({
    enginePath: nativeEnginePath(),
    commandTimeoutMs: 60_000,
    restoreSystemRouteOnStop: true,
    recoverSystemRouteOnStart: true
  });
  await engineSupervisor.start();
  await engineSupervisor.send("request-mic-permission");
}

function registerSoundPadIpc() {
  function stopActiveSoundPad() {
    const playback = activeSoundPad;
    if (!playback) return false;
    clearTimeout(playback.timeout);
    activeSoundPad = null;
    playback.child.kill("SIGTERM");
    return true;
  }

  ipcMain.handle("sound-pad:play", async (_event, padId: unknown) => {
    if (typeof padId !== "string" || !supportedSoundPads.has(padId)) {
      return { ok: false, error: "Unsupported sound pad" };
    }

    const helper = soundPadHelperPath();
    if (!existsSync(helper)) {
      return { ok: false, error: "Native sound pad helper is not built. Run npm run build:native:sound-pad." };
    }
    const assetDir = soundPadAssetDir();
    if (!existsSync(join(assetDir, `${padId}.wav`))) {
      return { ok: false, error: "Sound pad audio files are missing. Run npm run build:native:sound-pad." };
    }

    stopActiveSoundPad();
    return new Promise<{ ok: boolean; error?: string }>((resolve) => {
      const child = spawn(helper, ["--play", padId, assetDir], { stdio: ["ignore", "ignore", "pipe"] });
      let error = "";
      const timeout = setTimeout(() => {
        if (activeSoundPad?.child === child) stopActiveSoundPad();
      }, soundPadMaxMs);
      activeSoundPad = { child, timeout };

      child.stderr.on("data", (chunk: Buffer) => {
        error += chunk.toString("utf8");
      });
      child.on("error", (spawnError: Error) => resolve({ ok: false, error: spawnError.message }));
      child.on("close", (code: number | null, signal: NodeJS.Signals | null) => {
        if (activeSoundPad?.child === child) {
          clearTimeout(activeSoundPad.timeout);
          activeSoundPad = null;
        }
        if (code === 0 || signal === "SIGTERM") resolve({ ok: true });
        else resolve({ ok: false, error: error.trim() || `Native helper exited with code ${code ?? "unknown"}` });
      });
    });
  });

  ipcMain.handle("sound-pad:stop", async () => ({ ok: true, stopped: stopActiveSoundPad() }));
}

function registerMediaIpc() {
  ipcMain.handle("media:choose-file", async () => {
    const result = await dialog.showOpenDialog({
      properties: ["openFile"],
      filters: [
        { name: "Audio", extensions: ["wav", "aif", "aiff", "flac", "mp3", "m4a"] }
      ]
    });
    if (result.canceled || result.filePaths.length === 0) return { ok: true, canceled: true };
    return { ok: true, path: result.filePaths[0] };
  });
}

function registerProjectIpc() {
  ipcMain.handle("project:choose-open", async () => {
    const result = await dialog.showOpenDialog({
      properties: ["openFile"],
      filters: [
        { name: "Local Audio Mixer Project", extensions: ["lam.json"] },
        { name: "JSON", extensions: ["json"] }
      ]
    });
    if (result.canceled || result.filePaths.length === 0) return { ok: true, canceled: true };
    return validateProjectOpenPath(result.filePaths[0]);
  });

  ipcMain.handle("project:choose-save", async () => {
    const result = await dialog.showSaveDialog({
      defaultPath: "Untitled.lam.json",
      filters: [
        { name: "Local Audio Mixer Project", extensions: ["lam.json"] }
      ]
    });
    if (result.canceled || !result.filePath) return { ok: true, canceled: true };
    return { ok: true, path: normalizeProjectSavePath(result.filePath) };
  });

  ipcMain.handle("project:read", async (_event, path: unknown) => {
    if (typeof path !== "string") return { ok: false, error: "Invalid project path" };
    return readProjectFile(path);
  });

  ipcMain.handle("project:write", async (_event, payload: unknown) => {
    if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
      return { ok: false, error: "Invalid project write payload" };
    }
    const { path, content } = payload as { path?: unknown; content?: unknown };
    if (typeof path !== "string" || typeof content !== "string") {
      return { ok: false, error: "Invalid project write payload" };
    }
    return writeProjectFile(path, content);
  });
}

function registerEngineIpc() {
  ipcMain.handle("engine:command", async (_event, type: unknown, payload: unknown) => {
    if (typeof type !== "string" || !supportedEngineCommands.has(type)) {
      return { ok: false, error: "Unsupported engine command" };
    }
    if (!engineSupervisor || engineSupervisor.state !== "RUNNING") {
      return { ok: false, error: "Native engine is not running. Start with npm run dev:engine." };
    }
    if (payload !== undefined && (payload === null || typeof payload !== "object" || Array.isArray(payload))) {
      return { ok: false, error: "Invalid engine command payload" };
    }

    try {
      return await engineSupervisor.send(type, payload as Record<string, unknown> | undefined);
    } catch (error) {
      return { ok: false, error: error instanceof Error ? error.message : String(error) };
    }
  });
}

async function createWindow() {
  const window = new BrowserWindow({
    width: 1500,
    height: 930,
    minWidth: 1280,
    minHeight: 800,
    title: "Local Audio Mixer",
    backgroundColor: "#081217",
    show: false,
    webPreferences: {
      preload: join(__dirname, "preload.cjs"),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
      webSecurity: true
    }
  });

  window.webContents.setWindowOpenHandler(({ url }) => {
    void shell.openExternal(url);
    return { action: "deny" };
  });

  window.webContents.on("will-navigate", (event, url) => {
    if (isDev && url.startsWith(devUrl)) return;
    if (!isDev && url.startsWith("file://")) return;
    event.preventDefault();
  });

  window.once("ready-to-show", () => window.show());

  if (isDev) {
    await window.loadURL(devUrl);
  } else {
    await window.loadFile(join(__dirname, "../ui/index.html"));
  }
}

app.whenReady().then(async () => {
  validateRequiredEngine();
  await startEngineIfRequired();
  registerSoundPadIpc();
  registerMediaIpc();
  registerProjectIpc();
  registerEngineIpc();
  await createWindow();
  app.on("activate", async () => {
    if (BrowserWindow.getAllWindows().length === 0) await createWindow();
  });
});

app.on("window-all-closed", () => {
  app.quit();
});

app.on("before-quit", () => {
  void engineSupervisor?.stop();
});
