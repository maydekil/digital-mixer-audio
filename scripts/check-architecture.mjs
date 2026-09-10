import { readdirSync, readFileSync, statSync } from "node:fs";
import { join } from "node:path";

const root = "apps/desktop/src";
const rules = [
  {
    owner: "components/ui",
    forbidden: ["/features/", "/state/", "/adapters/", "electron"]
  },
  {
    owner: "components/audio",
    forbidden: ["/features/", "/adapters/", "electron"]
  },
  {
    owner: "features",
    forbidden: ["AudioContext", "webkitAudioContext", "OfflineAudioContext", "AudioWorklet", "MediaRecorder", "getUserMedia", "getDisplayMedia", "HTMLAudioElement"]
  },
  {
    owner: "adapters",
    forbidden: ["AudioContext", "webkitAudioContext", "OfflineAudioContext", "AudioWorklet", "MediaRecorder", "getUserMedia", "getDisplayMedia"]
  }
];

let failed = false;

function files(dir) {
  const out = [];
  for (const name of readdirSync(dir)) {
    const path = join(dir, name);
    const stat = statSync(path);
    if (stat.isDirectory()) out.push(...files(path));
    if (stat.isFile() && /\.(ts|tsx|js|jsx)$/.test(path)) out.push(path);
  }
  return out;
}

for (const path of files(root)) {
  const text = readFileSync(path, "utf8");
  for (const rule of rules) {
    if (!path.includes(rule.owner)) continue;
    for (const forbidden of rule.forbidden) {
      if (text.includes(forbidden)) {
        console.error(`Architecture violation: ${path} contains ${forbidden}`);
        failed = true;
      }
    }
  }
}

if (failed) process.exit(1);
console.log("check:architecture ok");
