import type { ChannelState, EqBandState, ProcessorId } from "../../../adapters/MixerControlPort";
import { EqResponseGraph } from "../../../components/audio/EqResponseGraph";
import { NumericParameter } from "../../../components/audio/NumericParameter";
import { RotaryKnob } from "../../../components/audio/RotaryKnob";
import { Button } from "../../../components/ui/Button";
import { Badge } from "../../../components/ui/Badge";

interface ChannelProcessingPanelProps {
  channel: ChannelState;
  eqBands: EqBandState[];
  onEqChange(bandId: EqBandState["id"], field: "freqHz" | "gainDb" | "qValue" | "type", value: number | string): void;
  onEqReset(): void;
  onProcessor(processorId: ProcessorId, enabled: boolean): void;
  onNoiseChange(field: keyof ChannelState["dynamics"]["noise"], value: number): void;
  onCompressorChange(field: keyof ChannelState["dynamics"]["compressor"], value: number): void;
  onDeEsserChange(field: keyof ChannelState["dynamics"]["deEsser"], value: number): void;
}

export function ChannelProcessingPanel({ channel, eqBands, onEqChange, onEqReset, onProcessor, onNoiseChange, onCompressorChange, onDeEsserChange }: ChannelProcessingPanelProps) {
  const noise = channel.dynamics.noise;
  const compressor = channel.dynamics.compressor;
  const deEsser = channel.dynamics.deEsser;
  const deEsserAvailable = channel.role === "vocal";
  const deEsserEnabled = deEsserAvailable && channel.processing.deEsser;
  return (
    <aside className="processing-panel">
      <header className="processing-header">
        <div>
          <h2>{channel.name}</h2>
          <p>{channel.kind === "master" ? "Master output processing" : "Channel processing"}</p>
        </div>
        {channel.monitor ? <Badge>MONITOR ON</Badge> : null}
        <Button>Presets⌄</Button>
      </header>
      <section className={`processor-card eq-card ${channel.processing.eq ? "is-active" : "is-bypassed"}`}>
        <div className="processor-title"><span>⏻</span><strong>PARAMETRIC EQ</strong><Button tone="cyan" className="processor-reset" onClick={onEqReset}>RESET</Button><label>HPF <input type="checkbox" checked readOnly /> <b>80 Hz</b></label></div>
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
        <section className={`processor-card ${channel.processing.noise ? "is-active" : "is-bypassed"}`}>
          <div className="processor-title"><span>⏻</span><strong>NOISE</strong></div>
          <div className="mini-controls">
            <RotaryKnob label="Threshold" value={`${noise.thresholdDb} dB`} numericValue={noise.thresholdDb} min={-90} max={0} onChange={(value) => onNoiseChange("thresholdDb", value)} />
            <RotaryKnob label="Range" value={`${noise.rangeDb} dB`} numericValue={noise.rangeDb} min={-90} max={0} onChange={(value) => onNoiseChange("rangeDb", value)} />
            <RotaryKnob label="Hold" value={`${noise.holdMs} ms`} numericValue={noise.holdMs} min={0} max={1000} onChange={(value) => onNoiseChange("holdMs", value)} />
            <RotaryKnob label="Release" value={`${noise.releaseMs} ms`} numericValue={noise.releaseMs} min={5} max={3000} onChange={(value) => onNoiseChange("releaseMs", value)} />
          </div>
        </section>
        <section className={`processor-card ${deEsserEnabled ? "is-active" : "is-bypassed"}`}>
          <div className="processor-title">
            <button
              type="button"
              className="processor-power"
              disabled={!deEsserAvailable}
              aria-pressed={deEsserEnabled}
              onClick={() => onProcessor("deEsser", !deEsserEnabled)}
            >
              ⏻
            </button>
            <strong>DE-ESSER</strong>
          </div>
          <RotaryKnob label="Frequency" value={formatFrequency(deEsser.frequencyHz)} numericValue={deEsser.frequencyHz} min={1000} max={12000} step={100} disabled={!deEsserEnabled} onChange={(value) => onDeEsserChange("frequencyHz", value)} />
          <RotaryKnob label="Threshold" value={`${deEsser.thresholdDb} dB`} numericValue={deEsser.thresholdDb} min={-80} max={0} disabled={!deEsserEnabled} onChange={(value) => onDeEsserChange("thresholdDb", value)} />
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
