export interface MeterLevel {
  left: number;
  right?: number;
  clip: boolean;
}

export interface EqBandDisplay {
  id: "low" | "mid1" | "mid2" | "high";
  label: string;
  color: string;
  freqHz: number;
  gainDb: number;
  qValue?: number;
  freq: string;
  gain: string;
  q?: string;
  type?: string;
}
