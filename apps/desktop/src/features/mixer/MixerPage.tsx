import { useEffect, useMemo, useRef, useState } from "react";
import { PreviewAdapter } from "../../adapters/preview/PreviewAdapter";
import { snapshotToExportRequest } from "../export/exportDocument";
import { HarmonyQuickPanel } from "../harmony/components/HarmonyQuickPanel";
import { HardwareMonitorPanel } from "../hardware/components/HardwareMonitorPanel";
import { MediaImportPanel } from "../media/components/MediaImportPanel";
import { ChannelProcessingPanel } from "../processing/components/ChannelProcessingPanel";
import { SoundPadPanel } from "../sound-pads/components/SoundPadPanel";
import { VocalFxPanel } from "../vocal-fx/components/VocalFxPanel";
import { plannedTakeFromResponse, replayChannelFromTake, snapshotToRecordingPlan } from "../recording/recordingDocument";
import { ChannelBank } from "./components/ChannelBank";
import type { ChannelState, FxProgram, MixerSnapshot } from "../../adapters/MixerControlPort";
import type { MeterLevel } from "../../components/audio/types";
import type { ProjectMediaStatus } from "../project/projectMediaWorkflow";
import { relinkMissingProjectMedia } from "../project/projectMediaWorkflow";
import { projectSessionToSnapshot, serializeProjectSession } from "../project/sessionDocument";

interface HardwareDevice {
  uid: string;
  name: string;
  defaultInput?: boolean;
  defaultOutput?: boolean;
  inputChannels?: number;
  outputChannels?: number;
}

const monitorSafetyGainDb = 0;

