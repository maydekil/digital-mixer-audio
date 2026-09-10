export type EngineState = "STOPPED" | "STARTING" | "RUNNING" | "RECONFIGURING" | "RECOVERING" | "ERROR";

export interface EngineMessage {
  id?: string;
  type?: string;
  ok?: boolean;
  error?: string;
  [key: string]: unknown;
}

export interface PendingCommand {
  resolve(message: EngineMessage): void;
  reject(error: Error): void;
  timeout: ReturnType<typeof setTimeout>;
}

export const engineProtocolVersion = 1;
export const maxEngineMessageBytes = 8192;
