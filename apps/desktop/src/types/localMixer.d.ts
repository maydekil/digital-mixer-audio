export {};

declare global {
  interface Window {
    localMixer?: {
      mode: string;
      nativeEngine: string;
      playSoundPad?: (padId: string) => Promise<{ ok: boolean; error?: string }>;
      stopSoundPads?: () => Promise<{ ok: boolean; stopped?: boolean; error?: string }>;
      chooseMediaFile?: () => Promise<{ ok: boolean; path?: string; canceled?: boolean; error?: string }>;
      chooseProjectOpenPath?: () => Promise<{ ok: boolean; path?: string; canceled?: boolean; error?: string }>;
      chooseProjectSavePath?: () => Promise<{ ok: boolean; path?: string; canceled?: boolean; error?: string }>;
      readProjectFile?: (path: string) => Promise<{ ok: boolean; path?: string; content?: string; error?: string }>;
      writeProjectFile?: (path: string, content: string) => Promise<{ ok: boolean; path?: string; error?: string }>;
      engineCommand?: (type: string, payload?: Record<string, unknown>) => Promise<Record<string, unknown>>;
    };
  }
}
