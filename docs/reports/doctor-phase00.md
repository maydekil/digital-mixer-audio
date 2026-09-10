# Phase00 Doctor Report

Generated: 2026-09-10.

| Check | Result |
| --- | --- |
| Architecture | `arm64` |
| macOS | `26.6.2`, build `25G83` |
| Xcode tools | `/Library/Developer/CommandLineTools` |
| macOS SDK path | `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk` |
| macOS SDK version | `26.5` |
| Apple Clang | `21.0.0 (clang-2100.1.1.101)`, target `arm64-apple-darwin25.6.0` |
| CMake | `4.3.2` |
| Ninja | `1.13.2` |
| Node | `v24.15.0` |
| npm | `11.12.1` |

Package pins are recorded in `package-lock.json` and summarized by `npm run doctor`.

Phase00 conclusion: native dependency pins are documented and the local CMake + Ninja + Apple Clang toolchain is available for Phase01.
