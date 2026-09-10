import { contextBridge, ipcRenderer } from "electron";

contextBridge.exposeInMainWorld("localMixer", {
  mode: "ui-preview",
  nativeEngine: "sound-pad-helper",
  playSoundPad: (padId: string) => ipcRenderer.invoke("sound-pad:play", padId),
  stopSoundPads: () => ipcRenderer.invoke("sound-pad:stop"),
  chooseMediaFile: () => ipcRenderer.invoke("media:choose-file"),
  engineCommand: (type: string, payload?: Record<string, unknown>) => ipcRenderer.invoke("engine:command", type, payload)
});