export function MixerPage() {
  const adapter = useMemo(() => new PreviewAdapter(), []);
  const [snapshot, setSnapshot] = useState(adapter.getSnapshot());
  const [hardwareOpen, setHardwareOpen] = useState(false);
  const [mediaImportOpen, setMediaImportOpen] = useState(false);
  const [vocalFxOpen, setVocalFxOpen] = useState(false);
  const [harmonyOpen, setHarmonyOpen] = useState(false);
  const [devices, setDevices] = useState<HardwareDevice[]>([]);
  const [outputUid, setOutputUid] = useState("");
  const [transportState, setTransportState] = useState("stopped");
  const [projectPath, setProjectPath] = useState("");
  const [systemAudioEnabled, setSystemAudioEnabled] = useState(false);
  const [systemMeter, setSystemMeter] = useState<MeterLevel>({ left: -60, right: -60, clip: false });
  const autosaveReady = useRef(false);
  const liveMonitorRefreshTimer = useRef<number | undefined>(undefined);

  function refresh(action: () => void) {
    action();
    const nextSnapshot = structuredClone(adapter.getSnapshot());
    setSnapshot(nextSnapshot);
    return nextSnapshot;
  }

  async function refreshDevices() {
    const result = await window.localMixer?.engineCommand?.("list-devices");
    const nextDevices = Array.isArray(result?.devices) ? result.devices as HardwareDevice[] : [];
    setDevices(nextDevices);
    const defaultOutput = nextDevices.find((device) => device.defaultOutput && (device.outputChannels ?? 0) > 0) ?? nextDevices.find((device) => (device.outputChannels ?? 0) > 0);
    setOutputUid((current) => current || defaultOutput?.uid || "");
  }

  async function refreshFxProgramBank() {
    if (!window.localMixer?.engineCommand) return;
    const result = await window.localMixer.engineCommand("fx-program-bank");
    if (!Array.isArray(result?.programs)) return;
    const programs = parseFxPrograms(result.programs);
    if (programs.length === 99) refresh(() => adapter.setPrograms(programs));
  }

  async function restoreAutosaveIfAvailable() {
    if (!window.localMixer?.readProjectAutosave) return;
    const autosave = await window.localMixer.readProjectAutosave();
    if (!autosave.ok || autosave.canceled || !autosave.content) return;
    if (!window.confirm("Restore autosaved project?")) return;
    refresh(() => adapter.replaceSnapshot(projectSessionToSnapshot(autosave.content ?? "", adapter.getSnapshot())));
    await window.localMixer.clearProjectAutosave?.();
  }

  async function refreshActiveMonitor(nextSnapshot: MixerSnapshot, nextOutputUid = outputUid) {
    if (liveMonitorRefreshTimer.current !== undefined) {
      window.clearTimeout(liveMonitorRefreshTimer.current);
      liveMonitorRefreshTimer.current = undefined;
    }
    if (!window.localMixer?.engineCommand) return;
    const activeSource = autoMonitorChannel(nextSnapshot);
    await syncMixerGraph(nextSnapshot, nextOutputUid);
    if (!activeSource || !isMasterAudible(nextSnapshot)) {
      await window.localMixer.engineCommand("stop-mixer-monitor");
      return;
    }
    await window.localMixer.engineCommand("start-mixer-monitor", {
      sampleRate: 48000,
      monitorGainDb: monitorSafetyGainDb
    });
  }

  function scheduleActiveMonitorRefresh(nextSnapshot: MixerSnapshot) {
    if (liveMonitorRefreshTimer.current !== undefined) window.clearTimeout(liveMonitorRefreshTimer.current);
    liveMonitorRefreshTimer.current = window.setTimeout(() => {
      liveMonitorRefreshTimer.current = undefined;
      void refreshActiveMonitor(nextSnapshot);
    }, 220);
  }

  function refreshLiveProcessing(action: () => void) {
    const nextSnapshot = refresh(action);
    scheduleActiveMonitorRefresh(nextSnapshot);
  }

  function changeOutput(uid: string) {
    setOutputUid(uid);
    void refreshActiveMonitor(adapter.getSnapshot(), uid);
  }

  async function setSystemChannelEnabled(enabled: boolean) {
    const engineCommand = window.localMixer?.engineCommand;
    if (!enabled) {
      if (engineCommand && systemAudioEnabled) {
        await engineCommand("stop-mixer-monitor");
        const result = await engineCommand("routing-system-disable");
        if (result?.ok === false) window.alert(String(result.error ?? "Route restore failed"));
      }
      refresh(() => {
        adapter.setChannelEnabled("system", false);
        adapter.setChannelMonitor("system", false);
      });
      setSystemAudioEnabled(false);
      return;
    }

    if (!engineCommand) {
      const nextSnapshot = refresh(() => adapter.setChannelEnabled("system", true));
      void refreshActiveMonitor(nextSnapshot);
      return;
    }

    const blackHole = devices.find((device) => isBlackHoleDevice(device) && (device.inputChannels ?? 0) >= 2);
    const physicalOutput = physicalOutputDevice(devices, outputUid);
    if (!blackHole || !physicalOutput) {
      window.alert(!blackHole ? "BlackHole 2ch belum terdeteksi." : "Pilih output fisik, bukan Default/BlackHole.");
      return;
    }
    if (outputUid !== physicalOutput.uid) setOutputUid(physicalOutput.uid);

    const route = await engineCommand("routing-system-diagnostics", {
      blackHoleUid: blackHole.uid,
      physicalOutputUid: physicalOutput.uid,
      sampleRate: 48000,
      blackHoleInputStartChannel: 0,
      physicalOutputStartChannel: 0
    });
    if (route.routeValid !== true) {
      window.alert(`Route belum siap: ${String(route.error ?? "Route rejected")}`);
      return;
    }

    if (!window.confirm("Switch macOS output to BlackHole and monitor SYSTEM through the mixer?")) return;
    const routeEnabled = await engineCommand("routing-system-enable", {
      blackHoleUid: blackHole.uid,
      physicalOutputUid: physicalOutput.uid,
      sampleRate: 48000,
      blackHoleInputStartChannel: 0,
      physicalOutputStartChannel: 0,
      allowOsRouteChange: true
    });
    if (routeEnabled.ok === false) {
      window.alert(String(routeEnabled.error ?? "Route failed"));
      return;
    }

    refresh(() => {
      adapter.setChannelEnabled("system", true);
      adapter.setChannelSource("system", blackHole.uid);
      adapter.setChannelMonitor("system", true);
      adapter.setChannelSend("system", "fx-a", -60);
      adapter.setChannelSend("system", "fx-b", -60);
      adapter.selectChannel("system");
    });
    await syncMixerGraph(adapter.getSnapshot(), physicalOutput.uid);
    await engineCommand("start-mixer-monitor", { sampleRate: 48000, monitorGainDb: monitorSafetyGainDb });
    setSystemAudioEnabled(true);
  }

  async function sendTransport(type: "transport-play" | "transport-pause" | "transport-stop") {
    if (!window.localMixer?.engineCommand) return;
    const result = await window.localMixer.engineCommand(type);
    if (typeof result?.state === "string") setTransportState(result.state);
  }

  async function saveProject() {
    if (!window.localMixer?.chooseProjectSavePath || !window.localMixer?.writeProjectFile) return;
    const target = await window.localMixer.chooseProjectSavePath();
    if (!target.ok || target.canceled || !target.path) return;
    const written = await window.localMixer.writeProjectFile(target.path, serializeProjectSession(adapter.getSnapshot()));
    if (written.ok && written.path) setProjectPath(written.path);
  }

  async function openProject() {
    if (!window.localMixer?.chooseProjectOpenPath || !window.localMixer?.readProjectFile) return;
    const target = await window.localMixer.chooseProjectOpenPath();
    if (!target.ok || target.canceled || !target.path) return;
    const loaded = await window.localMixer.readProjectFile(target.path);
    if (!loaded.ok || !loaded.content) return;
    const resolved = await resolveProjectMedia(loaded.content, target.path);
    refresh(() => adapter.replaceSnapshot(projectSessionToSnapshot(resolved, adapter.getSnapshot())));
    setProjectPath(loaded.path ?? target.path);
  }

  async function collectProject() {
    if (!window.localMixer?.collectProjectMedia || !window.localMixer?.writeProjectFile) return;
    let targetPath = projectPath;
    if (!targetPath) {
      if (!window.localMixer.chooseProjectSavePath) return;
      const target = await window.localMixer.chooseProjectSavePath();
      if (!target.ok || target.canceled || !target.path) return;
      targetPath = target.path;
    }
    const collected = await window.localMixer.collectProjectMedia(targetPath, serializeProjectSession(adapter.getSnapshot()));
    if (!collected.ok || !collected.content) return;
    const resolved = await resolveProjectMedia(collected.content, targetPath);
    const written = await window.localMixer.writeProjectFile(targetPath, resolved);
    if (!written.ok) return;
    refresh(() => adapter.replaceSnapshot(projectSessionToSnapshot(resolved, adapter.getSnapshot())));
    setProjectPath(written.path ?? targetPath);
  }

  async function resolveProjectMedia(content: string, path: string) {
    if (!window.localMixer?.inspectProjectMedia || !window.localMixer?.relinkProjectMedia || !window.localMixer?.chooseMediaFile) {
      return content;
    }
    const result = await relinkMissingProjectMedia(content, {
      inspectProjectMedia: window.localMixer.inspectProjectMedia,
      relinkProjectMedia: window.localMixer.relinkProjectMedia,
      chooseReplacement: async (media: ProjectMediaStatus) => {
        if (!window.confirm(`Relink missing media ${media.id}?`)) return { ok: true, canceled: true };
        return window.localMixer?.chooseMediaFile?.() ?? { ok: false, error: "Media picker unavailable" };
      }
    });
    if (result.ok && result.relinked > 0 && window.localMixer?.writeProjectFile) {
      await window.localMixer.writeProjectFile(path, result.content);
    }
    return result.ok ? result.content : content;
  }

  async function exportProject() {
    if (!window.localMixer?.chooseExportOutputPath || !window.localMixer?.engineCommand) return;
    const target = await window.localMixer.chooseExportOutputPath();
    if (!target.ok || target.canceled || !target.path) return;
    const request = snapshotToExportRequest(adapter.getSnapshot(), target.path);
    const plan = await window.localMixer.engineCommand("export-plan", request as unknown as Record<string, unknown>);
    if (plan.exportable !== true) return;
    await window.localMixer.engineCommand("export-render", request as unknown as Record<string, unknown>);
  }

  function addChannel() {
    const role = parseSourceRole(window.prompt("Channel role", "music") ?? "music");
    const name = window.prompt("Channel name", role.toUpperCase()) ?? "";
    const source = window.prompt("Source label or path", "") ?? "";
    refresh(() => adapter.addSourceChannel(role, name, source));
  }

  function renameSelectedChannel() {
    const channel = adapter.getSnapshot().channels.find((item) => item.id === adapter.getSnapshot().selectedChannelId);
    if (!channel || channel.kind === "master") return;
    const name = window.prompt("Channel name", channel.name) ?? "";
    refresh(() => adapter.renameChannel(channel.id, name));
  }

  function removeSelectedChannel() {
    const channel = adapter.getSnapshot().channels.find((item) => item.id === adapter.getSnapshot().selectedChannelId);
    if (!channel || channel.kind === "master") return;
    if (!window.confirm(`Remove channel ${channel.name}?`)) return;
    refresh(() => adapter.removeChannel(channel.id));
  }

  async function planRecording() {
    if (!window.localMixer?.chooseRecordingDirectory || !window.localMixer?.engineCommand) return;
    if (adapter.getSnapshot().recording.status === "recording") {
      const result = await window.localMixer.engineCommand("recording-stop");
      const takes = plannedTakeFromResponse(result);
      refresh(() => {
        if (takes.length > 0) {
          adapter.addRecordedTakes(takes);
          for (const take of takes) {
            const replay = replayChannelFromTake(take);
            if (replay) adapter.addSourceChannel(replay.role, replay.name, replay.source);
          }
          adapter.setRecordingStatus("saved");
        } else {
          adapter.setRecordingStatus("failed", typeof result.error === "string" ? result.error : "RECORDING_STOP_FAILED");
        }
      });
      return;
    }
    const target = await window.localMixer.chooseRecordingDirectory();
    if (!target.ok || target.canceled || !target.path) return;
    const request = snapshotToRecordingPlan(adapter.getSnapshot(), target.path);
    refresh(() => adapter.setRecordingStatus("planned"));
    const result = await window.localMixer.engineCommand("recording-start", request as unknown as Record<string, unknown>);
    const takes = plannedTakeFromResponse(result);
    refresh(() => {
      if (takes.length > 0) {
        adapter.addRecordedTakes(takes, request.armedChannelIds, target.path);
        adapter.setRecordingStatus("recording");
      }
      else adapter.setRecordingStatus("failed", typeof result.error === "string" ? result.error : "RECORDING_PLAN_FAILED");
    });
  }

  async function setHarmonyEnabled(enabled: boolean) {
    const current = adapter.getSnapshot().harmony;
    const channel = adapter.getSnapshot().channels.find((item) => item.role === "vocal");
    if (!window.localMixer?.engineCommand || !channel) {
      refreshLiveProcessing(() => adapter.setHarmonyEnabled(enabled));
      return;
    }
    refresh(() => adapter.setHarmonyPending(true));
    const result = await window.localMixer.engineCommand("channel-harmony-set-enabled", {
      channelId: channel.id,
      enabled,
      expectedRevision: current.revision
    });
    applyHarmonyResult(result);
  }

  async function configureHarmony(field: keyof MixerSnapshot["harmony"], value: string | number | boolean) {
    const before = adapter.getSnapshot().harmony;
    refresh(() => adapter.updateHarmony(field, value));
    const current = adapter.getSnapshot().harmony;
    const channel = adapter.getSnapshot().channels.find((item) => item.role === "vocal");
    if (!window.localMixer?.engineCommand || !channel) return;
    const result = await window.localMixer.engineCommand("channel-harmony-configure", {
      channelId: channel.id,
      key: current.key,
      scale: current.scale,
      mode: current.mode,
      voice1: current.voice1,
      voice2: current.voice2,
      levelDb: current.levelDb,
      expectedRevision: before.revision
    });
    applyHarmonyResult(result);
  }

  function applyHarmonyResult(result: Record<string, unknown> | undefined) {
    if (!result || result.ok === false || result.accepted === false) {
      refresh(() => adapter.setHarmonyError(String(result?.error ?? "HARMONY_REJECTED")));
      return;
    }
    refreshLiveProcessing(() => adapter.ackHarmony({
      enabled: Boolean(result.desiredEnabled),
      effectiveEnabled: Boolean(result.effectiveEnabled),
      revision: Number(result.revision ?? 0),
      primaryInstanceId: String(result.primaryHarmonyInstanceId ?? ""),
      key: String(result.key ?? "C"),
      scale: String(result.scale ?? "Major"),
      mode: String(result.mode ?? "Diatonic"),
      voice1: String(result.voice1 ?? "+3rd"),
      voice2: String(result.voice2 ?? "+5th"),
      levelDb: Number(result.levelDb ?? 0)
    }));
  }

  useEffect(() => {
    void restoreAutosaveIfAvailable();
    void refreshDevices();
    void refreshFxProgramBank();
  }, []);

  useEffect(() => () => {
    if (liveMonitorRefreshTimer.current !== undefined) window.clearTimeout(liveMonitorRefreshTimer.current);
  }, []);

  useEffect(() => {
    if (!autosaveReady.current) {
      autosaveReady.current = true;
      return;
    }
    const handle = window.setTimeout(() => {
      void window.localMixer?.writeProjectAutosave?.(serializeProjectSession(snapshot));
    }, 5000);
    return () => window.clearTimeout(handle);
  }, [snapshot]);

  useEffect(() => {
    void syncMixerGraph(snapshot, outputUid);
  }, [snapshot, outputUid]);

  const systemMonitorActive = systemAudioEnabled || snapshot.channels.some((channel) => (
    channel.id === "system" && channel.enabled && !channel.mute && Boolean(channel.monitor)
  ));

  useEffect(() => {
    if (!systemMonitorActive || !window.localMixer?.engineCommand) {
      setSystemMeter({ left: -60, right: -60, clip: false });
      return;
    }
    let canceled = false;
    async function refreshSystemMeter() {
      const result = await window.localMixer?.engineCommand?.("mixer-monitor-status");
      if (canceled) return;
      const peak = Number(result?.inputPeak ?? 0);
      const leftPeak = Number(result?.inputPeakLeft ?? peak);
      const rightPeak = Number(result?.inputPeakRight ?? peak);
      setSystemMeter({
        left: linearPeakToDb(leftPeak),
        right: linearPeakToDb(rightPeak),
        clip: Math.max(leftPeak, rightPeak) >= 0.98
      });
    }
    void refreshSystemMeter();
    const interval = window.setInterval(() => void refreshSystemMeter(), 120);
    return () => {
      canceled = true;
      window.clearInterval(interval);
    };
  }, [systemMonitorActive]);

  const selected = snapshot.channels.find((channel) => channel.id === snapshot.selectedChannelId) ?? snapshot.channels[0];
  const masterChannel = snapshot.channels.find((channel) => channel.kind === "master") ?? selected;
  useEffect(() => {
    if (selected.role === "system" && vocalFxOpen) setVocalFxOpen(false);
  }, [selected.role, vocalFxOpen]);

  const inputOptions = devices
    .filter((device) => (device.inputChannels ?? 0) > 0)
    .map((device) => ({ value: device.uid, label: device.name || device.uid }));
  const outputOptions = devices
    .filter((device) => (device.outputChannels ?? 0) > 0)
    .map((device) => ({ value: device.uid, label: device.name || device.uid }));
  const sourceOptions = Object.fromEntries(snapshot.channels.map((channel) => {
    if (!["system", "vocal", "instrument"].includes(channel.role)) return [channel.id, []];
    const options = inputOptions.some((option) => option.value === channel.source)
      ? inputOptions
      : [{ value: channel.source, label: channel.source }, ...inputOptions];
    return [channel.id, options];
  }));
  const systemOutputMeter = systemMonitorActive ? systemOutputMeterFromInput(snapshot, systemMeter) : systemMeter;
  const masterMeter = systemMonitorActive ? masterMeterFromSystem(snapshot, systemOutputMeter) : undefined;
  const displayChannels = snapshot.channels.map((channel) => {
    if (channel.id === "system") return { ...channel, meter: systemOutputMeter };
    if (channel.kind === "master" && masterMeter) return { ...channel, meter: masterMeter };
    return channel;
  });

  return (
    <main className="mixer-app">
      <TopBar
        projectName={snapshot.projectName}
        time={snapshot.transportTime}
        rate={snapshot.sampleRateLabel}
        transportState={transportState}
        onVocalFx={() => {
          if (selected.role !== "system") setVocalFxOpen(true);
        }}
        onTimeline={() => setMediaImportOpen(true)}
        onPlay={() => void sendTransport(transportState === "playing" ? "transport-pause" : "transport-play")}
        onStop={() => void sendTransport("transport-stop")}
        onRecord={() => void planRecording()}
        onOpen={() => void openProject()}
        onSave={() => void saveProject()}
        onCollect={() => void collectProject()}
        onExport={() => void exportProject()}
        vocalFxEnabled={selected.role !== "system"}
        onAddChannel={addChannel}
        onRenameChannel={renameSelectedChannel}
        onRemoveChannel={removeSelectedChannel}
      />
      <section className="workspace">
        <div className="left-zone">
          <ChannelBank
            channels={displayChannels}
            sourceOptions={sourceOptions}
            vocalFxPresetId={snapshot.vocalFx.activePresetId}
            vocalFxPresets={snapshot.vocalFx.presets}
            vocalFxSlots={snapshot.vocalFx.slots}
            onSelect={(id) => refresh(() => adapter.selectChannel(id))}
            onEnabled={(id, enabled) => {
              if (id === "system") {
                void setSystemChannelEnabled(enabled);
                return;
              }
              const nextSnapshot = refresh(() => adapter.setChannelEnabled(id, enabled));
              void refreshActiveMonitor(nextSnapshot);
            }}
            onSource={(id, source) => {
              const nextSnapshot = refresh(() => adapter.setChannelSource(id, source));
              void refreshActiveMonitor(nextSnapshot);
            }}
            onTrim={(id, value) => refreshLiveProcessing(() => adapter.setChannelTrim(id, value))}
            onPan={(id, value) => refreshLiveProcessing(() => adapter.setChannelPan(id, value))}
            onFader={(id, value) => refreshLiveProcessing(() => adapter.setChannelFader(id, value))}
            onEqBand={(id, bandId, gainDb) => refreshLiveProcessing(() => adapter.setChannelEqBand(id, bandId, "gainDb", gainDb))}
            onNoiseAmount={(id, amount) => refreshLiveProcessing(() => adapter.setChannelNoiseAmount(id, amount))}
            onCompressorParam={(id, field, value) => refreshLiveProcessing(() => adapter.setChannelCompressorParam(id, field, value))}
            onCompressorEnabled={(id, enabled) => refreshLiveProcessing(() => adapter.setChannelProcessor(id, "comp", enabled))}
            onMute={(id, muted) => {
              const nextSnapshot = refresh(() => adapter.setChannelMute(id, muted));
              void refreshActiveMonitor(nextSnapshot);
            }}
            onRecordArm={(id, armed) => refresh(() => adapter.setChannelRecordArm(id, armed))}
            onVocalFxPreset={(id, presetId) => {
              refreshLiveProcessing(() => {
                adapter.selectChannel(id);
                adapter.applyVocalFxPreset(presetId);
                const active = adapter.getSnapshot().vocalFx.slots.some((slot) => slot.enabled);
                adapter.setChannelProcessor(id, "insertFx", active);
              });
            }}
            onVocalFxMix={(id, slotId, amount) => {
              refreshLiveProcessing(() => {
                adapter.selectChannel(id);
                adapter.setVocalFxSlotMix(slotId, amount / 100);
                adapter.setVocalFxSlotEnabled(slotId, amount > 0);
                const active = adapter.getSnapshot().vocalFx.slots.some((slot) => slot.enabled);
                adapter.setChannelProcessor(id, "insertFx", active);
              });
            }}
            onClipReset={(id) => refresh(() => adapter.resetClip(id))}
            onHarmonyToggle={() => void setHarmonyEnabled(!adapter.getSnapshot().harmony.enabled)}
            onHarmonySettings={(id) => {
              refresh(() => adapter.selectChannel(id));
              setHarmonyOpen(true);
            }}
          />
        </div>
        <div className="right-zone">
          <ChannelProcessingPanel
            channel={masterChannel}
            eqBands={masterChannel.eqBands}
            onEqChange={(bandId, field, value) => refreshLiveProcessing(() => adapter.setChannelEqBand(masterChannel.id, bandId, field, value))}
            onEqReset={() => refreshLiveProcessing(() => adapter.resetChannelEqBands(masterChannel.id))}
            onProcessor={(processorId, enabled) => refreshLiveProcessing(() => adapter.setChannelProcessor(masterChannel.id, processorId, enabled))}
            onNoiseChange={(field, value) => refreshLiveProcessing(() => adapter.setChannelNoiseParam(masterChannel.id, field, value))}
            onCompressorChange={(field, value) => refreshLiveProcessing(() => adapter.setChannelCompressorParam(masterChannel.id, field, value))}
            onDeEsserChange={(field, value) => refreshLiveProcessing(() => adapter.setChannelDeEsserParam(masterChannel.id, field, value))}
          />
          <SoundPadPanel />
        </div>
      </section>
      <Footer outputUid={outputUid} outputOptions={outputOptions} onOutput={changeOutput} onHardware={() => setHardwareOpen(true)} />
      {harmonyOpen ? (
        <div className="modal-backdrop">
          <HarmonyQuickPanel
            harmony={snapshot.harmony}
            onToggle={(enabled) => void setHarmonyEnabled(enabled)}
            onChange={(field, value) => void configureHarmony(field, value)}
            onClose={() => setHarmonyOpen(false)}
          />
        </div>
      ) : null}
      <HardwareMonitorPanel open={hardwareOpen} onClose={() => setHardwareOpen(false)} />
      <MediaImportPanel open={mediaImportOpen} onClose={() => setMediaImportOpen(false)} />
      <VocalFxPanel
        open={vocalFxOpen}
        vocalFx={snapshot.vocalFx}
        onClose={() => setVocalFxOpen(false)}
        onSelect={(slotId) => refresh(() => adapter.selectVocalFxSlot(slotId))}
        onToggle={(slotId, enabled) => refreshLiveProcessing(() => {
          adapter.setVocalFxSlotEnabled(slotId, enabled);
          const active = adapter.getSnapshot().vocalFx.slots.some((slot) => slot.enabled);
          if (selected.role === "vocal") adapter.setChannelProcessor(selected.id, "insertFx", active);
        })}
        onPreset={(presetId) => refreshLiveProcessing(() => {
          adapter.applyVocalFxPreset(presetId);
          const active = adapter.getSnapshot().vocalFx.slots.some((slot) => slot.enabled);
          if (selected.role === "vocal") adapter.setChannelProcessor(selected.id, "insertFx", active);
        })}
      />
    </main>
  );
}

