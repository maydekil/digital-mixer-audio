import { readFileSync } from "node:fs";

const plan = JSON.parse(readFileSync("docs/task-plan.json", "utf8"));
const phases = plan.phases ?? [];
const ids = new Set();
const allowedStatuses = new Set([
  "NOT_STARTED",
  "IN_PROGRESS",
  "IMPLEMENTED_UNVERIFIED",
  "BLOCKED_ENVIRONMENT",
  "FAILED",
  "CAPABILITY_UNAVAILABLE",
  "UI_VERIFIED",
  "VERIFIED"
]);

for (const phase of phases) {
  if (!phase.id || ids.has(phase.id)) throw new Error(`Duplicate or missing phase id: ${phase.id}`);
  ids.add(phase.id);
  if (!allowedStatuses.has(phase.status)) throw new Error(`Invalid status for ${phase.id}: ${phase.status}`);
}

if (phases.length !== 49) throw new Error(`Expected 49 phases, found ${phases.length}`);

const visiting = new Set();
const visited = new Set();

function visit(id) {
  if (visited.has(id)) return;
  if (visiting.has(id)) throw new Error(`Dependency cycle at ${id}`);
  const phase = phases.find((item) => item.id === id);
  if (!phase) throw new Error(`Missing phase ${id}`);
  visiting.add(id);
  for (const dep of phase.dependsOn ?? []) {
    if (!ids.has(dep)) throw new Error(`${id} depends on missing ${dep}`);
    visit(dep);
  }
  visiting.delete(id);
  visited.add(id);
}

for (const phase of phases) visit(phase.id);

console.log(`check:plan ok (${phases.length} phases)`);
