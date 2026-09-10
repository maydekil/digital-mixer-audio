import { contextBridge, ipcRenderer } from "electron";

contextBridge.exposeInMainWorld("localMixer", {
  mode: "ui-preview",
  nativeEngine: "sound-pad-helper",
  playSoundPad: (padId: string) => ipcRenderer.invoke("sound-pad:play", padId),
  stopSoundPads: () => ipcRenderer.invoke("sound-pad:stop"),
  chooseMediaFile: () => ipcRenderer.invoke("media:choose-file"),
  chooseProjectOpenPath: () => ipcRenderer.invoke("project:choose-open"),
  chooseProjectSavePath: () => ipcRenderer.invoke("project:choose-save"),
  readProjectFile: (path: string) => ipcRenderer.invoke("project:read", path),
  writeProjectFile: (path: string, content: string) => ipcRenderer.invoke("project:write", { path, content }),
  inspectProjectMedia: (content: string) => ipcRenderer.invoke("project:inspect-media", content),
  collectProjectMedia: (path: string, content: string) => ipcRenderer.invoke("project:collect-media", { path, content }),
  relinkProjectMedia: (content: string, mediaId: string, path: string) => ipcRenderer.invoke("project:relink-media", { content, mediaId, path }),
  chooseExportOutputPath: () => ipcRenderer.invoke("export:choose-output"),
  chooseRecordingDirectory: () => ipcRenderer.invoke("recording:choose-directory"),
  engineCommand: (type: string, payload?: Record<string, unknown>) => ipcRenderer.invoke("engine:command", type, payload)
});