async function syncMixerGraph(snapshot: MixerSnapshot, outputUid: string) {
  if (!window.localMixer?.engineCommand) return;
  const channels = snapshot.channels.slice(0, 32);
  const autoMonitorId = autoMonitorChannel(snapshot)?.id ?? "";
  const fxA = snapshot.fxUnits.find((unit) => unit.id === "fx-a");
  const fxB = snapshot.fxUnits.find((unit) => unit.id === "fx-b");
  const payload: Record<string, string | number | boolean> = {
    channelCount: channels.length,
    outputUid,
    monitorGainDb: monitorSafetyGainDb,
    fxAEnabled: false,
    fxAProgramId: fxA?.programId ?? 12,
    fxAReturnDb: -60,
    fxBEnabled: false,
    fxBProgramId: fxB?.programId ?? 50,
    fxBReturnDb: -60,
    vocalFxSlotCount: snapshot.vocalFx.slots.length
  };
  snapshot.vocalFx.slots.forEach((slot, index) => {
    const prefix = `vocalFxSlot${index}`;
    payload[`${prefix}Id`] = slot.id;
    payload[`${prefix}Type`] = liveVocalFxType(slot.id, slot.effectType, slot.enabled);
    payload[`${prefix}Enabled`] = slot.enabled;
    payload[`${prefix}Mix`] = slot.mix;
    if (slot.id === "harmony") {
      payload[`${prefix}Param3`] = harmonyIntervalSemitones(snapshot.harmony.voice1);
      payload[`${prefix}Param4`] = harmonyIntervalSemitones(snapshot.harmony.voice2);
      payload[`${prefix}Param5`] = snapshot.harmony.levelDb;
    }
  });
  channels.forEach((channel, index) => {
    const prefix = `channel${index}`;
    const fxSendsEnabled = false;
    payload[`${prefix}Id`] = channel.id;
    payload[`${prefix}Kind`] = channel.kind;
    payload[`${prefix}Name`] = channel.name;
    payload[`${prefix}Color`] = channelColor(channel);
    payload[`${prefix}SourceUid`] = channel.kind === "source" ? channel.source : "";
    payload[`${prefix}Assignment`] = channel.role === "music" || channel.role === "system" || channel.kind !== "source" ? "stereo" : "mono";
    payload[`${prefix}Enabled`] = channel.enabled;
    payload[`${prefix}Mute`] = channel.mute;
    payload[`${prefix}Solo`] = channel.solo;
    payload[`${prefix}Monitor`] = channel.id === autoMonitorId;
    payload[`${prefix}ProcessorEq`] = channel.processing.eq;
    payload[`${prefix}ProcessorComp`] = channel.processing.comp;
    payload[`${prefix}ProcessorNoise`] = channel.role === "vocal" ? true : channel.role === "system" ? false : channel.processing.noise;
    payload[`${prefix}ProcessorInsertFx`] = channel.role === "system" ? false : channel.processing.insertFx;
    payload[`${prefix}ProcessorDeEsser`] = channel.role === "vocal" && channel.processing.deEsser;
    payload[`${prefix}NoiseThresholdDb`] = channel.dynamics.noise.thresholdDb;
    payload[`${prefix}NoiseRangeDb`] = channel.dynamics.noise.rangeDb;
    payload[`${prefix}NoiseHoldMs`] = channel.dynamics.noise.holdMs;
    payload[`${prefix}NoiseReleaseMs`] = channel.dynamics.noise.releaseMs;
    payload[`${prefix}NoiseMode`] = channel.role === "vocal" ? vocalNoiseMode(channel.dynamics.noise.thresholdDb) : "gate";
    payload[`${prefix}NoiseRatio`] = channel.role === "vocal" ? vocalNoiseRatio(channel.dynamics.noise.thresholdDb) : 2;
    payload[`${prefix}CompThresholdDb`] = channel.dynamics.compressor.thresholdDb;
    payload[`${prefix}CompRatio`] = channel.dynamics.compressor.ratio;
    payload[`${prefix}CompAttackMs`] = channel.dynamics.compressor.attackMs;
    payload[`${prefix}CompReleaseMs`] = channel.dynamics.compressor.releaseMs;
    payload[`${prefix}DeEsserFrequencyHz`] = channel.dynamics.deEsser.frequencyHz;
    payload[`${prefix}DeEsserThresholdDb`] = channel.dynamics.deEsser.thresholdDb;
    payload[`${prefix}DeEsserMaxReductionDb`] = channel.dynamics.deEsser.maxReductionDb;
    payload[`${prefix}SendAEnabled`] = fxSendsEnabled && channel.sends["fx-a"].enabled;
    payload[`${prefix}SendAGainDb`] = fxSendsEnabled ? channel.sends["fx-a"].gainDb : -60;
    payload[`${prefix}SendBEnabled`] = fxSendsEnabled && channel.sends["fx-b"].enabled;
    payload[`${prefix}SendBGainDb`] = fxSendsEnabled ? channel.sends["fx-b"].gainDb : -60;
    channel.eqBands.forEach((band, bandIndex) => {
      payload[`${prefix}Eq${bandIndex}FreqHz`] = band.freqHz;
      payload[`${prefix}Eq${bandIndex}GainDb`] = band.gainDb;
      payload[`${prefix}Eq${bandIndex}Q`] = band.qValue ?? 0.707;
      payload[`${prefix}Eq${bandIndex}Type`] = band.type ?? "Peak";
    });
    payload[`${prefix}TrimDb`] = channel.trimDb;
    payload[`${prefix}FaderDb`] = channel.faderDb;
    payload[`${prefix}Pan`] = channel.pan / 100;
  });
  await window.localMixer.engineCommand("sync-mixer-graph", payload);
}

