# Dependency Manifest

Status: Phase00 native dependency audit.

## Runtime And UI

| Dependency | Pin | Source | License | Update Policy |
| --- | --- | --- | --- | --- |
| Node.js | `>=24.15.0`; audited local `v24.15.0` | https://nodejs.org/ | MIT | Update only with lockfile/test pass. |
| npm | audited local `11.12.1` | Bundled with Node/npm | Artistic-2.0 | Keep with Node baseline unless security fix requires otherwise. |
| Electron | `44.3.0` | npm lockfile | MIT | Exact package-lock pin; update in explicit UI/native shell phase. |
| React | `19.1.1` | npm lockfile | MIT | Exact package-lock pin. |
| React DOM | `19.1.1` | npm lockfile | MIT | Exact package-lock pin. |
| Vite | `7.1.5` | npm lockfile | MIT | Exact package-lock pin. |
| TypeScript | `5.9.2` | npm lockfile | Apache-2.0 | Exact package-lock pin. |
| Playwright | `1.55.0` | npm lockfile | Apache-2.0 | Exact package-lock pin. |
| Vitest | `3.2.4` | npm lockfile | MIT | Exact package-lock pin. |

## Native Toolchain

| Dependency | Pin / Baseline | Source | License | Phase00 Result |
| --- | --- | --- | --- | --- |
| macOS | local `26.6.2` build `25G83` | Apple | Proprietary | Available. |
| Architecture | local `arm64` | Apple Silicon | N/A | Available. |
| Xcode Command Line Tools | `/Library/Developer/CommandLineTools` | Apple | Apple SDK terms | Available. |
| macOS SDK | path `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`; version `26.5` | Apple | Apple SDK terms | Available. |
| Apple Clang | `21.0.0 (clang-2100.1.1.101)` | Apple Command Line Tools | Apache-2.0 with LLVM exceptions / Apple terms | Available. |
| CMake | local `4.3.2` | https://cmake.org/ | BSD-3-Clause | Available; use this as initial supported baseline until lower versions are tested. |
| Ninja | local `1.13.2` | https://ninja-build.org/ | Apache-2.0 | Available. |

## Native Audio Stack Decision

| Dependency | Pin | Source | License | Notes |
| --- | --- | --- | --- | --- |
| JUCE | `8.0.15`, git tag SHA `91ad83ae34a81e0833b1a2b0866f54846370ae53` | https://github.com/juce-framework/JUCE | JUCE 8 license terms | Pinned to JUCE 8 as required by spec. Do not float to JUCE 9 without a new ADR. |
| C++ standard | C++20 | Apple Clang 21 | N/A | Required for native engine modules. |
| macOS deployment target | `14.0` initial target | Apple SDK | N/A | Core Audio taps and newer APIs still need runtime availability guards. |
| Core Audio | macOS SDK `26.5` | Apple SDK | Apple SDK terms | Used only from native macOS adapter layer. |
| FFmpeg | optional native executable, not yet pinned | https://ffmpeg.org/ | LGPL/GPL depending build | Not required for Phase01; pin before adding native decode/encode worker. |

## Doctor Evidence

Run:

```bash
npm run doctor
```

Current Phase00 finding: native build work can proceed with the canonical CMake + Ninja + Apple Clang stack.
