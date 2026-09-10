# UI Layout Review

Status: IMPLEMENTED_UNVERIFIED pending final `verify` run.

Reference:
- `docs/design/Digital Mixer Audio.png`

Screenshots:
- `docs/reports/ui/desktop-1680x945.png`
- `docs/reports/ui/minimum-1280x800.png`

Verified visually:
- Compact FX A and FX B rows remain above the mixer.
- Mixer bank sits on the left and Channel Processing remains visible on the right.
- VOICE strip is highlighted and includes distinct `INSERT FX` and `HARMONY ON` controls.
- Harmony quick tray remains visible below the channel bank.
- EQ graph, Freq/Gain/Q controls, compressor, de-esser area, linked SEND A area, CLIP, MON/REC, Modified and preview badge are present.
- Preview label is visible at 1680x945 and 1280x800.
- Typography and control sizing were compacted after review so labels stay within their containers at 1280x800.
- Channel strip fader/meter/action sections are bottom-aligned so channel controls do not appear to hang at inconsistent heights.
- Strip dividers above the pan/send area align across source and master strips.
- Master fader and meter use a taller master-specific layout so they extend toward the divider instead of matching source fader height.
- Text weight and label/control spacing were tuned down for a more compact professional tool feel.
- Channel Processing inspector typography and control scale were reduced so the right panel does not dominate the mixer.
- Harmony quick tray is split into two rows so voice controls and level controls have clearer spacing.
- FX A/B macro knob positions are grid-aligned across both rows, and stereo return meters include visible L/R labels.
- FX A/B return sliders are shortened and shifted right to avoid an overlong bar immediately after the macro controls.
- Right-side panel no longer spends the entire lower area on empty processing cards; a dedicated Sound Pads panel now occupies the lower strip with six compact effect buttons.
- Sound Pad buttons have hover/focus/pressed states and remain clickable in UI preview; native bridge absence is handled as a warning rather than disabled UI.
- Sound Pad panel now surfaces playback/error status directly, and desktop dev startup rebuilds the native helper and Electron bridge before launch.
- Electron bridge is now loaded via copied CommonJS preload `dist/electron/preload.cjs`, which exposes `window.localMixer.playSoundPad` for the sound pad buttons.
- Sound Pad assets are local CC0 WAV samples documented in `docs/reports/sound-pad-assets.md`; they are no longer procedural placeholder sounds.

Known visual limitations:
- The 1280x800 layout is intentionally dense; lower-priority helper text in the Harmony tray is hidden at that breakpoint.
- Native telemetry, audio meters, engine status, recording, and hardware state are preview fixtures only.
- Sound Pad audio still requires the Electron desktop app with an available macOS output device; browser UI preview does not play audio.

Native status:
- PARTIAL_SOUND_PAD_SPIKE. A native C++ sound-pad helper is wired through Electron IPC, but full mixer engine, capture, DSP, recording, and export remain unimplemented.
- Sound pad clicks now target persistent real WAV files in `assets/sound-pads`; click-time playback no longer creates, deletes, or regenerates audio files.
- Audible playback was not verified in this command environment because macOS `/usr/bin/afplay` reports `AudioQueueStart failed (-66680)`; native helper build and asset validation passed.