function liveVocalFxType(slotId: string, effectType: string, enabled: boolean) {
  if (slotId === "harmony" && enabled) return "live_harmony";
  return effectType;
}

function harmonyIntervalSemitones(interval: string) {
  if (interval === "+5th") return 7;
  if (interval === "-3rd") return -3;
  if (interval === "-5th") return -7;
  if (interval === "+Oct") return 12;
  return 3;
}

function autoMonitorChannel(snapshot: MixerSnapshot) {
  const eligible = snapshot.channels.filter((channel) => (
    channel.kind === "source" && channel.enabled && !channel.mute && Boolean(channel.source)
  ));
  return eligible.find((channel) => Boolean(channel.monitor)) ?? eligible[0];
}

function vocalNoiseRatio(thresholdDb: number) {
  const normalized = vocalNoiseNormalized(thresholdDb);
  return Math.round((1.6 + normalized * 8.4) * 10) / 10;
}

function vocalNoiseMode(thresholdDb: number) {
  return vocalNoiseNormalized(thresholdDb) >= 0.9 ? "gate" : "expander";
}

function vocalNoiseNormalized(thresholdDb: number) {
  return Math.max(0, Math.min(1, (thresholdDb + 55) / 45));
}

function channelColor(channel: ChannelState) {
  if (channel.role === "vocal") return "#18d6e7";
  if (channel.role === "music") return "#f3c842";
  if (channel.role === "group") return "#b568f0";
  if (channel.role === "master") return "#20f0a0";
  return "#6ed6e8";
}

