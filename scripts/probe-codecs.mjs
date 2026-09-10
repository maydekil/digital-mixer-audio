import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { spawnSync } from "node:child_process";

const ffmpeg = process.env.LOCAL_MIXER_FFMPEG_PATH || findOnPath("ffmpeg");
const reportPath = join("docs", "reports", "codec-probe-phase06.md");
const requiredDecoders = [
  { id: "aac", label: "M4A/AAC" },
  { id: "flac", label: "FLAC" },
  { id: "mp3", label: "MP3" },
  { id: "pcm_s16le", label: "WAV PCM" },
  { id: "pcm_f32le", label: "WAV float" }
];

if (!ffmpeg) {
  writeReport({
    ffmpeg: "",
    version: "NOT_FOUND",
    decoders: new Set(),
    ok: false
  });
  process.exitCode = 1;
} else {
  const version = run(ffmpeg, ["-hide_banner", "-version"]);
  const decoders = run(ffmpeg, ["-hide_banner", "-decoders"]);
  if (version.status !== 0 || decoders.status !== 0) {
    writeReport({
      ffmpeg,
      version: version.output.trim() || "ERROR",
      decoders: new Set(),
      ok: false
    });
    process.exitCode = 1;
  } else {
    const decoderSet = parseDecoders(decoders.output);
    const ok = requiredDecoders.every((decoder) => decoderSet.has(decoder.id));
    writeReport({
      ffmpeg,
      version: firstLine(version.output),
      decoders: decoderSet,
      ok
    });
    if (!ok) process.exitCode = 1;
  }
}

function findOnPath(binary) {
  for (const entry of (process.env.PATH || "").split(":")) {
    if (!entry) continue;
    const candidate = join(entry, binary);
    const result = spawnSync(candidate, ["-version"], { encoding: "utf8", shell: false });
    if (result.status === 0) return candidate;
  }
  return "";
}

function run(command, args) {
  const result = spawnSync(command, args, { encoding: "utf8", shell: false, maxBuffer: 4 * 1024 * 1024 });
  return {
    status: result.status ?? 1,
    output: `${result.stdout || ""}${result.stderr || ""}`
  };
}

function parseDecoders(text) {
  const decoders = new Set();
  for (const line of text.split("\n")) {
    const match = line.match(/^\s*A[.FSXBD]{5}\s+([^\s]+)/);
    if (match) decoders.add(match[1]);
  }
  return decoders;
}

function firstLine(text) {
  return text.split("\n").find((line) => line.trim())?.trim() || "";
}

function writeReport({ ffmpeg, version, decoders, ok }) {
  mkdirSync(dirname(reportPath), { recursive: true });
  const lines = [
    "# Phase06 Codec Probe",
    "",
    `Status: ${ok ? "PASS" : "FAIL"}`,
    `FFmpeg path: ${ffmpeg || "NOT_FOUND"}`,
    `Version: ${version}`,
    "",
    "| Format | Decoder | Status |",
    "| --- | --- | --- |",
    ...requiredDecoders.map((decoder) =>
      `| ${decoder.label} | ${decoder.id} | ${decoders.has(decoder.id) ? "available" : "missing"} |`
    ),
    "",
    "Notes:",
    "- Probe uses argv spawning with `shell:false`.",
    "- This proves local development decoder availability only; packaged end-user FFmpeg bundling remains a later packaging requirement.",
    "- Browser audio APIs are not used.",
    ""
  ];
  writeFileSync(reportPath, lines.join("\n"));
  console.log(`${ok ? "codec probe ok" : "codec probe failed"} (${reportPath})`);
}
