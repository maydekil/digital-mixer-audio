export interface MeterLevel {
  left: number;
  right?: number;
  clip: boolean;
}

export interface EqBandDisplay {
  id: "low" | "mid1" | "mid2" | "high";
  label: string;
  color: string;
  freq: string;
  gain: string;
  q?: string;
  type?: string;
}