function isMasterAudible(snapshot: MixerSnapshot) {
  const master = snapshot.channels.find((channel) => channel.kind === "master" || channel.role === "master");
  return !master || (master.enabled && !master.mute);
}

function linearPeakToDb(peak: number) {
  if (!Number.isFinite(peak) || peak <= 0.000001) return -60;
  return Math.max(-60, Math.min(6, 20 * Math.log10(peak)));
}

function systemOutputMeterFromInput(snapshot: MixerSnapshot, inputMeter: MeterLevel): MeterLevel {
  const system = snapshot.channels.find((channel) => channel.id === "system");
  if (!system?.enabled || system.mute) return { left: -60, right: -60, clip: false };
  const pan = Math.max(-1, Math.min(1, system.pan / 100));
  const leftPanDb = linearGainToDb(Math.min(1, 1 - pan));
  const rightPanDb = linearGainToDb(Math.min(1, 1 + pan));
  const gainDb = system.trimDb + system.faderDb + monitorSafetyGainDb;
  const left = clampMeterDb(inputMeter.left + gainDb + leftPanDb);
  const right = clampMeterDb((inputMeter.right ?? inputMeter.left) + gainDb + rightPanDb);
  return { left, right, clip: left >= 0 || right >= 0 };
}

function masterMeterFromSystem(snapshot: MixerSnapshot, systemOutputMeter: MeterLevel): MeterLevel {
  const master = snapshot.channels.find((channel) => channel.kind === "master" || channel.role === "master");
  if (!master?.enabled || master.mute) return { left: -60, right: -60, clip: false };
  const gainDb = master.trimDb + master.faderDb;
  const left = clampMeterDb(systemOutputMeter.left + gainDb);
  const right = clampMeterDb((systemOutputMeter.right ?? systemOutputMeter.left) + gainDb);
  return { left, right, clip: left >= 0 || right >= 0 };
}

