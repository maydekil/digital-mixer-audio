import { spawnSync } from "node:child_process";
import { cpSync, existsSync, mkdirSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { basename, join } from "node:path";

const appName = "Local Audio Mixer";
const bundleId = "audio.local-mixer.dev";
const packageRoot = join("build", "package");
const appPath = join(packageRoot, `${appName}.app`);
const resourcesPath = join(appPath, "Contents", "Resources");
const appResourcesPath = join(resourcesPath, "app");
const archivePath = join(packageRoot, `${appName.replaceAll(" ", "-")}-dev.zip`);
const reportPath = join("docs", "reports", "int02-package-smoke.md");

run("build UI/desktop main", "npm", ["run", "build:ui"]);
run("build native engine", "npm", ["run", "build:native"]);
run("build native sound pad", "npm", ["run", "build:native:sound-pad"]);

prepareBundle();
writePackageApp();
copyRuntimeResources();
writeInfoPlist();
writePkgInfo();
createArchive();
writeReport();

function prepareBundle() {
  const electronApp = join("node_modules", "electron", "dist", "Electron.app");
  if (!existsSync(electronApp)) fail(`Missing Electron runtime: ${electronApp}`);
  rmSync(packageRoot, { recursive: true, force: true });
  mkdirSync(packageRoot, { recursive: true });
  cpSync(electronApp, appPath, { recursive: true });
}

function writePackageApp() {
  mkdirSync(appResourcesPath, { recursive: true });
  cpSync("dist", join(appResourcesPath, "dist"), { recursive: true });
  writeFileSync(
    join(appResourcesPath, "package.json"),
    JSON.stringify({ name: "local-audio-mixer-packaged", version: packageVersion(), main: "dist/electron/main.js" }, null, 2)
  );
}

function copyRuntimeResources() {
  const nativeEngineDir = join(resourcesPath, "native", "engine");
  const nativeSoundPadDir = join(resourcesPath, "native", "sound-pad");
  const soundPadAssetsDir = join(resourcesPath, "assets", "sound-pads");
  mkdirSync(nativeEngineDir, { recursive: true });
  mkdirSync(nativeSoundPadDir, { recursive: true });
  mkdirSync(soundPadAssetsDir, { recursive: true });
  cpSync(join("native", "engine", "build", "native", "engine", "local-mixer-engine"), join(nativeEngineDir, "local-mixer-engine"));
  cpSync(join("native", "engine", "build", "native", "engine", "local-mixer-plugin-scanner"), join(nativeEngineDir, "local-mixer-plugin-scanner"));
  cpSync(join("native", "sound-pad", "build", "sound-pad-helper"), join(nativeSoundPadDir, "sound-pad-helper"));
  for (const asset of ["applause.wav", "laugh.wav", "cheer.wav", "drumroll.wav", "ding.wav", "whoosh.wav"]) {
    cpSync(join("assets", "sound-pads", asset), join(soundPadAssetsDir, asset));
  }
}

function writeInfoPlist() {
  const plistPath = join(appPath, "Contents", "Info.plist");
  const plist = readFileSync(plistPath, "utf8")
    .replace(/<string>org\.electronjs\.electron<\/string>/, `<string>${bundleId}</string>`)
    .replace(/<string>Electron<\/string>/g, `<string>${appName}</string>`)
    .replace(
      "</dict>",
      [
        "  <key>NSMicrophoneUsageDescription</key>",
        "  <string>Local Audio Mixer needs microphone access for native live input monitoring and recording.</string>",
        "  <key>NSScreenCaptureUsageDescription</key>",
        "  <string>Local Audio Mixer may request system audio capture permission for supported Core Audio tap workflows.</string>",
        "</dict>"
      ].join("\n")
    );
  writeFileSync(plistPath, plist);
}

function writePkgInfo() {
  writeFileSync(join(appPath, "Contents", "PkgInfo"), "APPL????");
}

function createArchive() {
  rmSync(archivePath, { force: true });
  run("create dev archive", "ditto", ["-c", "-k", "--keepParent", basename(appPath), basename(archivePath)], {
    cwd: packageRoot
  });
}

function writeReport() {
  const smoke = [
    binaryInfo(join(appPath, "Contents", "MacOS", "Electron")),
    binaryInfo(join(resourcesPath, "native", "engine", "local-mixer-engine")),
    binaryInfo(join(resourcesPath, "native", "engine", "local-mixer-plugin-scanner")),
    binaryInfo(join(resourcesPath, "native", "sound-pad", "sound-pad-helper")),
    runCapture("packaged engine version", join(resourcesPath, "native", "engine", "local-mixer-engine"), ["--version"]),
    runCapture("packaged plugin scanner self-test", join(resourcesPath, "native", "engine", "local-mixer-plugin-scanner"), ["--self-test"]),
    runCapture("packaged sound pad validate", join(resourcesPath, "native", "sound-pad", "sound-pad-helper"), ["--validate"])
  ];
  mkdirSync(join("docs", "reports"), { recursive: true });
  writeFileSync(reportPath, [
    "# INT-02 Package Smoke",
    "",
    `Bundle: \`${appPath}\``,
    `Archive: \`${archivePath}\``,
    `Revision: \`${gitRevision()}\``,
    "",
    "## Automated Smoke",
    "",
    "| Check | Result | Evidence |",
    "| --- | --- | --- |",
    ...smoke.map((item) => `| ${item.label} | ${item.ok ? "PASS" : "FAIL"} | ${escapeCell(item.detail)} |`),
    "",
    "## Permission And Offline Notes",
    "",
    "- `NSMicrophoneUsageDescription` is present in `Info.plist` for packaged-app TCC identity.",
    "- Runtime GUI launch and macOS permission prompt from the `.app` are NOT_RUN in this command flow.",
    "- Native engine, plugin scanner, sound-pad helper, UI bundle, and sound-pad assets are copied into app resources.",
    "- This unsigned dev package is not notarized or suitable for public distribution.",
    ""
  ].join("\n"));
  console.log(`wrote ${reportPath}`);
  console.log(`created ${appPath}`);
  console.log(`created ${archivePath}`);
}

function run(label, command, args, options = {}) {
  const result = spawnSync(command, args, { stdio: "inherit", shell: false, ...options });
  if (result.status !== 0) fail(`${label} failed`);
}

function runCapture(label, command, args) {
  const result = spawnSync(command, args, { encoding: "utf8", shell: false });
  return {
    label,
    ok: result.status === 0,
    detail: (result.stdout + result.stderr).trim() || `exit ${result.status ?? "unknown"}`
  };
}

function binaryInfo(path) {
  const result = spawnSync("file", [path], { encoding: "utf8", shell: false });
  return { label: `file ${basename(path)}`, ok: result.status === 0, detail: result.stdout.trim() };
}

function packageVersion() {
  return JSON.parse(readFileSync("package.json", "utf8")).version ?? "0.0.0";
}

function gitRevision() {
  const result = spawnSync("git", ["rev-parse", "--short", "HEAD"], { encoding: "utf8", shell: false });
  return result.status === 0 ? result.stdout.trim() : "UNKNOWN";
}

function escapeCell(value) {
  return value.replace(/\|/g, "\\|").replace(/\n/g, "<br>");
}

function fail(message) {
  console.error(message);
  process.exit(1);
}
