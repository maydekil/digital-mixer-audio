# INT-02 Package Smoke

Bundle: `build/package/Local Audio Mixer.app`
Archive: `build/package/Local-Audio-Mixer-dev.zip`
Revision: `0f7acca`

## Automated Smoke

| Check | Result | Evidence |
| --- | --- | --- |
| file Electron | PASS | build/package/Local Audio Mixer.app/Contents/MacOS/Electron: Mach-O 64-bit executable arm64 |
| packaged plist identity | PASS | {<br>  "CFBundleDisplayName" => "Local Audio Mixer"<br>  "CFBundleExecutable" => "Electron"<br>  "CFBundleIconFile" => "electron.icns"<br>  "CFBundleIdentifier" => "audio.local-mixer.dev"<br>  "CFBundleInfoDictionaryVersion" => "6.0"<br>  "CFBundleName" => "Local Audio Mixer"<br>  "CFBundlePackageType" => "APPL"<br>  "CFBundleShortVersionString" => "44.3.0"<br>  "CFBundleVersion" => "44.3.0"<br>  "DTCompiler" => "com.apple.compilers.llvm.clang.1_0"<br>  "DTSDKBuild" => "24F74"<br>  "DTSDKName" => "macosx15.5"<br>  "DTXcode" => "1640"<br>  "DTXcodeBuild" => "16F6"<br>  "ElectronAsarIntegrity" => {<br>    "Resources/default_app.asar" => {<br>      "algorithm" => "SHA256"<br>      "hash" => "553d6dd413d71a62ae1e67b1db59bd83fadf16ae529f015415dce85787ea437b"<br>    }<br>  }<br>  "LSApplicationCategoryType" => "public.app-category.developer-tools"<br>  "LSEnvironment" => {<br>    "MallocNanoZone" => "0"<br>  }<br>  "LSMinimumSystemVersion" => "13.0"<br>  "NSAppTransportSecurity" => {<br>    "NSAllowsArbitraryLoads" => true<br>  }<br>  "NSAudioCaptureUsageDescription" => "This app needs access to audio capture"<br>  "NSBluetoothAlwaysUsageDescription" => "This app needs access to Bluetooth"<br>  "NSBluetoothPeripheralUsageDescription" => "This app needs access to Bluetooth"<br>  "NSCameraUsageDescription" => "This app needs access to the camera"<br>  "NSHighResolutionCapable" => true<br>  "NSMainNibFile" => "MainMenu"<br>  "NSMicrophoneUsageDescription" => "Local Audio Mixer needs microphone access for native live input monitoring and recording."<br>  "NSPrefersDisplaySafeAreaCompatibilityMode" => false<br>  "NSPrincipalClass" => "AtomApplication"<br>  "NSQuitAlwaysKeepsWindows" => false<br>  "NSRequiresAquaSystemAppearance" => false<br>  "NSScreenCaptureUsageDescription" => "Local Audio Mixer may request system audio capture permission for supported Core Audio tap workflows."<br>  "NSSupportsAutomaticGraphicsSwitching" => true<br>} |
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
