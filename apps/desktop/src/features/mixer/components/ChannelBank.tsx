import type { ChannelState, EqBandState, VocalFxPreset } from "../../../adapters/MixerControlPort";
import { ChannelStrip } from "./ChannelStrip";

interface ChannelBankProps {
  channels: ChannelState[];
  sourceOptions: Record<string, Array<{ value: string; label: string }>>;
  vocalFxPresetId: string;
  vocalFxPresets: VocalFxPreset[];
  onSelect(channelId: string): void;
  onEnabled(channelId: string, enabled: boolean): void;
  onSource(channelId: string, source: string): void;
  onTrim(channelId: string, value: number): void;
  onPan(channelId: string, value: number): void;
  onFader(channelId: string, value: number): void;
  onEqBand(channelId: string, bandId: EqBandState["id"], gainDb: number): void;
  onMute(channelId: string, muted: boolean): void;
  onSolo(channelId: string, solo: boolean): void;
  onMonitor(channelId: string, monitor: boolean): void;
  onRecordArm(channelId: string, armed: boolean): void;
  onVocalFxPreset(channelId: string, presetId: string): void;
  onClipReset(channelId: string): void;
  onHarmonyToggle(): void;
  onHarmonySettings(channelId: string): void;
}

export function ChannelBank({ channels, sourceOptions, vocalFxPresetId, vocalFxPresets, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onEqBand, onMute, onSolo, onMonitor, onRecordArm, onVocalFxPreset, onClipReset, onHarmonyToggle, onHarmonySettings }: ChannelBankProps) {
  return (
    <section className="channel-bank">
      {channels.map((channel) => (
        <ChannelStrip
          key={channel.id}
          channel={channel}
          sourceOptions={sourceOptions[channel.id] ?? []}
          vocalFxPresetId={vocalFxPresetId}
          vocalFxPresets={vocalFxPresets}
          onSelect={() => onSelect(channel.id)}
          onEnabled={(enabled) => onEnabled(channel.id, enabled)}
          onSource={(source) => onSource(channel.id, source)}
          onTrim={(value) => onTrim(channel.id, value)}
          onPan={(value) => onPan(channel.id, value)}
          onFader={(value) => onFader(channel.id, value)}
          onEqBand={(bandId, gainDb) => onEqBand(channel.id, bandId, gainDb)}
          onMute={(muted) => onMute(channel.id, muted)}
          onSolo={(solo) => onSolo(channel.id, solo)}
          onMonitor={(monitor) => onMonitor(channel.id, monitor)}
          onRecordArm={(armed) => onRecordArm(channel.id, armed)}
          onVocalFxPreset={(presetId) => onVocalFxPreset(channel.id, presetId)}
          onClipReset={() => onClipReset(channel.id)}
          onHarmonyToggle={onHarmonyToggle}
          onHarmonySettings={() => onHarmonySettings(channel.id)}
        />
      ))}
    </section>
  );
}
