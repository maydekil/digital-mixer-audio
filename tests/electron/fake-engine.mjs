import { createInterface } from "node:readline";

const mode = process.argv[2] ?? "normal";

if (mode === "malformed") {
  console.log("not-json");
  setTimeout(() => process.exit(0), 50);
} else if (mode === "silent") {
  setTimeout(() => process.exit(0), 50);
} else if (mode === "oversized") {
  console.log("x".repeat(9000));
  setTimeout(() => process.exit(0), 50);
} else {
  console.log(JSON.stringify({ type: "hello", protocol: 1, state: "RUNNING", audio: "not-started" }));
  if (mode === "exit-after-hello") setTimeout(() => process.exit(0), 20);
}

const input = createInterface({ input: process.stdin });

input.on("line", (line) => {
  let message;
  try {
    message = JSON.parse(line);
  } catch {
    console.log(JSON.stringify({ type: "error", ok: false, error: "MALFORMED_JSON" }));
    return;
  }

  if (message.type === "ping") {
    console.log(JSON.stringify({ id: message.id, type: "pong", ok: true }));
    return;
  }

  if (message.type === "never") return;

  if (message.type === "crash") {
    console.error("fake crash");
    process.exit(70);
  }

  if (message.type === "shutdown") {
    console.log(JSON.stringify({ id: message.id, type: "bye", ok: true }));
    process.exit(0);
  }
});
