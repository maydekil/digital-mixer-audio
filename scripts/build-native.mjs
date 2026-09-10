import { mkdirSync } from "node:fs";
import { spawnSync } from "node:child_process";
import { join } from "node:path";

const buildDir = join("native", "engine", "build");

mkdirSync(buildDir, { recursive: true });

run("cmake", [
  "-S",
  ".",
  "-B",
  buildDir,
  "-G",
  "Ninja",
  "-DCMAKE_BUILD_TYPE=Debug",
  "-DCMAKE_OSX_DEPLOYMENT_TARGET=14.0"
]);

run("cmake", ["--build", buildDir]);
run(process.execPath, [join("scripts", "build-native-sound-pad.mjs")]);

function run(command, args) {
  const result = spawnSync(command, args, { stdio: "inherit", shell: false });
  if (result.status !== 0) process.exit(result.status ?? 1);
}
