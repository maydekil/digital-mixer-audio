# INT-02 Package Smoke

Bundle: `build/package/Local Audio Mixer.app`
Archive: `build/package/Local-Audio-Mixer-dev.zip`
Revision: `dcc8655`

## Automated Smoke

| Check | Result | Evidence |
| --- | --- | --- |
| file Electron | PASS | build/package/Local Audio Mixer.app/Contents/MacOS/Electron: Mach-O 64-bit executable arm64 |
| file local-mixer-engine | PASS | build/package/Local Audio Mixer.app/Contents/Resources/native/engine/local-mixer-engine: Mach-O 64-bit executable arm64 |
| file local-mixer-plugin-scanner | PASS | build/package/Local Audio Mixer.app/Contents/Resources/native/engine/local-mixer-plugin-scanner: Mach-O 64-bit executable arm64 |
| file sound-pad-helper | PASS | build/package/Local Audio Mixer.app/Contents/Resources/native/sound-pad/sound-pad-helper: Mach-O 64-bit executable arm64 |
| packaged engine version | PASS | {"name":"local-mixer-engine","version":"0.1.0-phase04-routing-foundation","protocol":1,"audio":"not-started","juce":"pinned-8.0.15-not-linked"} |
| packaged plugin scanner self-test | PASS | {"scanner":"ok","selfTest":true} |
| packaged sound pad validate | PASS | sound-pad-helper validate ok |

## Permission And Offline Notes

- `NSMicrophoneUsageDescription` is present in `Info.plist` for packaged-app TCC identity.
- Runtime GUI launch and macOS permission prompt from the `.app` are NOT_RUN in this command flow.
- Native engine, plugin scanner, sound-pad helper, UI bundle, and sound-pad assets are copied into app resources.
- This unsigned dev package is not notarized or suitable for public distribution.
