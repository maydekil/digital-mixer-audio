import type { ChannelState } from "../../../adapters/MixerControlPort";
import { ChannelStrip } from "./ChannelStrip";

interface ChannelBankProps {
  channels: ChannelState[];
  onSelect(channelId: string): void;
  onFader(channelId: string, value: number): void;
  onClipReset(channelId: string): void;
  onHarmonyToggle(): void;
}

export function ChannelBank({ channels, onSelect, onFader, onClipReset, onHarmonyToggle }: ChannelBankProps) {
  return (
    <section className="channel-bank">
      {channels.map((channel) => (
        <ChannelStrip
          key={channel.id}
          channel={channel}
          onSelect={() => onSelect(channel.id)}
          onFader={(value) => onFader(channel.id, value)}
          onClipReset={() => onClipReset(channel.id)}
          onHarmonyToggle={onHarmonyToggle}
        />
      ))}
    </section>
  );
}
