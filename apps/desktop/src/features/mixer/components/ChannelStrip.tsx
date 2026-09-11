import type { ChannelState, EqBandState, VocalFxPreset } from "../../../adapters/MixerControlPort";
import { ClipIndicator } from "../../../components/audio/ClipIndicator";
import { LevelMeter } from "../../../components/audio/LevelMeter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { VerticalFader } from "../../../components/audio/VerticalFader";
import { Button } from "../../../components/ui/Button";

interface ChannelStripProps {
  channel: ChannelState;
  sourceOptions: Array<{ value: string; label: string }>;
  vocalFxPresetId: string;
  vocalFxPresets: VocalFxPreset[];
  onSelect(): void;
  onEnabled(enabled: boolean): void;
  onSource(source: string): void;
  onTrim(value: number): void;
  onPan(value: number): void;
  onFader(value: number): void;
  onEqBand(bandId: EqBandState["id"], gainDb: number): void;
  onMute(muted: boolean): void;
  onSolo(solo: boolean): void;
  onMonitor(monitor: boolean): void;
  onRecordArm(armed: boolean): void;
  onVocalFxPreset(presetId: string): void;
  onClipReset(): void;
  onHarmonyToggle?(): void;
  onHarmonySettings?(): void;
}

export function ChannelStrip({ channel, sourceOptions, vocalFxPresetId, vocalFxPresets, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onEqBand, onMute, onSolo, onMonitor, onRecordArm, onVocalFxPreset, onClipReset, onHarmonyToggle, onHarmonySettings }: ChannelStripProps) {
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
      {isMaster ? <MasterUpperControls /> : <ChannelToneControls channel={channel} vocalFxPresetId={vocalFxPresetId} vocalFxPresets={vocalFxPresets} onEqBand={onEqBand} onVocalFxPreset={onVocalFxPreset} />}
      {channel.harmonyVisible ? (
        <div className="harmony-shortcut">
          <Button tone="violet" active={channel.harmonyEnabled} onClick={onHarmonyToggle}>HARMONY {channel.harmonyEnabled ? "ON" : "OFF"}</Button>
          <Button aria-label="Harmony settings" onClick={onHarmonySettings}>⚙</Button>
        </div>
      ) : !isMaster ? <div className="harmony-shortcut is-placeholder" aria-hidden="true" /> : null}
      <div className="strip-divider" />
      {!isMaster ? <RotaryKnob label="Pan" value={formatPan(channel.pan)} numericValue={channel.pan} min={-100} max={100} step={1} onChange={onPan} /> : <div className="master-spacer" aria-hidden="true" />}
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

function ChannelToneControls({ channel, vocalFxPresetId, vocalFxPresets, onEqBand, onVocalFxPreset }: {
  channel: ChannelState;
  vocalFxPresetId: string;
  vocalFxPresets: VocalFxPreset[];
  onEqBand(bandId: EqBandState["id"], gainDb: number): void;
  onVocalFxPreset(presetId: string): void;
}) {
  const toneBands: Array<{ id: EqBandState["id"]; label: string }> = [
    { id: "low", label: "LOW" },
    { id: "mid1", label: "MID 1" },
    { id: "mid2", label: "MID 2" },
    { id: "high", label: "HIGH" }
  ];

  return (
    <div className="channel-tone-section">
      <div className="channel-tone-controls">
        {toneBands.map((tone) => {
          const band = channel.eqBands.find((item) => item.id === tone.id);
          const gainDb = band?.gainDb ?? 0;
          return (
            <RotaryKnob
              key={tone.id}
              label={tone.label}
              value={`${gainDb >= 0 ? "+" : ""}${gainDb.toFixed(1)}`}
              numericValue={gainDb}
              min={-12}
              max={12}
              step={0.5}
              size="sm"
              onChange={(value) => onEqBand(tone.id, value)}
            />
          );
        })}
      </div>
      {channel.role === "vocal" ? (
        <select
          className="channel-insert-preset"
          value={channel.processing.insertFx ? vocalFxPresetId : "default"}
          onChange={(event) => onVocalFxPreset(event.target.value)}
          onClick={(event) => event.stopPropagation()}
          aria-label="Voice preset"
        >
          <option value="default">Default</option>
          {vocalFxPresets.map((preset) => <option key={preset.id} value={preset.id}>{preset.name}</option>)}
        </select>
      ) : null}
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

function formatPan(value: number) {
  if (value < 0) return `L ${Math.abs(value)}`;
  if (value > 0) return `R ${value}`;
  return "L R";
}
