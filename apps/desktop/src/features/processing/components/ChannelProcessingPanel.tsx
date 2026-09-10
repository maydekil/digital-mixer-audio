import type { ChannelState, EqBandState, FxProgram } from "../../../adapters/MixerControlPort";
import { EqResponseGraph } from "../../../components/audio/EqResponseGraph";
import { NumericParameter } from "../../../components/audio/NumericParameter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { Button } from "../../../components/ui/Button";
import { Badge } from "../../../components/ui/Badge";

interface ChannelProcessingPanelProps {
  channel: ChannelState;
  eqBands: EqBandState[];
  linkedProgram: FxProgram;
}

export function ChannelProcessingPanel({ channel, eqBands, linkedProgram }: ChannelProcessingPanelProps) {
  const sendA = channel.sends["fx-a"];
  return (
    <aside className="processing-panel">
      <header className="processing-header">
        <div>
          <h2>{channel.name}</h2>
          <p>Channel processing</p>
        </div>
        {channel.monitor ? <Badge>MONITOR ON</Badge> : null}
        <Button>Presets⌄</Button>
      </header>
      <section className="processor-card eq-card">
        <div className="processor-title"><span>⏻</span><strong>PARAMETRIC EQ</strong><label>HPF <input type="checkbox" checked readOnly /> <b>80 Hz</b></label></div>
        <EqResponseGraph bands={eqBands} />
        <div className="eq-band-grid">
          {eqBands.map((band) => (
            <div className="eq-band" key={band.id}>
              <strong><span style={{ background: band.color }} />{band.label}</strong>
              <NumericParameter label="Freq" value={band.freq} color={band.color} />
              <NumericParameter label="Gain" value={band.gain} color={band.color} />
              <NumericParameter label={band.q ? "Q" : "Type"} value={band.q ?? band.type ?? ""} color={band.color} />
            </div>
          ))}
        </div>
      </section>
      <section className="processor-card compressor-card">
        <div className="processor-title"><span>⏻</span><strong>COMPRESSOR</strong><div className="gain-reduction">Gain Reduction <i /></div></div>
        <div className="compressor-controls">
          <RotaryKnob label="Threshold" value="-18 dB" />
          <RotaryKnob label="Ratio" value="3:1" />
          <RotaryKnob label="Attack" value="10 ms" />
          <RotaryKnob label="Release" value="120 ms" />
        </div>
      </section>
      <div className="lower-processors">
        <section className="processor-card">
          <div className="processor-title"><span>⏻</span><strong>DE-ESSER</strong></div>
          <RotaryKnob label="Frequency" value="6.0 kHz" />
        </section>
        <section className="processor-card">
          <div className="processor-title"><span>⏻</span><strong>SEND A · {linkedProgram.name}</strong></div>
          <RotaryKnob label="Send A" value={sendA.enabled ? `${sendA.gainDb} dB` : "OFF"} tone="amber" />
        </section>
      </div>
    </aside>
  );
}
