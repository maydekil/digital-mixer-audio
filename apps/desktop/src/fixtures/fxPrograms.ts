import type { FxProgram } from "../adapters/MixerControlPort";

const rows = [
  ["Room", "Tiny Booth", "0.2 s", "0 ms"], ["Room", "Dry Studio", "0.3 s", "2 ms"],
  ["Room", "Small Room", "0.4 s", "4 ms"], ["Room", "Vocal Room", "0.5 s", "6 ms"],
  ["Room", "Warm Room", "0.65 s", "8 ms"], ["Room", "Bright Room", "0.8 s", "10 ms"],
  ["Room", "Wood Room", "0.95 s", "12 ms"], ["Room", "Drum Room", "1.1 s", "14 ms"],
  ["Room", "Medium Room", "1.3 s", "16 ms"], ["Room", "Wide Room", "1.5 s", "18 ms"],
  ["Room", "Large Room", "1.8 s", "20 ms"], ["Plate", "Vocal Plate", "1.4 s", "20 ms"],
  ["Plate", "Short Plate", "0.6 s", "5 ms"], ["Plate", "Soft Plate", "0.8 s", "10 ms"],
  ["Plate", "Bright Plate", "1 s", "12 ms"], ["Plate", "Warm Plate", "1.2 s", "15 ms"],
  ["Plate", "Classic Plate", "1.6 s", "22 ms"], ["Plate", "Wide Plate", "1.8 s", "25 ms"],
  ["Plate", "Pop Plate", "2 s", "28 ms"], ["Plate", "Smooth Plate", "2.3 s", "30 ms"],
  ["Plate", "Long Plate", "2.8 s", "35 ms"], ["Plate", "Epic Plate", "3.5 s", "40 ms"],
  ["Hall", "Small Hall", "1.2 s", "10 ms"], ["Hall", "Vocal Hall", "1.6 s", "15 ms"],
  ["Hall", "Warm Hall", "1.9 s", "20 ms"], ["Hall", "Bright Hall", "2.2 s", "25 ms"],
  ["Hall", "Concert Hall", "2.5 s", "30 ms"], ["Hall", "Wide Hall", "2.8 s", "35 ms"],
  ["Hall", "Deep Hall", "3.2 s", "40 ms"], ["Hall", "Long Hall", "3.8 s", "45 ms"],
  ["Hall", "Grand Hall", "4.5 s", "50 ms"], ["Hall", "Cathedral", "5.5 s", "60 ms"],
  ["Hall", "Ambient Hall", "7 s", "70 ms"], ["Slapback", "Micro Slap", "40 ms", "0 %"],
  ["Slapback", "Tight Slap", "50 ms", "0 %"], ["Slapback", "Short Slap", "60 ms", "0 %"],
  ["Slapback", "Vocal Slap", "75 ms", "0 %"], ["Slapback", "Vintage Slap", "90 ms", "5 %"],
  ["Slapback", "Rock Slap", "100 ms", "8 %"], ["Slapback", "Double Slap", "115 ms", "10 %"],
  ["Slapback", "Warm Slap", "130 ms", "12 %"], ["Slapback", "Wide Slap", "145 ms", "15 %"],
  ["Slapback", "Long Slap", "160 ms", "18 %"], ["Slapback", "Echo Slap", "180 ms", "20 %"],
  ["Stereo Delay", "Stereo 80", "80 ms", "10 %"], ["Stereo Delay", "Stereo 120", "120 ms", "12 %"],
  ["Stereo Delay", "Stereo 160", "160 ms", "15 %"], ["Stereo Delay", "Stereo 200", "200 ms", "18 %"],
  ["Stereo Delay", "Stereo 250", "250 ms", "20 %"], ["Stereo Delay", "Stereo 320", "320 ms", "25 %"],
  ["Stereo Delay", "Stereo 400", "400 ms", "30 %"], ["Stereo Delay", "Stereo 500", "500 ms", "35 %"],
  ["Stereo Delay", "Stereo 650", "650 ms", "40 %"], ["Stereo Delay", "Stereo 800", "800 ms", "45 %"],
  ["Stereo Delay", "Stereo 1000", "1000 ms", "50 %"], ["Ping-pong", "Ping 100", "100 ms", "10 %"],
  ["Ping-pong", "Ping 150", "150 ms", "12 %"], ["Ping-pong", "Ping 200", "200 ms", "15 %"],
  ["Ping-pong", "Ping 250", "250 ms", "18 %"], ["Ping-pong", "Ping 320", "320 ms", "22 %"],
  ["Ping-pong", "Ping 400", "400 ms", "26 %"], ["Ping-pong", "Ping 500", "500 ms", "30 %"],
  ["Ping-pong", "Ping 600", "600 ms", "35 %"], ["Ping-pong", "Ping 750", "750 ms", "40 %"],
  ["Ping-pong", "Ping 900", "900 ms", "45 %"], ["Ping-pong", "Ping 1200", "1200 ms", "50 %"],
  ["Chorus", "Subtle Chorus", "0.1 Hz", "10 %"], ["Chorus", "Slow Chorus", "0.15 Hz", "12 %"],
  ["Chorus", "Warm Chorus", "0.2 Hz", "15 %"], ["Chorus", "Vocal Chorus", "0.3 Hz", "18 %"],
  ["Chorus", "Wide Chorus", "0.4 Hz", "22 %"], ["Chorus", "Soft Motion", "0.5 Hz", "25 %"],
  ["Chorus", "Pop Chorus", "0.6 Hz", "30 %"], ["Chorus", "Bright Motion", "0.8 Hz", "35 %"],
  ["Chorus", "Deep Chorus", "1 Hz", "40 %"], ["Chorus", "Fast Chorus", "1.5 Hz", "45 %"],
  ["Chorus", "Liquid Chorus", "2 Hz", "50 %"], ["Phaser", "Subtle Phase", "0.1 Hz", "10 %"],
  ["Phaser", "Slow Phase", "0.15 Hz", "15 %"], ["Phaser", "Warm Phase", "0.2 Hz", "20 %"],
  ["Phaser", "Vocal Phase", "0.3 Hz", "25 %"], ["Phaser", "Wide Phase", "0.4 Hz", "30 %"],
  ["Phaser", "Soft Sweep", "0.5 Hz", "35 %"], ["Phaser", "Classic Phase", "0.6 Hz", "40 %"],
  ["Phaser", "Bright Sweep", "0.8 Hz", "45 %"], ["Phaser", "Deep Phase", "1 Hz", "50 %"],
  ["Phaser", "Fast Phase", "1.5 Hz", "55 %"], ["Phaser", "Liquid Phase", "2 Hz", "60 %"],
  ["Delay + Plate", "Vocal Space", "100 ms", "0.6 s"], ["Delay + Plate", "Short Space", "150 ms", "0.8 s"],
  ["Delay + Plate", "Warm Space", "200 ms", "1 s"], ["Delay + Plate", "Pop Space", "250 ms", "1.2 s"],
  ["Delay + Plate", "Wide Space", "320 ms", "1.4 s"], ["Delay + Plate", "Ballad Space", "400 ms", "1.8 s"],
  ["Delay + Plate", "Dream Space", "500 ms", "2.2 s"], ["Delay + Plate", "Long Space", "600 ms", "2.8 s"],
  ["Delay + Plate", "Deep Space", "750 ms", "3.5 s"], ["Delay + Plate", "Ambient Space", "900 ms", "4.5 s"],
  ["Delay + Plate", "Infinite Mood", "1200 ms", "6 s"]
];

const macroLabels = new Map([
  ["Room", ["Decay", "Pre-delay"]], ["Plate", ["Decay", "Pre-delay"]],
  ["Hall", ["Decay", "Pre-delay"]], ["Slapback", ["Time", "Feedback"]],
  ["Stereo Delay", ["Time", "Feedback"]], ["Ping-pong", ["Time", "Feedback"]],
  ["Chorus", ["Rate", "Depth"]], ["Phaser", ["Rate", "Depth"]],
  ["Delay + Plate", ["Delay", "Plate"]]
]);

export const fxPrograms: FxProgram[] = rows.map(([family, name, value1, value2], index) => {
  const labels = macroLabels.get(family) ?? ["P1", "P2"];
  return {
    id: index + 1,
    name,
    family,
    macro1: { label: labels[0], value: value1 },
    macro2: { label: labels[1], value: value2 }
  };
});
