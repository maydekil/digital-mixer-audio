import { existsSync, mkdirSync, readFileSync, statSync } from "node:fs";
import { join } from "node:path";
import { spawnSync } from "node:child_process";

const outDir = join("native", "sound-pad", "build");
const output = join(outDir, "sound-pad-helper");
const assetDir = join("assets", "sound-pads");
const sources = [
  join("native", "sound-pad", "src", "main.cpp"),
  join("native", "sound-pad", "src", "SoundPadSynth.cpp"),
  join("native", "sound-pad", "src", "NativeAudioPlayer.cpp"),
  join("native", "sound-pad", "src", "WavWriter.cpp")
];
const requiredAssets = ["applause", "laugh", "cheer", "drumroll", "ding", "whoosh"];

mkdirSync(outDir, { recursive: true });
mkdirSync(assetDir, { recursive: true });

const build = spawnSync("clang++", [
  "-std=c++20",
  "-Wall",
  "-Wextra",
  "-Werror",
  "-O2",
  ...sources,
  "-o",
  output
], { stdio: "inherit" });

if (build.status !== 0) process.exit(build.status ?? 1);

for (const padId of requiredAssets) {
  const assetPath = join(assetDir, `${padId}.wav`);
  if (!existsSync(assetPath)) {
    console.error(`Missing sound pad asset: ${assetPath}`);
    process.exit(1);
  }

  const header = readFileSync(assetPath).subarray(0, 12);
  const isWav = header.subarray(0, 4).toString("ascii") === "RIFF" &&
    header.subarray(8, 12).toString("ascii") === "WAVE";

  if (!isWav || statSync(assetPath).size < 4096) {
    console.error(`Invalid sound pad WAV asset: ${assetPath}`);
    process.exit(1);
  }
}

console.log(`Built ${output}`);
console.log(`Verified ${requiredAssets.length} persistent WAV sound pad assets in ${assetDir}`);
