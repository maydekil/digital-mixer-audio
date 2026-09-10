import { useEffect, useMemo, useState } from "react";
import { PreviewAdapter } from "../../adapters/preview/PreviewAdapter";
import { CompactFxRow } from "../fx/components/CompactFxRow";
import { HarmonyQuickPanel } from "../harmony/components/HarmonyQuickPanel";
import { HardwareMonitorPanel } from "../hardware/components/HardwareMonitorPanel";
import { MediaImportPanel } from "../media/components/MediaImportPanel";
import { ChannelProcessingPanel } from "../processing/components/ChannelProcessingPanel";
import { SoundPadPanel } from "../sound-pads/components/SoundPadPanel";
import { ChannelBank } from "./components/ChannelBank";
import type { ChannelState, MixerSnapshot } from "../../adapters/MixerControlPort";

interface HardwareDevice {
  uid: string;
  name: string;
  defaultInput?: boolean;
  defaultOutput?: boolean;
  inputChannels?: number;
  outputChannels?: number;
}

export function MixerPage() {
  const adapter = useMemo(() => new PreviewAdapter(), []);
  const [snapshot, setSnapshot] = useState(adapter.getSnapshot());
  const [hardwareOpen, setHardwareOpen] = useState(false);
  const [mediaImportOpen, setMediaImportOpen] = useState(false);
  const [devices, setDevices] = useState<HardwareDevice[]>([]);
  const [outputUid, setOutputUid] = useState("");
  const [transportState, setTransportState] = useState("stopped");

  function refresh(action: () => void) {
    action();
    setSnapshot(structuredClone(adapter.getSnapshot()));
  }

  async function refreshDevices() {
    const result = await window.localMixer?.engineCommand?.("list-devices");
    const nextDevices = Array.isArray(result?.devices) ? result.devices as HardwareDevice[] : [];
    setDevices(nextDevices);
    const defaultOutput = nextDevices.find((device) => device.defaultOutput && (device.outputChannels ?? 0) > 0) ?? nextDevices.find((device) => (device.outputChannels ?? 0) > 0);
    setOutputUid((current) => current || defaultOutput?.uid || "");
  }

  async function runChannelMonitor(channelId: string, monitor: boolean) {
    if (!window.localMixer?.engineCommand) return;
    const channel = adapter.getSnapshot().channels.find((item) => item.id === channelId);
    if (!channel || channel.kind !== "source") return;
    await syncMixerGraph(adapter.getSnapshot(), outputUid);
    if (!monitor || !channel.enabled || !channel.source) {
      await window.localMixer.engineCommand("stop-mixer-monitor");
      return;
    }
    await window.localMixer.engineCommand("start-mixer-monitor", {
      sampleRate: 48000,
      monitorGainDb: -18
    });
  }

  async function sendTransport(type: "transport-play" | "transport-pause" | "transport-stop") {
    if (!window.localMixer?.engineCommand) return;
    const result = await window.localMixer.engineCommand(type);
    if (typeof result?.state === "string") setTransportState(result.state);
  }

  useEffect(() => {
    void refreshDevices();
  }, []);

  useEffect(() => {
    void syncMixerGraph(snapshot, outputUid);
  }, [snapshot, outputUid]);

  const selected = snapshot.channels.find((channel) => channel.id === snapshot.selectedChannelId) ?? snapshot.channels[0];
  const fxA = snapshot.fxUnits[0];
  const fxB = snapshot.fxUnits[1];
  const programA = snapshot.programs.find((program) => program.id === fxA.programId) ?? snapshot.programs[0];
  const programB = snapshot.programs.find((program) => program.id === fxB.programId) ?? snapshot.programs[0];
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

  return (
    <main className="mixer-app">
      <TopBar
        projectName={snapshot.projectName}
        time={snapshot.transportTime}
        rate={snapshot.sampleRateLabel}
        status={snapshot.engineStatus}
        mode={snapshot.modeLabel}
        transportState={transportState}
        onTimeline={() => setMediaImportOpen(true)}
        onPlay={() => void sendTransport(transportState === "playing" ? "transport-pause" : "transport-play")}
        onStop={() => void sendTransport("transport-stop")}
      />
      <div className="fx-stack">
        <CompactFxRow unit={fxA} program={programA} programs={snapshot.programs} onProgramChange={(id) => refresh(() => adapter.setFxProgram("fx-a", id))} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-a", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-a", value))} onReset={() => refresh(() => adapter.resetFxProgram("fx-a"))} />
        <CompactFxRow unit={fxB} program={programB} programs={snapshot.programs} onProgramChange={(id) => refresh(() => adapter.setFxProgram("fx-b", id))} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-b", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-b", value))} onReset={() => refresh(() => adapter.resetFxProgram("fx-b"))} />
      </div>
      <section className="workspace">
        <div className="left-zone">
          <ChannelBank
            channels={snapshot.channels}
            sourceOptions={sourceOptions}
            onSelect={(id) => refresh(() => adapter.selectChannel(id))}
            onEnabled={(id, enabled) => {
              refresh(() => adapter.setChannelEnabled(id, enabled));
              if (!enabled) void window.localMixer?.engineCommand?.("stop-mixer-monitor");
            }}
            onSource={(id, source) => refresh(() => adapter.setChannelSource(id, source))}
            onTrim={(id, value) => refresh(() => adapter.setChannelTrim(id, value))}
            onPan={(id, value) => refresh(() => adapter.setChannelPan(id, value))}
            onFader={(id, value) => refresh(() => adapter.setChannelFader(id, value))}
            onSend={(id, unitId, value) => refresh(() => adapter.setChannelSend(id, unitId, value))}
            onMute={(id, muted) => refresh(() => adapter.setChannelMute(id, muted))}
            onSolo={(id, solo) => refresh(() => adapter.setChannelSolo(id, solo))}
            onMonitor={(id, monitor) => {
              refresh(() => adapter.setChannelMonitor(id, monitor));
              void runChannelMonitor(id, monitor);
            }}
            onRecordArm={(id, armed) => refresh(() => adapter.setChannelRecordArm(id, armed))}
            onProcessor={(id, processorId, enabled) => refresh(() => adapter.setChannelProcessor(id, processorId, enabled))}
            onClipReset={(id) => refresh(() => adapter.resetClip(id))}
            onHarmonyToggle={() => refresh(() => adapter.setHarmonyEnabled(!snapshot.harmony.enabled))}
          />
          <HarmonyQuickPanel
            harmony={snapshot.harmony}
            onToggle={(enabled) => refresh(() => adapter.setHarmonyEnabled(enabled))}
            onChange={(field, value) => refresh(() => adapter.updateHarmony(field, value))}
          />
        </div>
        <div className="right-zone">
          <ChannelProcessingPanel
            channel={selected}
            eqBands={selected.eqBands}
            linkedProgram={programA}
            onSendA={(value) => refresh(() => adapter.setChannelSend(selected.id, "fx-a", value))}
            onEqChange={(bandId, field, value) => refresh(() => adapter.updateEqBand(bandId, field, value))}
          />
          <SoundPadPanel />
        </div>
      </section>
      <Footer outputUid={outputUid} outputOptions={outputOptions} onOutput={setOutputUid} onHardware={() => setHardwareOpen(true)} />
      <HardwareMonitorPanel open={hardwareOpen} onClose={() => setHardwareOpen(false)} />
      <MediaImportPanel open={mediaImportOpen} onClose={() => setMediaImportOpen(false)} />
    </main>
  );
}