function linearGainToDb(gain: number) {
  if (!Number.isFinite(gain) || gain <= 0.000001) return -60;
  return 20 * Math.log10(gain);
}

function clampMeterDb(value: number) {
  if (!Number.isFinite(value)) return -60;
  return Math.max(-60, Math.min(6, value));
}

function parseFxPrograms(programs: unknown[]): FxProgram[] {
  return programs.flatMap((program) => {
    if (!program || typeof program !== "object") return [];
    const item = program as Record<string, unknown>;
    const macro1 = item.macro1 as Record<string, unknown> | undefined;
    const macro2 = item.macro2 as Record<string, unknown> | undefined;
    if (typeof item.id !== "number" || typeof item.name !== "string" || typeof item.family !== "string") return [];
    if (!macro1 || !macro2 || typeof macro1.label !== "string" || typeof macro1.value !== "string") return [];
    if (typeof macro2.label !== "string" || typeof macro2.value !== "string") return [];
    return [{
      id: item.id,
      name: item.name,
      family: item.family,
      macro1: { label: macro1.label, value: macro1.value },
      macro2: { label: macro2.label, value: macro2.value }
    }];
  });
}

function TopBar({ projectName, time, rate, transportState, vocalFxEnabled, onVocalFx, onTimeline, onPlay, onStop, onRecord, onOpen, onSave, onCollect, onExport, onAddChannel, onRenameChannel, onRemoveChannel }: {
  projectName: string;
  time: string;
  rate: string;
  transportState: string;
  vocalFxEnabled: boolean;
  onVocalFx(): void;
  onTimeline(): void;
  onPlay(): void;
  onStop(): void;
  onRecord(): void;
  onOpen(): void;
  onSave(): void;
  onCollect(): void;
  onExport(): void;
  onAddChannel(): void;
  onRenameChannel(): void;
  onRemoveChannel(): void;
}) {
  return (
    <header className="top-bar">
      <h1>{projectName}</h1>
      <nav><button className="active">Mixer</button><button onClick={onVocalFx} disabled={!vocalFxEnabled}>Vocal FX</button><button onClick={onTimeline}>Timeline</button><button>Routing</button></nav>
      <div className="transport">
        <button aria-label="Stop" onClick={onStop}>■</button>
        <button aria-label={transportState === "playing" ? "Pause" : "Play"} className="play" onClick={onPlay}>
          {transportState === "playing" ? "II" : "▶"}
        </button>
        <button aria-label="Record" className="record" onClick={onRecord}>●</button>
      </div>
      <div className="time-display">{time}</div>
      <div className="rate">{rate}</div>
      <button className="settings" aria-label="Open Project" title="Open Project" onClick={onOpen}>□</button>
      <button className="settings" aria-label="Save Project" title="Save Project" onClick={onSave}>▣</button>
      <button className="settings" aria-label="Collect Media" title="Collect Media" onClick={onCollect}>◇</button>
      <button className="settings" aria-label="Export Mix" title="Export Mix" onClick={onExport}>⇩</button>
      <button className="settings" aria-label="Add Channel" title="Add Channel" onClick={onAddChannel}>＋</button>
      <button className="settings" aria-label="Rename Channel" title="Rename Channel" onClick={onRenameChannel}>✎</button>
      <button className="settings" aria-label="Remove Channel" title="Remove Channel" onClick={onRemoveChannel}>−</button>
      <button className="settings">⚙</button>
    </header>
  );
}

