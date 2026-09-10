import { describe, expect, it } from "vitest";
import { join } from "node:path";
import { EngineSupervisor } from "../../apps/desktop/electron/EngineSupervisor";

const fakeEngine = join(process.cwd(), "tests", "electron", "fake-engine.mjs");

describe("EngineSupervisor", () => {
  it("handshakes and sends bounded JSONL commands", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine],
      startupTimeoutMs: 500,
      commandTimeoutMs: 500
    });

    await supervisor.start();
    expect(supervisor.state).toBe("RUNNING");
    await expect(supervisor.send("ping")).resolves.toMatchObject({ type: "pong", ok: true });
    await supervisor.stop();
  });

  it("rejects startup on malformed engine output", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine, "malformed"],
      startupTimeoutMs: 500
    });

    await expect(supervisor.start()).rejects.toThrow(/Malformed|timed out/);
    expect(supervisor.state).toBe("ERROR");
  });

  it("rejects timed-out commands without hanging", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine],
      startupTimeoutMs: 500,
      commandTimeoutMs: 50
    });

    await supervisor.start();
    await expect(supervisor.send("never")).rejects.toThrow(/timed out/);
    await supervisor.stop();
  });

  it("rejects commands after engine stdin closes without uncaught EPIPE", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine, "exit-after-hello"],
      startupTimeoutMs: 500,
      commandTimeoutMs: 100
    });

    await supervisor.start();
    await new Promise((resolve) => setTimeout(resolve, 50));
    await expect(supervisor.send("ping")).rejects.toThrow(/not running|stdin|exited/i);
  });

  it("reports unexpected EOF before handshake", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine, "silent"],
      startupTimeoutMs: 500
    });

    await expect(supervisor.start()).rejects.toThrow(/before handshake/);
    expect(supervisor.state).toBe("ERROR");
  });

  it("rejects oversized startup messages", async () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine, "oversized"],
      startupTimeoutMs: 500
    });

    await expect(supervisor.start()).rejects.toThrow(/Oversized|before handshake/);
    expect(supervisor.state).toBe("ERROR");
  });

  it("limits restart attempts", () => {
    const supervisor = new EngineSupervisor({
      enginePath: process.execPath,
      args: [fakeEngine],
      maxRestartsPerMinute: 3
    });

    expect(supervisor.canRestart()).toBe(true);
    expect(supervisor.canRestart()).toBe(true);
    expect(supervisor.canRestart()).toBe(true);
    expect(supervisor.canRestart()).toBe(false);
  });
});
