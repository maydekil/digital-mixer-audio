import { execFileSync } from "node:child_process";
import { readFileSync } from "node:fs";

const checks = [
  ["Architecture", "uname", ["-m"]],
  ["macOS", "sw_vers", []],
  ["Xcode tools", "xcode-select", ["-p"]],
  ["macOS SDK path", "xcrun", ["--show-sdk-path"]],
  ["macOS SDK version", "xcrun", ["--sdk", "macosx", "--show-sdk-version"]],
  ["Apple Clang", "clang", ["--version"]],
  ["CMake", "cmake", ["--version"]],
  ["Ninja", "ninja", ["--version"]],
  ["Node", "node", ["--version"]],
  ["npm", "npm", ["--version"]]
];

console.log("# Local Audio Mixer Doctor\n");
console.log(`Generated: ${new Date().toISOString()}\n`);
console.log("| Check | Result |");
console.log("| --- | --- |");

for (const [label, command, args] of checks) {
  const result = run(command, args);
  console.log(`| ${label} | ${escapeCell(result)} |`);
}

console.log("\n## Package Pins\n");
const packageJson = JSON.parse(readFileSync("package.json", "utf8"));
const dependencies = { ...(packageJson.dependencies ?? {}), ...(packageJson.devDependencies ?? {}) };
for (const name of Object.keys(dependencies).sort()) {
  console.log(`- ${name}: ${dependencies[name]}`);
}

function run(command, args) {
  try {
    return execFileSync(command, args, { encoding: "utf8", stdio: ["ignore", "pipe", "pipe"] }).trim() || "OK";
  } catch (error) {
    const message = error.stderr?.toString("utf8").trim() || error.message;
    return `NOT_AVAILABLE: ${message}`;
  }
}

function escapeCell(value) {
  return value.replace(/\n/g, "<br>").replace(/\|/g, "\\|");
}
