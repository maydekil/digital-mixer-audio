import type { EqBandDisplay } from "./types";

interface EqResponseGraphProps {
  bands: EqBandDisplay[];
  onBandChange?(bandId: EqBandDisplay["id"], freqHz: number, gainDb: number): void;
}

export function EqResponseGraph({ bands, onBandChange }: EqResponseGraphProps) {
  const points = bands.map((band) => `${freqToX(band.freqHz)},${gainToY(band.gainDb)}`).join(" ");
  const first = bands[0];
  const last = bands[bands.length - 1];
  const leftY = first ? gainToY(Math.max(-12, first.gainDb - 9)) : 132;
  const rightY = last ? gainToY(last.gainDb + 1) : 58;

  function updateBand(bandId: EqBandDisplay["id"], clientX: number, clientY: number, svg: SVGSVGElement) {
    const rect = svg.getBoundingClientRect();
    const x = ((clientX - rect.left) / rect.width) * 700;
    const y = ((clientY - rect.top) / rect.height) * 180;
    onBandChange?.(bandId, xToFreq(x), yToGain(y));
  }

  return (
    <svg className="eq-graph" viewBox="0 0 700 180" role="img" aria-label="Parametric EQ response preview">
      <defs>
        <linearGradient id="eqFill" x1="0" x2="0" y1="0" y2="1">
          <stop offset="0%" stopColor="#18d6e7" stopOpacity="0.26" />
          <stop offset="100%" stopColor="#18d6e7" stopOpacity="0.02" />
        </linearGradient>
      </defs>
      {Array.from({ length: 15 }).map((_, index) => <line key={`v-${index}`} x1={index * 50} x2={index * 50} y1="10" y2="150" />)}
      {Array.from({ length: 7 }).map((_, index) => <line key={`h-${index}`} x1="0" x2="700" y1={20 + index * 22} y2={20 + index * 22} />)}
      <text x="0" y="35">+12</text><text x="0" y="94">0</text><text x="0" y="145">-12</text>
      <text x="0" y="174">20</text><text x="150" y="174">100</text><text x="355" y="174">1k</text><text x="590" y="174">10k</text><text x="668" y="174">20k</text>
      <polygon points={`0,150 0,${leftY} ${points} 700,${rightY} 700,150`} fill="url(#eqFill)" />
      <polyline points={`0,${leftY} ${points} 700,${rightY}`} fill="none" stroke="#18d6e7" strokeWidth="3" />
      {bands.map((band) => (
        <circle
          key={band.id}
          cx={freqToX(band.freqHz)}
          cy={gainToY(band.gainDb)}
          r="12"
          fill={band.color}
          stroke="#061016"
          strokeWidth="3"
          tabIndex={0}
          onPointerDown={(event) => {
            event.currentTarget.setPointerCapture(event.pointerId);
            updateBand(band.id, event.clientX, event.clientY, event.currentTarget.ownerSVGElement as SVGSVGElement);
          }}
          onPointerMove={(event) => {
            if (event.buttons === 1) updateBand(band.id, event.clientX, event.clientY, event.currentTarget.ownerSVGElement as SVGSVGElement);
          }}
          onKeyDown={(event) => {
            const freqStep = event.shiftKey ? 1000 : 100;
            const gainStep = event.shiftKey ? 1 : 0.5;
            if (event.key === "ArrowLeft") onBandChange?.(band.id, band.freqHz - freqStep, band.gainDb);
            if (event.key === "ArrowRight") onBandChange?.(band.id, band.freqHz + freqStep, band.gainDb);
            if (event.key === "ArrowUp") onBandChange?.(band.id, band.freqHz, band.gainDb + gainStep);
            if (event.key === "ArrowDown") onBandChange?.(band.id, band.freqHz, band.gainDb - gainStep);
          }}
        >
          <title>{band.label}</title>
        </circle>
      ))}
    </svg>
  );
}

function freqToX(freqHz: number) {
  const min = Math.log10(20);
  const max = Math.log10(20_000);
  return ((Math.log10(Math.max(20, Math.min(20_000, freqHz))) - min) / (max - min)) * 680 + 10;
}

function xToFreq(x: number) {
  const min = Math.log10(20);
  const max = Math.log10(20_000);
  const normalized = Math.max(0, Math.min(1, (x - 10) / 680));
  return Math.round(10 ** (min + normalized * (max - min)));
}

function gainToY(gainDb: number) {
  return 85 - Math.max(-12, Math.min(12, gainDb)) * 5.4;
}

function yToGain(y: number) {
  return Math.round(Math.max(-12, Math.min(12, (85 - y) / 5.4)) * 2) / 2;
}
