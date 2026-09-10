export {};

declare global {
  interface Window {
    localMixer?: {
      mode: string;
      nativeEngine: string;
      playSoundPad?: (padId: string) => Promise<{ ok: boolean; error?: string }>;
      stopSoundPads?: () => Promise<{ ok: boolean; stopped?: boolean; error?: string }>;
    };
  }
}
