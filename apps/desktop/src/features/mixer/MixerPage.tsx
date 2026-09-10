import { useEffect, useMemo, useRef, useState } from "react";
import { PreviewAdapter } from "../../adapters/preview/PreviewAdapter";
import { CompactFxRow } from "../fx/components/CompactFxRow";
import { HarmonyQuickPanel } from "../harmony/components/HarmonyQuickPanel";
import { HardwareMonitorPanel } from "../hardware/components/HardwareMonitorPanel";
import { MediaImportPanel } from "../media/components/MediaImportPanel";
import { ChannelProcessingPanel } from "../processing/components/ChannelProcessingPanel";
import { SoundPadPanel } from "../sound-pads/components/SoundPadPanel";
import { VocalFxPanel } from "../vocal-fx/components/VocalFxPanel";
import { ChannelBank } from "./components/ChannelBank";
import type { ChannelState, FxProgram, MixerSnapshot } from "../../adapters/MixerControlPort";

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
  const [vocalFxOpen, setVocalFxOpen] = useState(false);
  const [harmonyOpen, setHarmonyOpen] = useState(true);
  const [devices, setDevices] = useState<HardwareDevice[]>([]);
  const [outputUid, setOutputUid] = useState("");
  const [transportState, setTransportState] = useState("stopped");
  const fxProgramInFlight = useRef<Record<"fx-a" | "fx-b", boolean>>({ "fx-a": false, "fx-b": false });
  const fxProgramDesired = useRef<Partial<Record<"fx-a" | "fx-b", number>>>({});

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

  async function refreshFxProgramBank() {
    if (!window.localMixer?.engineCommand) return;
    const result = await window.localMixer.engineCommand("fx-program-bank");
    if (!Array.isArray(result?.programs)) return;
    const programs = parseFxPrograms(result.programs);
    if (programs.length === 99) refresh(() => adapter.setPrograms(programs));
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

  async function selectFxProgram(unitId: "fx-a" | "fx-b", programId: number) {
    fxProgramDesired.current[unitId] = programId;
    if (!window.localMixer?.engineCommand) {
      refresh(() => adapter.setFxProgram(unitId, programId));
      delete fxProgramDesired.current[unitId];
      return;
    }
    if (fxProgramInFlight.current[unitId]) {
      refresh(() => adapter.setFxProgramPending(unitId, true));
      return;
    }

    fxProgramInFlight.current[unitId] = true;
    try {
      while (fxProgramDesired.current[unitId] !== undefined) {
        const desiredProgramId = fxProgramDesired.current[unitId] ?? programId;
        delete fxProgramDesired.current[unitId];
        await sendFxProgramRequest(unitId, desiredProgramId);
      }
    } finally {
      fxProgramInFlight.current[unitId] = false;
    }
  }

  async function sendFxProgramRequest(unitId: "fx-a" | "fx-b", programId: number) {
    const engineCommand = window.localMixer?.engineCommand;
    if (!engineCommand) return;
    const unit = adapter.getSnapshot().fxUnits.find((item) => item.id === unitId);
    refresh(() => adapter.setFxProgramPending(unitId, true));
    const result = await engineCommand("fx-unit-select-program", {
      unitId,
      programId,
      expectedRevision: unit?.revision ?? 0
    });
    if (result?.ok === false || result?.accepted === false || typeof result?.programId !== "number") {
      refresh(() => adapter.setFxError(unitId, String(result?.error ?? "FX_PROGRAM_REJECTED")));
      return;
    }
    refresh(() => adapter.ackFxProgram(unitId, result.programId as number, Number(result.revision ?? 0)));
  }

  async function setFxMacro(unitId: "fx-a" | "fx-b", macro: "macro1" | "macro2", value: string) {
    const unit = adapter.getSnapshot().fxUnits.find((item) => item.id === unitId);
    refresh(() => adapter.setFxProgramMacro(unitId, macro, value));
    if (!window.localMixer?.engineCommand || !unit) return;
    const result = await window.localMixer.engineCommand("fx-unit-set-macro", {
      unitId,
      macro,
      value,
      expectedRevision: unit.revision
    });
    if (result?.ok === false || result?.accepted === false) {
      refresh(() => adapter.setFxError(unitId, String(result?.error ?? "FX_MACRO_REJECTED")));
      return;
    }
    refresh(() => adapter.ackFxProgram(unitId, Number(result.programId ?? unit.programId), Number(result.revision ?? unit.revision), true));
  }

  async function resetFxProgram(unitId: "fx-a" | "fx-b") {
    const unit = adapter.getSnapshot().fxUnits.find((item) => item.id === unitId);
    if (!window.localMixer?.engineCommand || !unit) {
      refresh(() => adapter.resetFxProgram(unitId));
      return;
    }
    const result = await window.localMixer.engineCommand("fx-unit-reset-macros", {
      unitId,
      expectedRevision: unit.revision
    });
    if (result?.ok === false || result?.accepted === false) {
      refresh(() => adapter.setFxError(unitId, String(result?.error ?? "FX_RESET_REJECTED")));
      return;
    }
    refresh(() => adapter.ackFxProgram(unitId, Number(result.programId ?? unit.programId), Number(result.revision ?? unit.revision)));
  }

  async function setHarmonyEnabled(enabled: boolean) {
    const current = adapter.getSnapshot().harmony;
    const channel = adapter.getSnapshot().channels.find((item) => item.role === "vocal");
    if (!window.localMixer?.engineCommand || !channel) {
      refresh(() => adapter.setHarmonyEnabled(enabled));
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
    refresh(() => adapter.ackHarmony({
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
    void refreshDevices();
    void refreshFxProgramBank();
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
        onVocalFx={() => setVocalFxOpen(true)}
        onTimeline={() => setMediaImportOpen(true)}
        onPlay={() => void sendTransport(transportState === "playing" ? "transport-pause" : "transport-play")}
        onStop={() => void sendTransport("transport-stop")}
      />
      <div className="fx-stack">
        <CompactFxRow unit={fxA} program={programA} programs={snapshot.programs} onProgramChange={(id) => void selectFxProgram("fx-a", id)} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-a", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-a", value))} onMacro={(macro, value) => void setFxMacro("fx-a", macro, value)} onReset={() => void resetFxProgram("fx-a")} />
        <CompactFxRow unit={fxB} program={programB} programs={snapshot.programs} onProgramChange={(id) => void selectFxProgram("fx-b", id)} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-b", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-b", value))} onMacro={(macro, value) => void setFxMacro("fx-b", macro, value)} onReset={() => void resetFxProgram("fx-b")} />
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
            onProcessor={(id, processorId, enabled) => {
              refresh(() => adapter.setChannelProcessor(id, processorId, enabled));
              if (processorId === "insertFx" && enabled) {
                refresh(() => {
                  adapter.selectChannel(id);
                  adapter.selectVocalFxSlot("pitch-correct");
                });
                setVocalFxOpen(true);
              }
            }}
            onClipReset={(id) => refresh(() => adapter.resetClip(id))}
            onHarmonyToggle={() => void setHarmonyEnabled(!adapter.getSnapshot().harmony.enabled)}
            onHarmonySettings={(id) => {
              refresh(() => adapter.selectChannel(id));
              setHarmonyOpen(true);
            }}
          />
          {harmonyOpen ? (
            <HarmonyQuickPanel
              harmony={snapshot.harmony}
              onToggle={(enabled) => void setHarmonyEnabled(enabled)}
              onChange={(field, value) => void configureHarmony(field, value)}
              onClose={() => setHarmonyOpen(false)}
            />
          ) : null}
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
      <VocalFxPanel
        open={vocalFxOpen}
        vocalFx={snapshot.vocalFx}
        onClose={() => setVocalFxOpen(false)}
        onSelect={(slotId) => refresh(() => adapter.selectVocalFxSlot(slotId))}
        onToggle={(slotId, enabled) => refresh(() => adapter.setVocalFxSlotEnabled(slotId, enabled))}
        onPreset={(presetId) => refresh(() => adapter.applyVocalFxPreset(presetId))}
      />
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
    payload[`${prefix}ProcessorEq`] = channel.processing.eq;
    payload[`${prefix}ProcessorComp`] = channel.processing.comp;
    payload[`${prefix}ProcessorNoise`] = channel.processing.noise;
    payload[`${prefix}ProcessorDeEsser`] = false;
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

function channelColor(channel: ChannelState) {
  if (channel.role === "vocal") return "#18d6e7";
  if (channel.role === "music") return "#f3c842";
  if (channel.role === "group") return "#b568f0";
  if (channel.role === "master") return "#20f0a0";
  return "#6ed6e8";
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

function TopBar({ projectName, time, rate, status, mode, transportState, onVocalFx, onTimeline, onPlay, onStop }: {
  projectName: string;
  time: string;
  rate: string;
  status: string;
  mode: string;
  transportState: string;
  onVocalFx(): void;
  onTimeline(): void;
  onPlay(): void;
  onStop(): void;
}) {
  return (
    <header className="top-bar">
      <div className="window-dots"><span /><span /><span /></div>
      <h1>{projectName}</h1>
      <nav><button className="active">Mixer</button><button onClick={onVocalFx}>Vocal FX</button><button onClick={onTimeline}>Timeline</button><button>Routing</button></nav>
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
