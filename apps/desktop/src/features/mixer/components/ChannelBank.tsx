import type { ChannelState, ProcessorId } from "../../../adapters/MixerControlPort";
import { ChannelStrip } from "./ChannelStrip";

interface ChannelBankProps {
  channels: ChannelState[];
  onSelect(channelId: string): void;
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

export function ChannelBank({ channels, onSelect, onTrim, onPan, onFader, onSend, onMute, onSolo, onMonitor, onRecordArm, onProcessor, onClipReset, onHarmonyToggle }: ChannelBankProps) {
  return (
    <section className="channel-bank">
      {channels.map((channel) => (
        <ChannelStrip
          key={channel.id}
          channel={channel}
          onSelect={() => onSelect(channel.id)}
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
