import { EventEmitter } from "node:events";
import { spawn, type ChildProcessWithoutNullStreams } from "node:child_process";
import { engineProtocolVersion, maxEngineMessageBytes, type EngineMessage, type EngineState, type PendingCommand } from "./EngineProtocol.js";

export interface EngineSupervisorOptions {
  enginePath: string;
  args?: string[];
  startupTimeoutMs?: number;
  commandTimeoutMs?: number;
  maxRestartsPerMinute?: number;
}

export class EngineSupervisor extends EventEmitter {
  private child: ChildProcessWithoutNullStreams | null = null;
  private stateValue: EngineState = "STOPPED";
  private stdoutBuffer = "";
  private readonly pending = new Map<string, PendingCommand>();
  private nextId = 1;
  private restartTimes: number[] = [];
  private stderrTail: string[] = [];

  constructor(private readonly options: EngineSupervisorOptions) {
    super();
  }

  get state() {
    return this.stateValue;
  }

  get stderrLog() {
    return this.stderrTail.join("");
  }

  async start() {
    if (this.child) return;
    this.setState("STARTING");
    const args = this.options.args ?? ["--stdio"];
    const child = spawn(this.options.enginePath, args, { stdio: ["pipe", "pipe", "pipe"], shell: false });
    this.child = child;
    this.attachChild(child);

    await this.waitForHandshake(this.options.startupTimeoutMs ?? 2000);
  }

  send(type: string, payload: Record<string, unknown> = {}) {
    if (!this.child || this.stateValue !== "RUNNING") return Promise.reject(new Error("Engine is not running"));
    if (this.child.stdin.destroyed || !this.child.stdin.writable) {
      return Promise.reject(new Error("Engine stdin is closed"));
    }

    const id = `cmd-${this.nextId}`;
    this.nextId += 1;
    const message = `${JSON.stringify({ id, type, ...payload })}\n`;

    return new Promise<EngineMessage>((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.pending.delete(id);
        reject(new Error(`Engine command timed out: ${type}`));
      }, this.options.commandTimeoutMs ?? 1000);

      this.pending.set(id, { resolve, reject, timeout });
      try {
        const queued = this.child?.stdin.write(message);
        if (queued === false && this.child?.stdin.destroyed) {
          clearTimeout(timeout);
          this.pending.delete(id);
          reject(new Error("Engine stdin closed while sending command"));
        }
      } catch (error) {
        clearTimeout(timeout);
        this.pending.delete(id);
        reject(error instanceof Error ? error : new Error(String(error)));
      }
    });
  }

  async stop() {
    const child = this.child;
    if (!child) return;
    try {
      if (this.stateValue === "RUNNING") await this.send("shutdown");
    } catch {
      child.kill("SIGTERM");
    }
  }

  private attachChild(child: ChildProcessWithoutNullStreams) {
    child.stdout.on("data", (chunk: Buffer) => this.readStdout(chunk));
    child.stderr.on("data", (chunk: Buffer) => this.readStderr(chunk));
    child.stdin.on("error", (error) => {
      this.failAll(error);
      if (this.stateValue !== "STOPPED") this.setState("ERROR");
    });
    child.on("error", (error) => this.failAll(error));
    child.on("close", (code, signal) => {
      if (this.child === child) this.child = null;
      const clean = (code === 0 || signal === "SIGTERM") && this.stateValue === "RUNNING";
      this.failAll(new Error(`Engine exited unexpectedly: ${code ?? signal ?? "unknown"}`));
      if (this.stateValue !== "ERROR") this.setState(clean ? "STOPPED" : "ERROR");
      this.emit("engine-close", code, signal);
    });
  }

  private waitForHandshake(timeoutMs: number) {
    return new Promise<void>((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.off("hello", onHello);
        this.off("engine-close", onClose);
        this.off("startup-error", onStartupError);
        reject(new Error("Engine handshake timed out"));
      }, timeoutMs);

      const onHello = (message: EngineMessage) => {
        if (message.protocol !== engineProtocolVersion) {
          clearTimeout(timeout);
          this.off("engine-close", onClose);
          this.off("startup-error", onStartupError);
          reject(new Error("Engine protocol mismatch"));
          return;
        }
        clearTimeout(timeout);
        this.off("hello", onHello);
        this.off("engine-close", onClose);
        this.off("startup-error", onStartupError);
        this.setState("RUNNING");
        resolve();
      };

      const onClose = () => {
        clearTimeout(timeout);
        this.off("hello", onHello);
        this.off("startup-error", onStartupError);
        reject(new Error("Engine exited before handshake"));
      };

      const onStartupError = (error: Error) => {
        clearTimeout(timeout);
        this.off("hello", onHello);
        this.off("engine-close", onClose);
        reject(error);
      };

      this.on("hello", onHello);
      this.once("engine-close", onClose);
      this.once("startup-error", onStartupError);
    });
  }

  private readStdout(chunk: Buffer) {
    this.stdoutBuffer += chunk.toString("utf8");
    if (Buffer.byteLength(this.stdoutBuffer, "utf8") > maxEngineMessageBytes) {
      this.rejectOversizedMessage();
      return;
    }

    for (;;) {
      const newline = this.stdoutBuffer.indexOf("\n");
      if (newline < 0) return;
      const line = this.stdoutBuffer.slice(0, newline);
      this.stdoutBuffer = this.stdoutBuffer.slice(newline + 1);
      this.handleLine(line);
    }
  }

  private handleLine(line: string) {
    if (Buffer.byteLength(line, "utf8") > maxEngineMessageBytes) {
      this.rejectOversizedMessage();
      return;
    }

    let message: EngineMessage;
    try {
      message = JSON.parse(line) as EngineMessage;
    } catch {
      const error = new Error("Malformed engine message");
      this.failAll(error);
      this.setState("ERROR");
      this.emit("startup-error", error);
      return;
    }

    if (message.type === "hello") {
      this.emit("hello", message);
      return;
    }

    if (message.id && this.pending.has(message.id)) {
      const pending = this.pending.get(message.id);
      if (!pending) return;
      clearTimeout(pending.timeout);
      this.pending.delete(message.id);
      if (message.ok === false) pending.reject(new Error(String(message.error ?? "Engine command failed")));
      else pending.resolve(message);
    }
  }

  private readStderr(chunk: Buffer) {
    this.stderrTail.push(chunk.toString("utf8"));
    while (this.stderrTail.join("").length > 4096) this.stderrTail.shift();
  }

  private rejectOversizedMessage() {
    this.failAll(new Error("Oversized engine message"));
    this.child?.kill("SIGTERM");
    this.setState("ERROR");
  }

  private failAll(error: Error) {
    for (const pending of this.pending.values()) {
      clearTimeout(pending.timeout);
      pending.reject(error);
    }
    this.pending.clear();
  }

  private setState(state: EngineState) {
    this.stateValue = state;
    this.emit("state", state);
  }

  canRestart() {
    const now = Date.now();
    this.restartTimes = this.restartTimes.filter((time) => now - time < 60_000);
    const limit = this.options.maxRestartsPerMinute ?? 3;
    if (this.restartTimes.length >= limit) return false;
    this.restartTimes.push(now);
    return true;
  }
}
