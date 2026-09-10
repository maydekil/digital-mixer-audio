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
  onSendA(valueDb: number): void;
  onEqChange(bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string): void;
}

export function ChannelProcessingPanel({ channel, eqBands, linkedProgram, onSendA, onEqChange }: ChannelProcessingPanelProps) {
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
      <section className={`processor-card eq-card ${channel.processing.eq ? "is-active" : "is-bypassed"}`}>
        <div className="processor-title"><span>⏻</span><strong>PARAMETRIC EQ</strong><label>HPF <input type="checkbox" checked readOnly /> <b>80 Hz</b></label></div>
        <EqResponseGraph bands={eqBands} onBandChange={(bandId, freqHz, gainDb) => {
          onEqChange(bandId, "freqHz", freqHz);
          onEqChange(bandId, "gainDb", gainDb);
        }} />
        <div className="eq-band-grid">
          {eqBands.map((band) => (
            <div className="eq-band" key={band.id}>
              <strong><span style={{ background: band.color }} />{band.label}</strong>
              <NumericParameter label="Freq" value={band.freq} color={band.color} onCommit={(value) => onEqChange(band.id, "freqHz", parseFrequency(value))} />
              <NumericParameter label="Gain" value={band.gain} color={band.color} onCommit={(value) => onEqChange(band.id, "gainDb", parseNumber(value))} />
              <NumericParameter label={band.q ? "Q" : "Type"} value={band.q ?? band.type ?? ""} color={band.color} onCommit={(value) => {
                if (band.qValue !== undefined) onEqChange(band.id, "qValue", parseNumber(value));
                else onEqChange(band.id, "type", value);
              }} />
            </div>
          ))}
        </div>
      </section>
      <section className={`processor-card compressor-card ${channel.processing.comp ? "is-active" : "is-bypassed"}`}>
        <div className="processor-title"><span>⏻</span><strong>COMPRESSOR</strong><div className="gain-reduction">Gain Reduction <i /></div></div>
        <div className="compressor-controls">
          <RotaryKnob label="Threshold" value="-18 dB" />
          <RotaryKnob label="Ratio" value="3:1" />
          <RotaryKnob label="Attack" value="10 ms" />
          <RotaryKnob label="Release" value="120 ms" />
        </div>
      </section>
      <div className="lower-processors">
        <section className={`processor-card ${channel.processing.noise ? "is-active" : "is-bypassed"}`}>
          <div className="processor-title"><span>⏻</span><strong>NOISE CONTROL</strong></div>
          <RotaryKnob label="Frequency" value="6.0 kHz" />
        </section>
        <section className={`processor-card ${channel.processing.insertFx ? "is-active" : "is-bypassed"}`}>
          <div className="processor-title"><span>⏻</span><strong>SEND A · {linkedProgram.name}</strong></div>
          <RotaryKnob label="Send A" value={sendA.enabled ? `${sendA.gainDb} dB` : "OFF"} numericValue={sendA.gainDb} min={-60} max={10} tone="amber" onChange={onSendA} />
        </section>
      </div>
    </aside>
  );
}

function parseNumber(value: string) {
  const parsed = Number.parseFloat(value.replace(",", "."));
  return Number.isFinite(parsed) ? parsed : 0;
}

function parseFrequency(value: string) {
  const parsed = parseNumber(value);
  return value.toLowerCase().includes("k") ? parsed * 1000 : parsed;
}
