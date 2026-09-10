import type { ChannelState, ProcessorId } from "../../../adapters/MixerControlPort";
import { ChannelStrip } from "./ChannelStrip";

interface ChannelBankProps {
  channels: ChannelState[];
  sourceOptions: Record<string, Array<{ value: string; label: string }>>;
  onSelect(channelId: string): void;
  onEnabled(channelId: string, enabled: boolean): void;
  onSource(channelId: string, source: string): void;
  onTrim(channelId: string, value: number): void;
  onPan(channelId: string, value: number): void;
  onFader(channelId: string, value: number): void;
  onSend(channelId: string, unitId: "fx-a" | "fx-b", value: number): void;
  onMute(channelId: string, muted: boolean): void;
  onSolo(channelId: string, solo: boolean): void;
  onMonitor(channelId: string, monitor: boolean): void;
  onRecordArm(channelId: string, armed: boolean): void;
  onProcessor(channelId: string, processorId: ProcessorId, enabled: boolean): void;
  onClipReset(channelId: string): void;
  onHarmonyToggle(): void;
}

export function ChannelBank({ channels, sourceOptions, onSelect, onEnabled, onSource, onTrim, onPan, onFader, onSend, onMute, onSolo, onMonitor, onRecordArm, onProcessor, onClipReset, onHarmonyToggle }: ChannelBankProps) {
  return (
    <section className="channel-bank">
      {channels.map((channel) => (
        <ChannelStrip
          key={channel.id}
          channel={channel}
          sourceOptions={sourceOptions[channel.id] ?? []}
          onSelect={() => onSelect(channel.id)}
          onEnabled={(enabled) => onEnabled(channel.id, enabled)}
          onSource={(source) => onSource(channel.id, source)}
          onTrim={(value) => onTrim(channel.id, value)}
          onPan={(value) => onPan(channel.id, value)}
          onFader={(value) => onFader(channel.id, value)}
          onSend={(unitId, value) => onSend(channel.id, unitId, value)}
          onMute={(muted) => onMute(channel.id, muted)}
          onSolo={(solo) => onSolo(channel.id, solo)}
          onMonitor={(monitor) => onMonitor(channel.id, monitor)}
          onRecordArm={(armed) => onRecordArm(channel.id, armed)}
          onProcessor={(processorId, enabled) => onProcessor(channel.id, processorId, enabled)}
          onClipReset={() => onClipReset(channel.id)}
          onHarmonyToggle={onHarmonyToggle}
        />
      ))}
    </section>
  );
}
