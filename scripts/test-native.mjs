import { spawnSync } from "node:child_process";
import { spawn } from "node:child_process";
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
  console.log("engine protocol smoke ok");
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
