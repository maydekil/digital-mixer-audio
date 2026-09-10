import type { ChannelState } from "../../../adapters/MixerControlPort";
import { ClipIndicator } from "../../../components/audio/ClipIndicator";
import { LevelMeter } from "../../../components/audio/LevelMeter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { VerticalFader } from "../../../components/audio/VerticalFader";
import { Button } from "../../../components/ui/Button";

interface ChannelStripProps {
  channel: ChannelState;
  onSelect(): void;
  onFader(value: number): void;
  onClipReset(): void;
  onHarmonyToggle?(): void;
}

export function ChannelStrip({ channel, onSelect, onFader, onClipReset, onHarmonyToggle }: ChannelStripProps) {
  const isMaster = channel.kind === "master";
  const isGroup = channel.kind === "group";

  return (
    <article className={`channel-strip ${channel.selected ? "is-selected" : ""} ${isMaster ? "is-master" : ""}`} onClick={onSelect}>
      <header>
        <strong>{channel.name}</strong>
        <span>{channel.source}</span>
      </header>
      <RotaryKnob label="Trim" value={`${channel.trimDb.toFixed(1)} dB`} />
      {isMaster ? <MasterUpperControls /> : <ProcessingButtons />}
      {channel.harmonyVisible ? (
        <div className="harmony-shortcut">
          <Button tone="violet" active={channel.harmonyEnabled} onClick={onHarmonyToggle}>HARMONY {channel.harmonyEnabled ? "ON" : "OFF"}</Button>
          <Button aria-label="Harmony settings">⚙</Button>
        </div>
      ) : !isMaster ? <div className="harmony-shortcut is-placeholder" aria-hidden="true" /> : null}
      <div className="strip-divider" />
      {!isMaster ? <RotaryKnob label="Pan" value="L   R" /> : <div className="master-spacer" aria-hidden="true" />}
      {!isMaster ? <SendPair channel={channel} /> : null}
      <div className="fader-meter-row">
        <VerticalFader valueDb={channel.faderDb} onChange={onFader} label={`${channel.name} fader`} />
        <div className="strip-meter-stack">
          <ClipIndicator active={channel.meter.clip} onReset={onClipReset} />
          <LevelMeter level={channel.meter} vertical />
        </div>
      </div>
      <div className="strip-actions">
        <Button active={channel.mute}>M</Button>
        <Button active={channel.solo}>S</Button>
        {!isGroup && !isMaster ? <Button active={channel.monitor}>MON</Button> : null}
        {!isGroup && !isMaster ? <Button tone="danger" active={channel.recordArm}>REC</Button> : null}
      </div>
    </article>
  );
}

function ProcessingButtons() {
  return (
    <div className="processing-buttons">
      {["EQ", "COMP", "GATE", "INSERT FX"].map((label) => <Button key={label}>{label}</Button>)}
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

function SendPair({ channel }: { channel: ChannelState }) {
  const sendA = channel.sends["fx-a"];
  const sendB = channel.sends["fx-b"];
  return (
    <div className="send-pair">
      <RotaryKnob label="SEND A" value={sendA.enabled ? `${sendA.gainDb} dB` : "OFF"} tone="amber" size="sm" />
      <RotaryKnob label="SEND B" value={sendB.enabled ? `${sendB.gainDb} dB` : "OFF"} tone="cyan" size="sm" />
    </div>
  );
}
