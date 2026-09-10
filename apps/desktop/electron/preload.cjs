const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("localMixer", {
  mode: "ui-preview",
  nativeEngine: "sound-pad-helper",
  playSoundPad: (padId) => ipcRenderer.invoke("sound-pad:play", padId),
  stopSoundPads: () => ipcRenderer.invoke("sound-pad:stop")
});
