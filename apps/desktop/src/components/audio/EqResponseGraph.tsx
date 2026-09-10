import type { EqBandDisplay } from "./types";

export function EqResponseGraph({ bands }: { bands: EqBandDisplay[] }) {
  const points = "0,132 80,66 160,58 250,84 340,88 455,78 570,60 690,51";
  const nodePoints = [
    { x: 135, y: 61 }, { x: 260, y: 85 }, { x: 405, y: 68 }, { x: 560, y: 52 }
  ];

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
      <polygon points={`0,150 ${points} 700,58 700,150`} fill="url(#eqFill)" />
      <polyline points={`${points} 700,58`} fill="none" stroke="#18d6e7" strokeWidth="3" />
      {bands.map((band, index) => (
        <circle key={band.id} cx={nodePoints[index].x} cy={nodePoints[index].y} r="12" fill={band.color} stroke="#061016" strokeWidth="3" />
      ))}
    </svg>
  );
}
