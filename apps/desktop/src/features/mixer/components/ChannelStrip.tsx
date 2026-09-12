import type { ChannelState, EqBandState, VocalFxPreset, VocalFxSlot } from "../../../adapters/MixerControlPort";
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
  vocalFxSlots: VocalFxSlot[];
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
  onVocalFxMix(slotId: string, amount: number): void;
  onClipReset(): void;
  onHarmonyToggle?(): void;
  onHarmonySettings?(): void;
}

export function ChannelStrip({ channel, sourceOptions, vocalFxPresetId, vocalFxPresets, vocalFxSlots, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onEqBand, onNoiseAmount, onCompressorParam, onCompressorEnabled, onMute, onRecordArm, onVocalFxPreset, onVocalFxMix, onClipReset, onHarmonyToggle, onHarmonySettings }: ChannelStripProps) {
  const isMaster = channel.kind === "master";
  const isVocal = channel.role === "vocal";
  const isSystem = channel.role === "system";
  const meter = channel.enabled ? channel.meter : { left: -60, right: -60, clip: false };

  return (
    <article className={`channel-strip ${channel.selected ? "is-selected" : ""} ${isMaster ? "is-master" : ""} ${isVocal ? "is-vocal" : ""} ${isSystem ? "is-system" : ""} ${!channel.enabled ? "is-disabled" : ""}`} onClick={onSelect}>
      <header>
        <div className="channel-title-row">
          <strong>{channel.name}</strong>
          <Button tone={channel.enabled ? "green" : "neutral"} active={channel.enabled} onClick={() => onEnabled(!channel.enabled)}>{channel.enabled ? "ON" : "OFF"}</Button>
        </div>
        <div className={`channel-header-controls ${isVocal ? "has-preset" : ""}`}>
          {sourceOptions.length > 0 ? (
            <select className="channel-source-select" value={channel.source} onChange={(event) => onSource(event.target.value)} onClick={(event) => event.stopPropagation()} aria-label={`${channel.name} input`}>
              {sourceOptions.map((option) => <option key={option.value} value={option.value}>{option.label}</option>)}
            </select>
          ) : <span>{channel.source}</span>}
          {isVocal ? <VoicePresetSelect presetId={vocalFxPresetId} presets={vocalFxPresets} insertEnabled={channel.processing.insertFx} onPreset={onVocalFxPreset} /> : null}
        </div>
      </header>
      <RotaryKnob label="Gain" value={`${channel.trimDb.toFixed(1)} dB`} numericValue={channel.trimDb} min={-24} max={24} step={0.5} onChange={onTrim} />
      {isMaster ? <MasterUpperControls /> : <ChannelToneControls channel={channel} vocalFxSlots={vocalFxSlots} onEqBand={onEqBand} onVocalFxMix={onVocalFxMix} />}
      {isVocal ? (
        <div className="voice-dynamics-section">
          <ChannelCompressorControls channel={channel} onParam={onCompressorParam} onEnabled={onCompressorEnabled} />
          <ChannelNoiseControl channel={channel} onAmount={onNoiseAmount} />
        </div>
      ) : !isMaster ? (
        <ChannelCompressorControls channel={channel} onParam={onCompressorParam} onEnabled={onCompressorEnabled} />
      ) : null}
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

function VoicePresetSelect({ presetId, presets, insertEnabled, onPreset }: {
  presetId: string;
  presets: VocalFxPreset[];
  insertEnabled: boolean;
  onPreset(presetId: string): void;
}) {
  return (
    <select
      className="channel-insert-preset"
      value={insertEnabled ? presetId : "default"}
      onChange={(event) => onPreset(event.target.value)}
      onClick={(event) => event.stopPropagation()}
      aria-label="Voice preset"
    >
      <option value="default">Default</option>
      {presets.map((preset) => <option key={preset.id} value={preset.id}>{preset.name}</option>)}
    </select>
  );
}

function ChannelNoiseControl({ channel, onAmount }: {
  channel: ChannelState;
  onAmount(amount: number): void;
}) {
  const noiseActive = channel.role === "vocal" || channel.processing.noise;
  const amount = noiseActive ? noiseAmountFromThreshold(channel.dynamics.noise.thresholdDb) : 0;
  return (
    <div className={`channel-noise-section ${noiseActive ? "is-active" : "is-bypassed"}`}>
      <RotaryKnob
        label="NOISE"
        value={amount > 0 ? `${amount}` : "AUTO"}
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
  const normalized = Math.max(0, Math.min(1, (thresholdDb + 55) / 45));
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

function ChannelToneControls({ channel, vocalFxSlots, onEqBand, onVocalFxMix }: {
  channel: ChannelState;
  vocalFxSlots: VocalFxSlot[];
  onEqBand(bandId: EqBandState["id"], gainDb: number): void;
  onVocalFxMix(slotId: string, amount: number): void;
}) {
  const toneBands: Array<{ id: EqBandState["id"]; label: string }> = [
    { id: "low", label: "LOW" },
    { id: "mid1", label: "MID 1" },
    { id: "mid2", label: "MID 2" },
    { id: "high", label: "HIGH" }
  ];

  const toneControls = (
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
  );

  if (channel.role === "vocal") {
    return (
      <div className="channel-tone-section voice-tone-section">
        <div className="voice-tone-frame">{toneControls}</div>
        <div className="voice-space-frame">
          <VoiceSpaceControls slots={vocalFxSlots} onMix={onVocalFxMix} />
        </div>
      </div>
    );
  }

  return (
    <div className="channel-tone-section">
      {toneControls}
    </div>
  );
}

function VoiceSpaceControls({ slots, onMix }: {
  slots: VocalFxSlot[];
  onMix(slotId: string, amount: number): void;
}) {
  const reverb = slots.find((slot) => slot.id === "plate");
  const echo = slots.find((slot) => slot.id === "stereo-delay");
  return (
    <div className="channel-space-controls">
      {reverb ? <SpaceKnob label="REVERB" slot={reverb} onMix={onMix} /> : null}
      {echo ? <SpaceKnob label="ECHO" slot={echo} onMix={onMix} /> : null}
    </div>
  );
}

function SpaceKnob({ label, slot, onMix }: {
  label: string;
  slot: VocalFxSlot;
  onMix(slotId: string, amount: number): void;
}) {
  const amount = slot.enabled ? Math.round(slot.mix * 100) : 0;
  return (
    <RotaryKnob
      label={label}
      value={amount > 0 ? `${amount}` : "OFF"}
      numericValue={amount}
      min={0}
      max={100}
      step={5}
      size="sm"
      onChange={(value) => onMix(slot.id, value)}
    />
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