function parseSourceRole(role: string): "system" | "vocal" | "instrument" | "music" {
  const normalized = role.trim().toLowerCase();
  if (normalized === "system" || normalized === "vocal" || normalized === "instrument") return normalized;
  return "music";
}

function isBlackHoleDevice(device: HardwareDevice) {
  return /blackhole/i.test(`${device.name} ${device.uid}`);
}

function physicalOutputDevice(devices: HardwareDevice[], selectedUid: string) {
  const selected = devices.find((device) => device.uid === selectedUid && (device.outputChannels ?? 0) > 0 && !isBlackHoleDevice(device));
  if (selected) return selected;
  return devices.find((device) => device.defaultOutput && (device.outputChannels ?? 0) > 0 && !isBlackHoleDevice(device)) ??
    devices.find((device) => (device.outputChannels ?? 0) > 0 && !isBlackHoleDevice(device));
}

function Footer({ outputUid, outputOptions, onOutput, onHardware }: { outputUid: string; outputOptions: Array<{ value: string; label: string }>; onOutput(value: string): void; onHardware(): void }) {
  return (
    <footer className="footer-bar">
      <label>Output:<select value={outputUid} onChange={(event) => onOutput(event.target.value)}>
        <option value="">Default Output</option>
        {outputOptions.length === 0 ? <option value="preview-output">Headphones</option> : null}
        {outputOptions.map((option) => <option key={option.value} value={option.value}>{option.label}</option>)}
      </select></label>
      <div className="monitor-volume"><span>Monitor Volume</span><b>🔊</b><input type="range" value="55" readOnly /></div>
      <button onClick={onHardware}>HW</button>
      <button>DIM</button>
      <button>MUTE</button>
    </footer>
  );
}