async function syncMixerGraph(snapshot: MixerSnapshot, outputUid: string) {
  if (!window.localMixer?.engineCommand) return;
  const channels = snapshot.channels.slice(0, 32);
  const payload: Record<string, string | number | boolean> = {
    channelCount: channels.length,
    outputUid,
    monitorGainDb: -18
  };
  channels.forEach((channel, index) => {
    const prefix = `channel${index}`;
    payload[`${prefix}Id`] = channel.id;
    payload[`${prefix}Kind`] = channel.kind;
    payload[`${prefix}Name`] = channel.name;
    payload[`${prefix}Color`] = channelColor(channel);
    payload[`${prefix}SourceUid`] = channel.kind === "source" ? channel.source : "";
    payload[`${prefix}Assignment`] = channel.role === "music" || channel.kind !== "source" ? "stereo" : "mono";
    payload[`${prefix}Enabled`] = channel.enabled;
    payload[`${prefix}Mute`] = channel.mute;
    payload[`${prefix}Solo`] = channel.solo;
    payload[`${prefix}Monitor`] = Boolean(channel.monitor);
    payload[`${prefix}TrimDb`] = channel.trimDb;
    payload[`${prefix}FaderDb`] = channel.faderDb;
    payload[`${prefix}Pan`] = channel.pan / 100;
  });
  await window.localMixer.engineCommand("sync-mixer-graph", payload);
}

function channelColor(channel: ChannelState) {
  if (channel.role === "vocal") return "#18d6e7";
  if (channel.role === "music") return "#f3c842";
  if (channel.role === "group") return "#b568f0";
  if (channel.role === "master") return "#20f0a0";
  return "#6ed6e8";
}

function TopBar({ projectName, time, rate, status, mode, transportState, onTimeline, onPlay, onStop }: {
  projectName: string;
  time: string;
  rate: string;
  status: string;
  mode: string;
  transportState: string;
  onTimeline(): void;
  onPlay(): void;
  onStop(): void;
}) {
  return (
    <header className="top-bar">
      <div className="window-dots"><span /><span /><span /></div>
      <h1>{projectName}</h1>
      <nav><button className="active">Mixer</button><button>Vocal FX</button><button onClick={onTimeline}>Timeline</button><button>Routing</button></nav>
      <div className="transport">
        <button aria-label="Stop" onClick={onStop}>■</button>
        <button aria-label={transportState === "playing" ? "Pause" : "Play"} className="play" onClick={onPlay}>
          {transportState === "playing" ? "II" : "▶"}
        </button>
        <button aria-label="Record" className="record">●</button>
      </div>
      <div className="time-display">{time}</div>
      <div className="rate">{rate}</div>
      <div className="engine-status"><span />{status}</div>
      <div className="preview-banner">{mode}</div>
      <button className="settings">⚙</button>
    </header>
  );
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
