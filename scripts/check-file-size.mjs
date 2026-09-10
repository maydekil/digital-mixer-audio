import { readdirSync, readFileSync, statSync } from "node:fs";
import { join, extname } from "node:path";

const sourceExts = new Set([
  ".ts", ".tsx", ".js", ".jsx", ".mjs", ".cjs", ".css", ".scss", ".html",
  ".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hh", ".hxx", ".mm", ".m",
  ".cmake", ".sh", ".py", ".json"
]);
const excluded = new Set(["node_modules", ".git", "dist", "build", ".vite", "coverage"]);
const excludedFiles = new Set(["package-lock.json"]);
const limit = 1000;
let failed = false;

function isSource(path) {
  if (path.endsWith("CMakeLists.txt")) return true;
  return sourceExts.has(extname(path));
}

function scan(dir) {
  for (const name of readdirSync(dir)) {
    if (excluded.has(name)) continue;
    const path = join(dir, name);
    if (excludedFiles.has(name)) continue;
    const stat = statSync(path);
    if (stat.isDirectory()) scan(path);
    if (!stat.isFile() || !isSource(path)) continue;
    const text = readFileSync(path, "utf8");
    const lines = text.length === 0 ? 0 : text.split(/\r\n|\r|\n/).length;
    const status = lines > limit ? "ERROR" : lines >= 800 ? "WARN" : "OK";
    if (status !== "OK") console.log(`${status} ${path} ${lines}`);
    if (lines > limit) failed = true;
  }
}

scan(".");
if (failed) process.exit(1);
console.log("check:file-size ok");
