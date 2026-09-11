import type { VocalFxPreset } from "../adapters/MixerControlPort";

const voicePresetNames = [
  "Clean Voice", "Warm Broadcast", "Studio Pop", "Karaoke Hall", "Slapback", "Wide Double",
  "Low Character", "Bright Character", "Hard Tune", "Harmony Duo", "Telephone", "Robot",
  "Clean Tight", "Clean Air", "Clean Soft", "Clean Present", "Clean Natural", "Clean Stage",
  "Warm Close", "Warm Radio", "Warm Smooth", "Warm Room", "Warm Thick", "Warm Narration",
  "Pop Lead", "Pop Bright", "Pop Wide", "Pop Soft Tune", "Pop Plate", "Pop Chorus",
  "Hall Small", "Hall Medium", "Hall Wide", "Hall Long", "Hall Bright", "Hall Dark",
  "Delay Short", "Delay Quarter", "Delay Stereo", "Delay Tape", "Delay Air", "Delay Throw",
  "Double Light", "Double Wide", "Double Thick", "Double Chorus", "Double Pop", "Double Stage",
  "Low Male", "Low Warm", "Low Deep", "Low Monster", "Low Radio", "Low Space",
  "Bright Female", "Bright Air", "Bright Shine", "Bright Pop", "Bright Edge", "Bright Plate",
  "Tune Light", "Tune Modern", "Tune Fast", "Tune Pop", "Tune Tight", "Tune Hard",
  "Harmony Third", "Harmony Fifth", "Harmony Wide", "Harmony Soft", "Harmony Pop", "Harmony Hall",
  "Lo-Fi Phone", "Lo-Fi Radio", "Lo-Fi Megaphone", "Lo-Fi Thin", "Lo-Fi Saturated", "Lo-Fi Vintage",
  "Robot Soft", "Robot Modern", "Robot Deep", "Robot Bright", "Robot Wide", "Robot Hard",
  "Podcast Clean", "Podcast Warm", "Podcast Air", "Podcast Tight", "Podcast Deep", "Podcast Room",
  "Live Clean", "Live Warm", "Live Pop", "Live Hall", "Live Delay", "Live Double",
  "Special Whisper", "Special Crowd", "Special Synth"
] as const;

export const vocalFxPresets: VocalFxPreset[] = voicePresetNames.map((name, index) => ({
  id: `voice-${String(index + 1).padStart(2, "0")}-${slug(name)}`,
  name: `${String(index + 1).padStart(2, "0")} · ${name}`,
  archetypeId: archetypeForName(name)
}));

export function vocalFxPresetEffectTypes(presetId: string) {
  const archetypeId = vocalFxPresets.find((preset) => preset.id === presetId)?.archetypeId ?? presetId;
  const enabledByPreset: Record<string, string[]> = {
    "clean-voice": [],
    "warm-broadcast": ["saturation", "reverb"],
    "studio-pop": ["pitch_correct", "doubler", "reverb"],
    "karaoke-hall": ["reverb", "delay"],
    "slapback": ["delay"],
    "wide-double": ["doubler", "chorus"],
    "low-character": ["pitch_shift", "formant_shift"],
    "bright-character": ["pitch_shift", "formant_shift"],
    "hard-tune": ["pitch_correct"],
    "harmony-duo": ["harmony", "reverb"],
    "telephone": ["saturation"],
    "robot": ["vocoder"]
  };
  return enabledByPreset[archetypeId];
}

function slug(value: string) {
  return value.toLowerCase().replaceAll(" ", "-");
}

function archetypeForName(name: string) {
  if (name.includes("Robot")) return "robot";
  if (name.includes("Harmony")) return "harmony-duo";
  if (name.includes("Phone") || name.includes("Radio") || name.includes("Megaphone") || name.includes("Lo-Fi")) return "telephone";
  if (name.includes("Tune")) return "hard-tune";
  if (name.includes("Low")) return "low-character";
  if (name.includes("Bright")) return "bright-character";
  if (name.includes("Double")) return "wide-double";
  if (name.includes("Delay") || name.includes("Slapback")) return "slapback";
  if (name.includes("Hall") || name.includes("Karaoke")) return "karaoke-hall";
  if (name.includes("Pop") || name.includes("Studio")) return "studio-pop";
  if (name.includes("Warm") || name.includes("Broadcast") || name.includes("Podcast")) return "warm-broadcast";
  return "clean-voice";
}
