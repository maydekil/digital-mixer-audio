import type { ChannelState, FxUnitId, ProcessorId } from "../../../adapters/MixerControlPort";
import { ClipIndicator } from "../../../components/audio/ClipIndicator";
import { LevelMeter } from "../../../components/audio/LevelMeter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { VerticalFader } from "../../../components/audio/VerticalFader";
import { Button } from "../../../components/ui/Button";

interface ChannelStripProps {
  channel: ChannelState;
  sourceOptions: Array<{ value: string; label: string }>;
  onSelect(): void;
  onEnabled(enabled: boolean): void;
  onSource(source: string): void;
  onTrim(value: number): void;
  onPan(value: number): void;
  onFader(value: number): void;
  onSend(unitId: FxUnitId, value: number): void;
  onMute(muted: boolean): void;
  onSolo(solo: boolean): void;
  onMonitor(monitor: boolean): void;
  onRecordArm(armed: boolean): void;
  onProcessor(processorId: ProcessorId, enabled: boolean): void;
  onClipReset(): void;
  onHarmonyToggle?(): void;
  onHarmonySettings?(): void;
}

export function ChannelStrip({ channel, sourceOptions, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onSend, onMute, onSolo, onMonitor, onRecordArm, onProcessor, onClipReset, onHarmonyToggle, onHarmonySettings }: ChannelStripProps) {
  const isMaster = channel.kind === "master";
  const isGroup = channel.kind === "group";
  const meter = channel.enabled ? channel.meter : { left: -60, right: -60, clip: false };

  return (
    <article className={`channel-strip ${channel.selected ? "is-selected" : ""} ${isMaster ? "is-master" : ""} ${!channel.enabled ? "is-disabled" : ""}`} onClick={onSelect}>
      <header>
        <div className="channel-title-row">
          <strong>{channel.name}</strong>
          <Button tone={channel.enabled ? "green" : "neutral"} active={channel.enabled} onClick={() => onEnabled(!channel.enabled)}>{channel.enabled ? "ON" : "OFF"}</Button>
        </div>
        {sourceOptions.length > 0 ? (
          <select className="channel-source-select" value={channel.source} onChange={(event) => onSource(event.target.value)} onClick={(event) => event.stopPropagation()}>
            {sourceOptions.map((option) => <option key={option.value} value={option.value}>{option.label}</option>)}
          </select>
        ) : <span>{channel.source}</span>}
      </header>
      <RotaryKnob label="Gain" value={`${channel.trimDb.toFixed(1)} dB`} numericValue={channel.trimDb} min={-24} max={24} step={0.5} onChange={onTrim} />
      {isMaster ? <MasterUpperControls /> : <ProcessingButtons channel={channel} onProcessor={onProcessor} />}
      {channel.harmonyVisible ? (
        <div className="harmony-shortcut">
          <Button tone="violet" active={channel.harmonyEnabled} onClick={onHarmonyToggle}>HARMONY {channel.harmonyEnabled ? "ON" : "OFF"}</Button>
          <Button aria-label="Harmony settings" onClick={onHarmonySettings}>⚙</Button>
        </div>
      ) : !isMaster ? <div className="harmony-shortcut is-placeholder" aria-hidden="true" /> : null}
      <div className="strip-divider" />
      {!isMaster ? <RotaryKnob label="Pan" value={formatPan(channel.pan)} numericValue={channel.pan} min={-100} max={100} step={1} onChange={onPan} /> : <div className="master-spacer" aria-hidden="true" />}
      {!isMaster ? <SendPair channel={channel} onSend={onSend} /> : null}
      <div className="fader-meter-row">
        <VerticalFader valueDb={channel.faderDb} onChange={onFader} label={`${channel.name} fader`} />
        <div className="strip-meter-stack">
          <ClipIndicator active={meter.clip} onReset={onClipReset} />
          <LevelMeter level={meter} vertical />
        </div>
      </div>
      <div className="strip-actions">
        <Button active={channel.mute} onClick={() => onMute(!channel.mute)}>M</Button>
        <Button active={channel.solo} onClick={() => onSolo(!channel.solo)}>S</Button>
        {!isGroup && !isMaster ? <Button active={channel.monitor} onClick={() => onMonitor(!channel.monitor)}>MON</Button> : null}
        {!isGroup && !isMaster ? <Button tone="danger" active={channel.recordArm} onClick={() => onRecordArm(!channel.recordArm)}>REC</Button> : null}
      </div>
    </article>
  );
}

function ProcessingButtons({ channel, onProcessor }: { channel: ChannelState; onProcessor(processorId: ProcessorId, enabled: boolean): void }) {
  const buttons: Array<{ id: ProcessorId; label: string }> = [
    { id: "eq", label: "EQ" },
    { id: "comp", label: "COMP" },
    { id: "noise", label: "NOISE" }
  ];
  if (channel.role !== "system") buttons.push({ id: "insertFx", label: "INSERT FX" });

  return (
    <div className="processing-buttons">
      {buttons.map((button) => (
        <Button key={button.id} active={channel.processing[button.id]} onClick={() => onProcessor(button.id, !channel.processing[button.id])}>
          {button.label}
        </Button>
      ))}
    </div>
  );
}

function MasterUpperControls() {
  return (
    <div className="master-upper">
      <Button tone="green" active>LIMITER</Button>
      <div className="master-upper-spacer" aria-hidden="true" />
    </div>
  );
}

function SendPair({ channel, onSend }: { channel: ChannelState; onSend(unitId: FxUnitId, value: number): void }) {
  const sendA = channel.sends["fx-a"];
  const sendB = channel.sends["fx-b"];
  const disabled = channel.role === "system";
  return (
    <div className="send-pair">
      <RotaryKnob label="SEND A" value={disabled || !sendA.enabled ? "OFF" : `${sendA.gainDb} dB`} numericValue={disabled ? -60 : sendA.gainDb} min={-60} max={10} tone="amber" size="sm" disabled={disabled} onChange={(value) => onSend("fx-a", value)} />
      <RotaryKnob label="SEND B" value={disabled || !sendB.enabled ? "OFF" : `${sendB.gainDb} dB`} numericValue={disabled ? -60 : sendB.gainDb} min={-60} max={10} tone="cyan" size="sm" disabled={disabled} onChange={(value) => onSend("fx-b", value)} />
    </div>
  );
}

function formatPan(value: number) {
  if (value < 0) return `L ${Math.abs(value)}`;
  if (value > 0) return `R ${value}`;
  return "L R";
}
