# ADR 0001: Native Audio Stack

Status: Accepted for Phase00.

## Context

The UI gate is now `UI_VERIFIED`. The project must move into native work without changing the target into a browser audio app. The specification requires production audio to run locally through C++20, JUCE, Core Audio, or approved native media workers.

## Decision

- Keep React/Vite/Electron as the desktop UI shell.
- Implement the production audio engine in native C++20.
- Pin JUCE to `8.0.15` at tag SHA `91ad83ae34a81e0833b1a2b0866f54846370ae53`.
- Build native targets with CMake and Ninja using Apple Clang.
- Use macOS deployment target `14.0` as the initial baseline; newer Core Audio capabilities require runtime availability guards.
- Keep browser renderer audio APIs out of production audio paths: no Web Audio, `<audio>`, browser capture, or JS DSP fallback.
- Keep platform code isolated under the future `native/engine/src/platform/macos/` boundary.

## Consequences

- Phase01 cannot honestly claim CMake/Ninja native build success until Ninja is installed or the build generator decision is revised by a new ADR.
- JUCE license obligations must be reviewed before packaging or distribution.
- Existing sound-pad helper remains a narrow native spike and does not replace the JUCE mixer engine.
