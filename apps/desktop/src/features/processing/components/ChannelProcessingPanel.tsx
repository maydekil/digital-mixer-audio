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
  onCompressorChange(field: keyof ChannelState["dynamics"]["compressor"], value: number): void;
  onDeEsserChange(field: keyof ChannelState["dynamics"]["deEsser"], value: number): void;
}

export function ChannelProcessingPanel({ channel, eqBands, linkedProgram, onSendA, onEqChange, onCompressorChange, onDeEsserChange }: ChannelProcessingPanelProps) {
  const sendA = channel.sends["fx-a"];
  const compressor = channel.dynamics.compressor;
  const deEsser = channel.dynamics.deEsser;
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
          <RotaryKnob label="Threshold" value={`${compressor.thresholdDb} dB`} numericValue={compressor.thresholdDb} min={-80} max={0} onChange={(value) => onCompressorChange("thresholdDb", value)} />
          <RotaryKnob label="Ratio" value={`${compressor.ratio}:1`} numericValue={compressor.ratio} min={1} max={20} step={0.5} onChange={(value) => onCompressorChange("ratio", value)} />
          <RotaryKnob label="Attack" value={`${compressor.attackMs} ms`} numericValue={compressor.attackMs} min={0.1} max={200} step={0.1} onChange={(value) => onCompressorChange("attackMs", value)} />
          <RotaryKnob label="Release" value={`${compressor.releaseMs} ms`} numericValue={compressor.releaseMs} min={10} max={3000} onChange={(value) => onCompressorChange("releaseMs", value)} />
        </div>
      </section>
      <div className="lower-processors">
        <section className={`processor-card ${channel.role === "vocal" ? "is-active" : "is-bypassed"}`}>
          <div className="processor-title"><span>⏻</span><strong>DE-ESSER</strong></div>
          <RotaryKnob label="Frequency" value={formatFrequency(deEsser.frequencyHz)} numericValue={deEsser.frequencyHz} min={1000} max={12000} step={100} onChange={(value) => onDeEsserChange("frequencyHz", value)} />
          <RotaryKnob label="Threshold" value={`${deEsser.thresholdDb} dB`} numericValue={deEsser.thresholdDb} min={-80} max={0} onChange={(value) => onDeEsserChange("thresholdDb", value)} />
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

function formatFrequency(value: number) {
  return value >= 1000 ? `${(value / 1000).toFixed(1)} kHz` : `${Math.round(value)} Hz`;
}
