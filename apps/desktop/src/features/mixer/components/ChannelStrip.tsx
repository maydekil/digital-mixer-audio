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
  onNoiseAmount(amount: number): void;
  onCompressorParam(field: keyof ChannelState["dynamics"]["compressor"], value: number): void;
  onCompressorEnabled(enabled: boolean): void;
  onMute(muted: boolean): void;
  onRecordArm(armed: boolean): void;
  onVocalFxPreset(presetId: string): void;
  onClipReset(): void;
  onHarmonyToggle?(): void;
  onHarmonySettings?(): void;
}

export function ChannelStrip({ channel, sourceOptions, vocalFxPresetId, vocalFxPresets, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onEqBand, onNoiseAmount, onCompressorParam, onCompressorEnabled, onMute, onRecordArm, onVocalFxPreset, onClipReset, onHarmonyToggle, onHarmonySettings }: ChannelStripProps) {
  const isMaster = channel.kind === "master";
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
      {channel.role === "vocal" ? <ChannelNoiseControl channel={channel} onAmount={onNoiseAmount} /> : null}
      {!isMaster ? <ChannelCompressorControls channel={channel} onParam={onCompressorParam} onEnabled={onCompressorEnabled} /> : null}
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
        {!isMaster ? <Button tone="danger" active={channel.recordArm} onClick={() => onRecordArm(!channel.recordArm)}>REC</Button> : null}
      </div>
    </article>
  );
}

function ChannelNoiseControl({ channel, onAmount }: {
  channel: ChannelState;
  onAmount(amount: number): void;
}) {
  const amount = channel.processing.noise ? noiseAmountFromThreshold(channel.dynamics.noise.thresholdDb) : 0;
  return (
    <div className={`channel-noise-section ${channel.processing.noise ? "is-active" : "is-bypassed"}`}>
      <RotaryKnob
        label="NOISE"
        value={amount > 0 ? `${amount}` : "OFF"}
        numericValue={amount}
        min={0}
        max={100}
        step={5}
        size="sm"
        onChange={onAmount}
      />
    </div>
  );
}

function noiseAmountFromThreshold(thresholdDb: number) {
  const normalized = Math.max(0, Math.min(1, (thresholdDb + 80) / 58));
  return Math.round((normalized ** (1 / 0.55)) * 100 / 5) * 5;
}

function ChannelCompressorControls({ channel, onParam, onEnabled }: {
  channel: ChannelState;
  onParam(field: keyof ChannelState["dynamics"]["compressor"], value: number): void;
  onEnabled(enabled: boolean): void;
}) {
  const compressor = channel.dynamics.compressor;
  return (
    <div className={`channel-compressor-section ${channel.processing.comp ? "is-active" : "is-bypassed"}`}>
      <Button tone="cyan" active={channel.processing.comp} onClick={() => onEnabled(!channel.processing.comp)}>COMP</Button>
      <div className="channel-compressor-controls">
        <RotaryKnob label="Threshold" value={`${compressor.thresholdDb} dB`} numericValue={compressor.thresholdDb} min={-80} max={0} size="sm" onChange={(value) => onParam("thresholdDb", value)} />
        <RotaryKnob label="Ratio" value={`${compressor.ratio}:1`} numericValue={compressor.ratio} min={1} max={20} step={0.5} size="sm" onChange={(value) => onParam("ratio", value)} />
        <RotaryKnob label="Attack" value={`${compressor.attackMs} ms`} numericValue={compressor.attackMs} min={0.1} max={200} step={0.1} size="sm" onChange={(value) => onParam("attackMs", value)} />
        <RotaryKnob label="Release" value={`${compressor.releaseMs} ms`} numericValue={compressor.releaseMs} min={10} max={3000} size="sm" onChange={(value) => onParam("releaseMs", value)} />
      </div>
    </div>
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
