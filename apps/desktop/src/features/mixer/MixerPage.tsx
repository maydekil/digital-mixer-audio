import { useMemo, useState } from "react";
import { PreviewAdapter } from "../../adapters/preview/PreviewAdapter";
import { CompactFxRow } from "../fx/components/CompactFxRow";
import { HarmonyQuickPanel } from "../harmony/components/HarmonyQuickPanel";
import { ChannelProcessingPanel } from "../processing/components/ChannelProcessingPanel";
import { SoundPadPanel } from "../sound-pads/components/SoundPadPanel";
import { ChannelBank } from "./components/ChannelBank";

export function MixerPage() {
  const adapter = useMemo(() => new PreviewAdapter(), []);
  const [snapshot, setSnapshot] = useState(adapter.getSnapshot());

  function refresh(action: () => void) {
    action();
    setSnapshot(structuredClone(adapter.getSnapshot()));
  }

  const selected = snapshot.channels.find((channel) => channel.id === snapshot.selectedChannelId) ?? snapshot.channels[0];
  const fxA = snapshot.fxUnits[0];
  const fxB = snapshot.fxUnits[1];
  const programA = snapshot.programs.find((program) => program.id === fxA.programId) ?? snapshot.programs[0];
  const programB = snapshot.programs.find((program) => program.id === fxB.programId) ?? snapshot.programs[0];

  return (
    <main className="mixer-app">
      <TopBar projectName={snapshot.projectName} time={snapshot.transportTime} rate={snapshot.sampleRateLabel} status={snapshot.engineStatus} mode={snapshot.modeLabel} />
      <div className="fx-stack">
        <CompactFxRow unit={fxA} program={programA} programs={snapshot.programs} onProgramChange={(id) => refresh(() => adapter.setFxProgram("fx-a", id))} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-a", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-a", value))} onReset={() => refresh(() => adapter.resetFxProgram("fx-a"))} />
        <CompactFxRow unit={fxB} program={programB} programs={snapshot.programs} onProgramChange={(id) => refresh(() => adapter.setFxProgram("fx-b", id))} onToggle={(enabled) => refresh(() => adapter.setFxEnabled("fx-b", enabled))} onReturn={(value) => refresh(() => adapter.setFxReturn("fx-b", value))} onReset={() => refresh(() => adapter.resetFxProgram("fx-b"))} />
      </div>
      <section className="workspace">
        <div className="left-zone">
          <ChannelBank
            channels={snapshot.channels}
            onSelect={(id) => refresh(() => adapter.selectChannel(id))}
            onTrim={(id, value) => refresh(() => adapter.setChannelTrim(id, value))}
            onPan={(id, value) => refresh(() => adapter.setChannelPan(id, value))}
            onFader={(id, value) => refresh(() => adapter.setChannelFader(id, value))}
            onSend={(id, unitId, value) => refresh(() => adapter.setChannelSend(id, unitId, value))}
            onMute={(id, muted) => refresh(() => adapter.setChannelMute(id, muted))}
            onSolo={(id, solo) => refresh(() => adapter.setChannelSolo(id, solo))}
            onMonitor={(id, monitor) => refresh(() => adapter.setChannelMonitor(id, monitor))}
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
      <Footer />
    </main>
  );
}

function TopBar({ projectName, time, rate, status, mode }: { projectName: string; time: string; rate: string; status: string; mode: string }) {
  return (
    <header className="top-bar">
      <div className="window-dots"><span /><span /><span /></div>
      <h1>{projectName}</h1>
      <nav><button className="active">Mixer</button><button>Vocal FX</button><button>Timeline</button><button>Routing</button></nav>
      <div className="transport"><button>■</button><button className="play">▶</button><button className="record">●</button></div>
      <div className="time-display">{time}</div>
      <div className="rate">{rate}</div>
      <div className="engine-status"><span />{status}</div>
      <div className="preview-banner">{mode}</div>
      <button className="settings">⚙</button>
    </header>
  );
}

function Footer() {
  return (
    <footer className="footer-bar">
      <label>Output:<select><option>Headphones</option><option>Output 1-2</option></select></label>
      <div className="monitor-volume"><span>Monitor Volume</span><b>🔊</b><input type="range" value="55" readOnly /></div>
      <button>DIM</button>
      <button>MUTE</button>
    </footer>
  );
}
