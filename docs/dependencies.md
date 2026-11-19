# Dependencies

Every third-party component CUTLINE uses, the pinned version, its licence and why it's here.
See [licensing.md](licensing.md) for the rules these follow.

## Build tools

| Tool | Version | Licence | Why |
|---|---|---|---|
| CMake | ≥ 3.28 (dev PC: 4.4.4) | BSD-3 | build system |
| Ninja | from Visual Studio / distro | Apache-2.0 | build backend |
| MSVC | 19.44 (VS 2022) | proprietary (build tool only) | Windows compiler |
| Windows SDK | 10.0.26100 | proprietary (build tool only) | Windows headers |
| vcpkg | baseline `cf82c6e6a919217c1c3a36b36dc40e370a71591e` (tool 2026-10-29) | MIT | builds FFmpeg on Windows and macOS |
| aqtinstall | 3.3.0 | MIT | installs Qt binaries |

## Libraries

| Library | Version | Licence | Why |
|---|---|---|---|
| Qt | 6.10.3 (`tooling/qt-version.txt`); distro builds ≥ 6.8 | LGPLv3 | UI, QML, core utilities, tests |
| FFmpeg | 9.0.2 (vcpkg port at the baseline); distro version on Linux | LGPL 2.1+ (official builds) | media decode/encode |

**Why Qt 6.10.3 and not newer:** Qt 6.11 and 6.12 are released, but aqtinstall 3.3.0, which the dev setup and CI
both use, can't fetch their package index yet. 6.10.3 is the newest version both can install. Bump
`tooling/qt-version.txt` when that changes.

## Fonts and icons

Added in Milestone 2. Each will be listed here with its version and licence.
