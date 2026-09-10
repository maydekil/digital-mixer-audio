# Sound Pad Audio Assets

Status: `REAL_CC0_LOCAL_WAV`

These files are persistent local WAV assets for the native sound pad spike. They are not generated at click time, and the renderer does not use Web Audio or browser audio playback.

| Pad | Local file | Source | Original file | License |
| --- | --- | --- | --- | --- |
| Applause | `assets/sound-pads/applause.wav` | https://opengameart.org/content/applause-in-a-large-hall-or-church | `applause-clapping-church-crowd-immersive.wav`, trimmed to 4.5s with fade-out | CC0 |
| Laugh | `assets/sound-pads/laugh.wav` | https://opengameart.org/content/evil-cyber-laugh | `evil cyber laugh.wav` | CC0 |
| Cheer | `assets/sound-pads/cheer.wav` | https://opengameart.org/content/cheers-0 | `cheers.ogg`, converted to PCM WAV | CC0 |
| Drum Roll | `assets/sound-pads/drumroll.wav` | https://opengameart.org/content/horde-war-drums-loop | `horde_war_drums_by_william_hector.wav`, trimmed to 4.0s with fade-out | CC0 |
| Ding | `assets/sound-pads/ding.wav` | https://opengameart.org/content/bell-dingschimes | `bell_ding1.wav` | CC0 |
| Whoosh | `assets/sound-pads/whoosh.wav` | https://opengameart.org/content/air-whoosh | `whoosh2.wav` | CC0 |

Verification evidence:

- `file assets/sound-pads/*.wav`: all six assets report `RIFF ... WAVE audio`.
- `afinfo assets/sound-pads/*.wav`: all six assets parse as PCM WAV files with valid channel count, sample rate, duration, and bit depth.
- `npm run build:native:sound-pad`: native helper builds and validates that the persistent WAV assets exist without overwriting them.
