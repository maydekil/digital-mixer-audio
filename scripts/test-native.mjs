import { spawnSync } from "node:child_process";
import { spawn } from "node:child_process";
import { mkdtempSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { join } from "node:path";

const buildDir = join("native", "engine", "build");
const enginePath = join(buildDir, "native", "engine", "local-mixer-engine");

run(process.execPath, [join("scripts", "build-native.mjs")]);
run("ctest", ["--test-dir", buildDir, "--output-on-failure"]);
run(enginePath, ["--self-test"]);
run(enginePath, ["--version"]);
testDeviceEnumeration();
await testEngineProtocol();

function run(command, args) {
  const result = spawnSync(command, args, { stdio: "inherit", shell: false });
  if (result.status !== 0) process.exit(result.status ?? 1);
}

function testDeviceEnumeration() {
  const result = spawnSync(enginePath, ["--list-devices"], { encoding: "utf8", shell: false });
  if (result.status !== 0) {
    process.stderr.write(result.stderr);
    process.exit(result.status ?? 1);
  }

  const payload = JSON.parse(result.stdout);
  if (!Array.isArray(payload.devices)) {
    throw new Error("Engine device enumeration did not return a devices array");
  }

  console.log(`engine device enumeration smoke ok (${payload.devices.length} devices)`);
}

async function testEngineProtocol() {
  const mediaFixture = writeWavFixture();
  const child = spawn(enginePath, ["--stdio"], { stdio: ["pipe", "pipe", "pipe"], shell: false });
  const exit = waitForExit(child);
  const messages = [];
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

  await waitFor(() => messages.some((message) => message.type === "hello"));
  child.stdin.write(`${JSON.stringify({ id: "native-ping", type: "ping" })}\n`);
  await waitFor(() => messages.some((message) => message.id === "native-ping" && message.type === "pong"));
  child.stdin.write(`${JSON.stringify({ id: "native-devices", type: "list-devices" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-devices" &&
      message.type === "devices" &&
      Array.isArray(message.devices)
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-prepare", type: "prepare-passthrough" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-prepare" &&
      message.type === "prepare-passthrough" &&
      typeof message.status === "object"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-media-inspect", type: "media-inspect", path: mediaFixture.path })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-media-inspect" &&
      message.type === "media-inspect" &&
      message.imported === true &&
      message.container === "wav" &&
      message.channels === 1 &&
      message.frameCount === 2
    )
  );
  child.stdin.write(`${JSON.stringify({
    id: "native-media-import-start",
    type: "media-import-start",
    path: mediaFixture.path,
    framesPerPoint: 1
  })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-media-import-start" &&
      message.type === "media-import-status" &&
      message.state === "queued" &&
      typeof message.jobId === "string"
    )
  );
  const importJobId = messages.find((message) => message.id === "native-media-import-start")?.jobId;
  child.stdin.write(`${JSON.stringify({ id: "native-media-import-status", type: "media-import-status", jobId: importJobId })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-media-import-status" &&
      message.type === "media-import-status" &&
      message.state === "completed" &&
      message.waveformPoints === 2
    )
  );
  child.stdin.write(`${JSON.stringify({
    id: "native-media-import-start-cancel",
    type: "media-import-start",
    path: mediaFixture.path,
    framesPerPoint: 1
  })}\n`);
  await waitFor(() => messages.some((message) => message.id === "native-media-import-start-cancel" && typeof message.jobId === "string"));
  const cancelJobId = messages.find((message) => message.id === "native-media-import-start-cancel")?.jobId;
  child.stdin.write(`${JSON.stringify({ id: "native-media-import-cancel", type: "media-import-cancel", jobId: cancelJobId })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-media-import-cancel" &&
      message.type === "media-import-status" &&
      message.state === "canceled"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-transport-play", type: "transport-play" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-transport-play" &&
      message.type === "transport-status" &&
      message.state === "playing"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-transport-seek", type: "transport-seek", positionFrame: 24000 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-transport-seek" &&
      message.type === "transport-status" &&
      message.positionFrame === 24000 &&
      message.bufferGeneration === 1
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-transport-stop", type: "transport-stop" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-transport-stop" &&
      message.type === "transport-status" &&
      message.state === "stopped" &&
      message.positionFrame === 0 &&
      message.bufferGeneration === 2
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-route-diagnostics", type: "routing-system-diagnostics", sampleRate: 48000 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-route-diagnostics" &&
      message.type === "routing-system-diagnostics" &&
      typeof message.routeValid === "boolean" &&
      typeof message.error === "string"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-route-enable", type: "routing-system-enable", sampleRate: 48000 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-route-enable" &&
      message.type === "routing-system-enable" &&
      message.ok === false &&
      typeof message.state === "string" &&
      typeof message.error === "string"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-route-status", type: "routing-system-status" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-route-status" &&
      message.type === "routing-system-status" &&
      typeof message.ownsSystemRoute === "boolean" &&
      typeof message.recoveryMarkerPresent === "boolean"
    )
  );
  const routeStatus = messages.find((message) => message.id === "native-route-status" && message.type === "routing-system-status");
  if (routeStatus?.recoveryMarkerPresent !== true) {
    child.stdin.write(`${JSON.stringify({ id: "native-route-recover", type: "routing-system-recover" })}\n`);
    await waitFor(() =>
      messages.some((message) =>
        message.id === "native-route-recover" &&
        message.type === "routing-system-recover" &&
        message.ok === false &&
        message.error === "NO_RECOVERY_MARKER"
      )
    );
  }
  child.stdin.write(`${JSON.stringify({ id: "native-route-disable", type: "routing-system-disable" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-route-disable" &&
      message.type === "routing-system-disable" &&
      message.ok === false &&
      message.error === "NO_OWNED_ROUTE"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-per-app-capability", type: "per-app-capture-capability" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-per-app-capability" &&
      message.type === "per-app-capture-capability" &&
      typeof message.platformSupported === "boolean" &&
      Array.isArray(message.sources)
    )
  );
  child.stdin.write(`${JSON.stringify({
    id: "native-sync-graph",
    type: "sync-mixer-graph",
    channelCount: 2,
    channel0Id: "voice",
    channel0Kind: "source",
    channel0Name: "VOICE",
    channel0Color: "#18d6e7",
    channel0SourceUid: "mic",
    channel0Assignment: "mono",
    channel0Enabled: true,
    channel0Mute: false,
    channel0Solo: false,
    channel0Monitor: true,
    channel0TrimDb: 0,
    channel0FaderDb: -6,
    channel0Pan: 0,
    channel1Id: "master",
    channel1Kind: "master",
    channel1Name: "MASTER",
    channel1Color: "#20f0a0",
    channel1SourceUid: "",
    channel1Assignment: "stereo",
    channel1Enabled: true,
    channel1Mute: false,
    channel1Solo: false,
    channel1Monitor: false,
    channel1TrimDb: 0,
    channel1FaderDb: -1,
    channel1Pan: 0
  })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-sync-graph" &&
      message.type === "sync-mixer-graph" &&
      message.synced === true &&
      message.stripCount === 2 &&
      message.activeMonitorCount === 1
    )
  );
  child.stdin.write(`${JSON.stringify({
    id: "native-sync-multi-monitor",
    type: "sync-mixer-graph",
    channelCount: 2,
    channel0Kind: "source",
    channel0Name: "VOICE",
    channel0SourceUid: "mic-a",
    channel0Assignment: "mono",
    channel0Enabled: true,
    channel0Monitor: true,
    channel1Kind: "source",
    channel1Name: "GUITAR",
    channel1SourceUid: "mic-b",
    channel1Assignment: "mono",
    channel1Enabled: true,
    channel1Monitor: true
  })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-sync-multi-monitor" &&
      message.type === "sync-mixer-graph" &&
      message.synced === true &&
      message.activeMonitorCount === 2
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-monitor-start", type: "start-mixer-monitor", sampleRate: 48000 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-monitor-start" &&
      message.type === "start-mixer-monitor" &&
      typeof message.monitoring === "boolean" &&
      typeof message.error === "string"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-monitor-status", type: "mixer-monitor-status" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-monitor-status" &&
      message.type === "mixer-monitor-status" &&
      typeof message.monitoring === "boolean"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-monitor-stop", type: "stop-mixer-monitor" })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-monitor-stop" &&
      message.type === "stop-mixer-monitor" &&
      message.monitoring === false
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-tone", type: "play-test-tone", durationMs: 0 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-tone" &&
      message.type === "play-test-tone" &&
      typeof message.played === "boolean"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-meter", type: "meter-input", durationMs: 0 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-meter" &&
      message.type === "meter-input" &&
      typeof message.measured === "boolean"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-monitor", type: "monitor-passthrough", durationMs: 0 })}\n`);
  await waitFor(() =>
    messages.some((message) =>
      message.id === "native-monitor" &&
      message.type === "monitor-passthrough" &&
      typeof message.monitored === "boolean"
    )
  );
  child.stdin.write(`${JSON.stringify({ id: "native-shutdown", type: "shutdown" })}\n`);
  await waitFor(() => messages.some((message) => message.id === "native-shutdown" && message.type === "bye"));
  child.stdin.end();
  await exit;
  rmSync(mediaFixture.dir, { recursive: true, force: true });
  console.log("engine protocol smoke ok");
}

function writeWavFixture() {
  const dir = mkdtempSync(join(tmpdir(), "local-mixer-native-media-"));
  const path = join(dir, "fixture.wav");
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
  buffer.writeInt16LE(16384, 46);
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
      } else if (Date.now() - started > 1000) {
        clearInterval(interval);
        reject(new Error("Timed out waiting for engine protocol message"));
      }
    }, 10);
  });
}

function waitForExit(child) {
  return new Promise((resolve, reject) => {
    child.on("close", (code) => {
      if (code === 0) resolve();
      else reject(new Error(`Engine protocol child exited with ${code}`));
    });
  });
}
