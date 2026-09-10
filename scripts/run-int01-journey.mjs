import { spawn, spawnSync } from "node:child_process";
import { mkdirSync, mkdtempSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

const buildDir = join("native", "engine", "build");
const enginePath = join(buildDir, "native", "engine", "local-mixer-engine");
const reportPath = join("docs", "reports", "int01-daily-journey.md");
const evidence = [];

main().catch((error) => {
  console.error(error instanceof Error ? error.message : String(error));
  process.exit(1);
});

async function main() {
  runStep("build native engine", process.execPath, [join("scripts", "build-native.mjs")]);
  runStep("record/export/session native integrity tests", "ctest", [
    "--test-dir",
    buildDir,
    "-R",
    "local-mixer-(recording|export|session-document)-tests",
    "--output-on-failure"
  ]);
  await runProtocolJourney();
  writeReport();
}

function runStep(label, command, args) {
  const result = spawnSync(command, args, { encoding: "utf8", shell: false });
  evidence.push({
    label,
    ok: result.status === 0,
    detail: (result.stdout + result.stderr).trim().split("\n").slice(-8).join("\n")
  });
  if (result.status !== 0) {
    process.stderr.write(result.stdout);
    process.stderr.write(result.stderr);
    process.exit(result.status ?? 1);
  }
}

async function runProtocolJourney() {
  const media = writeWavFixture();
  const child = spawn(enginePath, ["--stdio"], { stdio: ["pipe", "pipe", "pipe"], shell: false });
  const exit = waitForExit(child);
  const messages = [];
  let currentStep = "handshake";
  let stderr = "";
  let buffer = "";
  child.stdout.on("data", (chunk) => {
    buffer += chunk.toString("utf8");
    for (;;) {
      const newline = buffer.indexOf("\n");
      if (newline < 0) return;
      const line = buffer.slice(0, newline);
      buffer = buffer.slice(newline + 1);
      messages.push(JSON.parse(line));
    }
  });
  child.stderr.on("data", (chunk) => {
    stderr += chunk.toString("utf8");
  });

  await waitForMessage(exit, messages, currentStep, stderr, (message) => message.type === "hello");
  currentStep = "import start";
  await sendAndWait(child, exit, messages, currentStep, "int01-import", "media-import-start", {
    path: media.path,
    framesPerPoint: 1
  }, (message) => message.state === "queued");
  const jobId = messages.find((message) => message.id === "int01-import")?.jobId;
  currentStep = "import status";
  await sendAndWait(child, exit, messages, currentStep, "int01-import-status", "media-import-status", {
    jobId
  }, (message) => message.state === "completed" && message.waveformPoints === 2);
  currentStep = "transport play";
  await sendAndWait(child, exit, messages, currentStep, "int01-transport-play", "transport-play", {}, (message) => message.state === "playing");
  currentStep = "fx bank";
  await sendAndWait(child, exit, messages, currentStep, "int01-fx-bank", "fx-program-bank", {}, (message) => Array.isArray(message.programs));
  currentStep = "fx a select";
  await sendAndWait(child, exit, messages, currentStep, "int01-fx-a", "fx-unit-select-program", {
    unitId: "fx-a",
    programId: 12,
    expectedRevision: 0
  }, (message) => message.accepted === true);
  currentStep = "harmony enable";
  await sendAndWait(child, exit, messages, currentStep, "int01-harmony", "channel-harmony-set-enabled", {
    channelId: "voice",
    enabled: true,
    expectedRevision: 0
  }, (message) => message.accepted === true && message.desiredEnabled === true);
  currentStep = "invalid file";
  await sendAndWait(child, exit, messages, currentStep, "int01-invalid-file", "media-inspect", {
    path: join(media.dir, "missing.wav")
  }, (message) => message.ok === false || message.imported === false);
  currentStep = "per-app capability";
  await sendAndWait(child, exit, messages, currentStep, "int01-per-app", "per-app-capture-capability", {}, (message) =>
    typeof message.platformSupported === "boolean"
  );
  currentStep = "transport stop";
  await sendAndWait(child, exit, messages, currentStep, "int01-stop", "transport-stop", {}, (message) => message.state === "stopped");
  currentStep = "shutdown";
  child.stdin.write(`${JSON.stringify({ id: "int01-shutdown", type: "shutdown" })}\n`);
  child.stdin.end();
  await exit;
  rmSync(media.dir, { recursive: true, force: true });
  evidence.push({
    label: "stdio daily journey contract",
    ok: true,
    detail: "import, waveform status, transport, FX A program, harmony, invalid-file fault, per-app capability, stop, shutdown"
  });
}

function sendAndWait(child, exit, messages, step, id, type, payload, predicate) {
  child.stdin.write(`${JSON.stringify({ id, type, ...payload })}\n`);
  return waitForMessage(exit, messages, step, "", (message) => message.id === id && predicate(message));
}

function writeWavFixture() {
  const dir = mkdtempSync(join(tmpdir(), "local-mixer-int01-"));
  const path = join(dir, "backing.wav");
  const dataBytes = 4;
  const buffer = Buffer.alloc(44 + dataBytes);
  buffer.write("RIFF", 0);
  buffer.writeUInt32LE(36 + dataBytes, 4);
  buffer.write("WAVE", 8);
  buffer.write("fmt ", 12);
  buffer.writeUInt32LE(16, 16);
  buffer.writeUInt16LE(1, 20);
  buffer.writeUInt16LE(1, 22);
  buffer.writeUInt32LE(48000, 24);
  buffer.writeUInt32LE(96000, 28);
  buffer.writeUInt16LE(2, 32);
  buffer.writeUInt16LE(16, 34);
  buffer.write("data", 36);
  buffer.writeUInt32LE(dataBytes, 40);
  buffer.writeInt16LE(0, 44);
  buffer.writeInt16LE(8192, 46);
  writeFileSync(path, buffer);
  return { dir, path };
}

function waitFor(predicate) {
  return new Promise((resolve, reject) => {
    const started = Date.now();
    const interval = setInterval(() => {
      if (predicate()) {
        clearInterval(interval);
        resolve();
      } else if (Date.now() - started > 1500) {
        clearInterval(interval);
        reject(new Error("Timed out waiting for INT-01 journey step"));
      }
    }, 10);
  });
}

function waitForMessage(exit, messages, step, stderr, predicate) {
  return Promise.race([
    waitFor(() => messages.some(predicate)),
    exit.then(() => {
      throw new Error(`Engine exited before INT-01 journey step completed: ${step}${stderr ? `\n${stderr}` : ""}`);
    })
  ]);
}

function waitForExit(child) {
  return new Promise((resolve, reject) => {
    child.on("close", (code) => {
      if (code === 0) resolve();
      else reject(new Error(`INT-01 child exited with ${code}`));
    });
  });
}

function writeReport() {
  mkdirSync(join("docs", "reports"), { recursive: true });
  const lines = [
    "# INT-01 Daily Journey Evidence",
    "",
    `Generated by \`node scripts/run-int01-journey.mjs\`.`,
    "",
    "## Automated Evidence",
    "",
    "| Step | Result | Evidence |",
    "| --- | --- | --- |",
    ...evidence.map((item) => `| ${item.label} | ${item.ok ? "PASS" : "FAIL"} | ${escapeCell(item.detail)} |`),
    "",
    "## Coverage",
    "",
    "| Scenario Item | Status | Notes |",
    "| --- | --- | --- |",
    "| import backing | PASS | WAV fixture imported through native media job and waveform status. |",
    "| mic monitor | NOT_RUN | Requires real input/output device selection in desktop session. |",
    "| A/B effects | PARTIAL | FX A program ACK covered; audible dual-return journey remains pending. |",
    "| harmony | PARTIAL | Native ACK covered; real vocal audition remains pending. |",
    "| record/stop/playback take | PARTIAL | Recording writer integrity test passes; full live take replay command flow is not exposed yet. |",
    "| save/open | PASS_CONTRACT | Session document roundtrip test passes. |",
    "| export/offline playback | PARTIAL | Export writer/stem tests pass; desktop export dialog and playback of exported result pending. |",
    "| fault injection | PARTIAL | Invalid-file and cancel/status paths covered; slow disk/disconnect fault injection pending. |",
    "",
    "## Result",
    "",
    "INT-01 is implemented as an automated contract journey and data-integrity audit. It is not PRODUCT_VERIFIED because hardware mic monitoring, live record/replay, and packaged desktop export playback have not all been run.",
    ""
  ];
  writeFileSync(reportPath, `${lines.join("\n")}\n`);
  console.log(`wrote ${reportPath}`);
}

function escapeCell(value) {
  return value.replace(/\|/g, "\\|").replace(/\n/g, "<br>");
}
